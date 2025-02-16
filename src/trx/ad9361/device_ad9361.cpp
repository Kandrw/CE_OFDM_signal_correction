#include "device_ad9361.hpp"

#include <mutex>

#ifdef M_FOUND_LIBIIO || 1
// #include <iio.h>
#include "/home/andrey/Документы/libiio/include/iio/iio.h"
// #include "/home/andrey/Документы/libiio/include/iio/iio-debug.h"
#endif

#include "../types_trx.hpp"

#include <output.hpp>

#define IIO_ENSURE(expr) { \
	if (!(expr)) { \
		(void) fprintf(stdout, "assertion failed (%s:%d)\n", __FILE__, __LINE__); \
		(void) abort(); \
	} \
}

#if 1
#define dev_perror(dev, err, ...) do{}while(0)
#else
#define dev_perror(dev, err, ...)	ctx_perror(__dev_ctx_or_null(dev), err, \
						   "%s: " __FIRST(__VA_ARGS__, 0) "%s", \
						   __dev_id_or_null(dev), \
						   __SKIPFIRST(__VA_ARGS__, ""))
#endif
enum iodev { RX = 1, TX };

static struct iio_device *tx = NULL;
static struct iio_device *rx = NULL;

/* static scratch mem for strings */
char tmpstr[64];

static struct iio_context *ctx   = NULL;
static struct iio_channel *rx0_i = NULL;
static struct iio_channel *rx0_q = NULL;
static struct iio_channel *tx0_i = NULL;
static struct iio_channel *tx0_q = NULL;
static struct iio_buffer  *rxbuf = NULL;
static struct iio_buffer  *txbuf = NULL;
static struct iio_stream  *rxstream = NULL;
static struct iio_stream  *txstream = NULL;
static struct iio_channels_mask *rxmask = NULL;
static struct iio_channels_mask *txmask = NULL;


static bool stop;
static std::mutex mutex_dev;

/* cleanup and exit */
void shutdown()
{
	print_log(LOG_DEVICE, "* Destroying streams\n");
	if (rxstream) {iio_stream_destroy(rxstream); }
	if (txstream) { iio_stream_destroy(txstream); }

	print_log(LOG_DEVICE, "* Destroying buffers\n");
	if (rxbuf) { iio_buffer_destroy(rxbuf); }
	if (txbuf) { iio_buffer_destroy(txbuf); }

	print_log(LOG_DEVICE, "* Destroying channel masks\n");
	if (rxmask) { iio_channels_mask_destroy(rxmask); }
	if (txmask) { iio_channels_mask_destroy(txmask); }

	print_log(LOG_DEVICE, "* Destroying context\n");
	if (ctx) { iio_context_destroy(ctx); }
	// exit(0);
}

static void handle_sig(int sig)
{
	print_log(LOG_DEVICE, "Waiting for process to finish... Got signal %d\n", sig);
	stop = true;
}

/* check return value of attr_write function */
static void errchk(int v, const char* what) {
	 if (v < 0) { fprintf(stderr, "Error %d writing to channel \"%s\"\nvalue may not be supported.\n", v, what); shutdown(); }
}

/* write attribute: long long int */
static void wr_ch_lli(struct iio_channel *chn, const char* what, long long val)
{
	const struct iio_attr *attr = iio_channel_find_attr(chn, what);

	errchk(attr ? iio_attr_write_longlong(attr, val) : -ENOENT, what);
}

/* write attribute: string */
static void wr_ch_str(struct iio_channel *chn, const char* what, const char* str)
{
	const struct iio_attr *attr = iio_channel_find_attr(chn, what);

	errchk(attr ? iio_attr_write_string(attr, str) : -ENOENT, what);
}


/* helper function generating channel names */
static char* get_ch_name(const char* type, int id)
{
	snprintf(tmpstr, sizeof(tmpstr), "%s%d", type, id);
	return tmpstr;
}


/* returns ad9361 phy device */
static struct iio_device* get_ad9361_phy(void)
{
	struct iio_device *dev =  iio_context_find_device(ctx, "ad9361-phy");
	IIO_ENSURE(dev && "No ad9361-phy found");
	return dev;
}

/* finds AD9361 streaming IIO devices */
static bool get_ad9361_stream_dev(enum iodev d, struct iio_device **dev)
{
	switch (d) {
	case TX: *dev = iio_context_find_device(ctx, "cf-ad9361-dds-core-lpc"); return *dev != NULL;
	case RX: *dev = iio_context_find_device(ctx, "cf-ad9361-lpc");  return *dev != NULL;
	default: IIO_ENSURE(0); return false;
	}
}

/* finds AD9361 streaming IIO channels */
static bool get_ad9361_stream_ch(enum iodev d, struct iio_device *dev, int chid, struct iio_channel **chn)
{
	*chn = iio_device_find_channel(dev, get_ch_name("voltage", chid), d == TX);
	if (!*chn)
		*chn = iio_device_find_channel(dev, get_ch_name("altvoltage", chid), d == TX);
	return *chn != NULL;
}

/* finds AD9361 phy IIO configuration channel with id chid */
static bool get_phy_chan(enum iodev d, int chid, struct iio_channel **chn)
{
	switch (d) {
	case RX: *chn = iio_device_find_channel(get_ad9361_phy(), get_ch_name("voltage", chid), false); return *chn != NULL;
	case TX: *chn = iio_device_find_channel(get_ad9361_phy(), get_ch_name("voltage", chid), true);  return *chn != NULL;
	default: IIO_ENSURE(0); return false;
	}
}
/* finds AD9361 local oscillator IIO configuration channels */
static bool get_lo_chan(enum iodev d, struct iio_channel **chn)
{
	switch (d) {
	 // LO chan is always output, i.e. true
	case RX: *chn = iio_device_find_channel(get_ad9361_phy(), get_ch_name("altvoltage", 0), true); return *chn != NULL;
	case TX: *chn = iio_device_find_channel(get_ad9361_phy(), get_ch_name("altvoltage", 1), true); return *chn != NULL;
	default: IIO_ENSURE(0); return false;
	}
}


/* applies streaming configuration through IIO */
bool cfg_ad9361_streaming_ch1(struct stream_cfg *cfg, enum iodev type, int chid)
{
	const struct iio_attr *attr;
	struct iio_channel *chn = NULL;

	// Configure phy and lo channels
	print_log(LOG_DEVICE, "* Acquiring AD9361 phy channel %d\n", chid);
	if (!get_phy_chan(type, chid, &chn)) {	return false; }

	attr = iio_channel_find_attr(chn, "rf_port_select");
	if (attr)
		errchk(iio_attr_write_string(attr, cfg->rfport), cfg->rfport);
	wr_ch_lli(chn, "rf_bandwidth",       cfg->bw_hz);
	wr_ch_lli(chn, "sampling_frequency", cfg->fs_hz);
	// wr_ch_lli(chn, "hardware_cain", cfg->fs_hz);
	
	// Configure LO channel
	print_log(LOG_DEVICE, "* Acquiring AD9361 %s lo channel\n", type == TX ? "TX" : "RX");
	if (!get_lo_chan(type, &chn)) { return false; }
	wr_ch_lli(chn, "frequency", cfg->lo_hz);
	return true;
}
#if 0
bool cfg_ad9361_streaming_ch2(struct stream_cfg *cfg, enum iodev type, int chid)
{
  struct iio_channel *chn = NULL;

  // Configure phy and lo channels
  printf("* Acquiring AD9361 phy channel %d\n", chid);
  if (!get_phy_chan(type, chid, &chn)) {  return false; }
  wr_ch_str(chn, "rf_port_select",     cfg->rfport);
  wr_ch_lli(chn, "rf_bandwidth",       cfg->bw_hz);
  wr_ch_lli(chn, "sampling_frequency", cfg->fs_hz);
//   iio_channel_attr_write(chn, "gain_control_mode", "manual");
  if (type == RX){   
      wr_ch_lli(chn, "hardwaregain", 20); // RX gain
  }

  else {
    wr_ch_lli(chn, "hardwaregain", 0);
  }

  // Configure LO channel
  printf("* Acquiring AD9361 %s lo channel\n", type == TX ? "TX" : "RX");
  if (!get_lo_chan(type, &chn)) { return false; }
  wr_ch_lli(chn, "frequency", cfg->lo_hz);
  return true;
}
#endif


bool cfg_ad9361_streaming_ch(struct stream_cfg *cfg, enum iodev type, int chid)
{
  const struct iio_attr *attr,*attr_gain;
  struct iio_channel *chn = NULL;

  // Configure phy and lo channels
  printf("* Acquiring AD9361 phy channel %d\n", chid);
  if (!get_phy_chan(type, chid, &chn)) {  return false; }

  attr = iio_channel_find_attr(chn, "rf_port_select");
  attr_gain = iio_channel_find_attr(chn, "gain_control_mode");

  if (attr)
    errchk(iio_attr_write_string(attr, cfg->rfport), cfg->rfport);

  if (attr_gain)
    errchk(iio_attr_write_string(attr_gain, "manual"), "manual");

  wr_ch_lli(chn, "rf_bandwidth",       cfg->bw_hz);
  

  wr_ch_lli(chn, "sampling_frequency", cfg->fs_hz);

  

  if(type == RX){
    wr_ch_lli(chn, "hardwaregain", cfg->power_gain);
  }
  else {
    wr_ch_lli(chn, "hardwaregain", cfg->power_gain);
  }

  // Configure LO channel
  printf("* Acquiring AD9361 %s lo channel\n", type == TX ? "TX" : "RX");
  if (!get_lo_chan(type, &chn)) { return false; }

  wr_ch_lli(chn, "frequency", cfg->lo_hz);

  //int ret = iio_channel_attr_write(chn,"gain_control_mode", "manual");

  return true;
}


void print_cfg(stream_cfg &txcfg){
	print_log(LOG_DEVICE, "bw_hz = %lld, fs_hz = %lld, lo_hz = %lld, rfport = %s\n", 
		txcfg.bw_hz, txcfg.fs_hz, txcfg.lo_hz, txcfg.rfport
	);
}
struct iio_device* context_tx(const char *addr_dev, struct stream_cfg &txcfg){
	struct iio_device *tx;
	print_log(LOG_DEVICE, "* Acquiring IIO TX context\n");

	if(!ctx)
		IIO_ENSURE((ctx = iio_create_context(NULL, addr_dev)) && "No context");
	
	IIO_ENSURE(iio_context_get_devices_count(ctx) > 0 && "No devices");
	print_log(LOG_DEVICE,"* Acquiring AD9361 tx devices\n");
	IIO_ENSURE(get_ad9361_stream_dev(TX, &tx) && "No tx dev found");
	print_log(LOG_DEVICE,"* Configuring AD9361 for tx\n");
	IIO_ENSURE(cfg_ad9361_streaming_ch(&txcfg, TX, 0) && "TX port 0 not found");
	print_log(LOG_DEVICE,"* Initializing AD9361 IIO tx channels\n");
	IIO_ENSURE(get_ad9361_stream_ch(TX, tx, 0, &tx0_i) && "TX chan i not found");
	IIO_ENSURE(get_ad9361_stream_ch(TX, tx, 1, &tx0_q) && "TX chan q not found");
	
	txmask = iio_create_channels_mask(iio_device_get_channels_count(tx));
	if (!txmask) {
		print_log(LOG_DEVICE, "Unable to alloc channels mask\n");
		shutdown();
	}
	print_log(LOG_DEVICE, "* Enabling IIO streaming channels\n");
	iio_channel_enable(tx0_i, txmask);
	iio_channel_enable(tx0_q, txmask);

	return tx;
}

struct iio_device* context_rx(const char *addr_dev, struct stream_cfg &rxcfg){
	struct iio_device *rx;
	print_log(LOG_DEVICE, "* Acquiring IIO TX context\n");
	if(!ctx)
		IIO_ENSURE((ctx = iio_create_context(NULL, addr_dev)) && "No context");
	
	return rx;
}


size_t rx_sample_sz, tx_sample_sz;

int init_device_TRX(config_device &cfg) {
	print_log(LOG_DEVICE, "* Acquiring IIO context\n");
	
	const char *addr = cfg.ip;
	struct stream_cfg *rxcfg = &cfg.rx_cfg;
	struct stream_cfg *txcfg = &cfg.tx_cfg;
	int BLOCK_SIZE_rx;
	int BLOCK_SIZE_tx;

	if(cfg.block_size > 0) {
		BLOCK_SIZE_rx = cfg.block_size;
		BLOCK_SIZE_tx = cfg.block_size;
	} else {
		BLOCK_SIZE_rx = rxcfg->block_size * sizeof(mod_symbol);
		BLOCK_SIZE_tx = txcfg->block_size * sizeof(mod_symbol);
	}
	

	int err;

	IIO_ENSURE((ctx = iio_create_context(NULL, addr)) && "No context");
	
	if(!ctx) {
		print_log(LOG, "[%s:%d] !ctx\n", __func__, __LINE__);
	}
	IIO_ENSURE(iio_context_get_devices_count(ctx) > 0 && "No devices");

	print_log(LOG_DEVICE, "* Acquiring AD9361 streaming devices\n");
	IIO_ENSURE(get_ad9361_stream_dev(TX, &tx) && "No tx dev found");
	IIO_ENSURE(get_ad9361_stream_dev(RX, &rx) && "No rx dev found");

	print_log(LOG_DEVICE, "* Configuring AD9361 for streaming\n");
	IIO_ENSURE(cfg_ad9361_streaming_ch(rxcfg, RX, 0) && "RX port 0 not found");
	IIO_ENSURE(cfg_ad9361_streaming_ch(txcfg, TX, 0) && "TX port 0 not found");

	print_log(LOG_DEVICE, "* Initializing AD9361 IIO streaming channels\n");
	IIO_ENSURE(get_ad9361_stream_ch(RX, rx, 0, &rx0_i) && "RX chan i not found");
	IIO_ENSURE(get_ad9361_stream_ch(RX, rx, 1, &rx0_q) && "RX chan q not found");
	IIO_ENSURE(get_ad9361_stream_ch(TX, tx, 0, &tx0_i) && "TX chan i not found");
	IIO_ENSURE(get_ad9361_stream_ch(TX, tx, 1, &tx0_q) && "TX chan q not found");

	rxmask = iio_create_channels_mask(iio_device_get_channels_count(rx));
	if (!rxmask) {
		fprintf(stderr, "Unable to alloc channels mask\n");
		shutdown();
	}

	txmask = iio_create_channels_mask(iio_device_get_channels_count(tx));
	if (!txmask) {
		fprintf(stderr, "Unable to alloc channels mask\n");
		shutdown();
	}

	print_log(LOG_DEVICE, "* Enabling IIO streaming channels\n");
	iio_channel_enable(rx0_i, rxmask);
	iio_channel_enable(rx0_q, rxmask);
	iio_channel_enable(tx0_i, txmask);
	iio_channel_enable(tx0_q, txmask);

	print_log(LOG_DEVICE, "* Creating non-cyclic IIO buffers with 1 MiS\n");
	rxbuf = iio_device_create_buffer(rx, 0, rxmask);
	err = iio_err(rxbuf);
	if (err) {
		rxbuf = NULL;
		dev_perror(rx, err, "Could not create RX buffer");
		shutdown();
	}
	txbuf = iio_device_create_buffer(tx, 0, txmask);
	err = iio_err(txbuf);
	if (err) {
		txbuf = NULL;
		dev_perror(tx, err, "Could not create TX buffer");
		shutdown();
	}

	rxstream = iio_buffer_create_stream(rxbuf, 4, BLOCK_SIZE_rx);
	err = iio_err(rxstream);
	if (err) {
		rxstream = NULL;
		dev_perror(rx, iio_err(rxstream), "Could not create RX stream");
		shutdown();
	}

	txstream = iio_buffer_create_stream(txbuf, 4, BLOCK_SIZE_tx);
	err = iio_err(txstream);
	if (err) {
		txstream = NULL;
		dev_perror(tx, iio_err(txstream), "Could not create TX stream");
		shutdown();
	}

	rx_sample_sz = iio_device_get_sample_size(rx, rxmask);
	tx_sample_sz = iio_device_get_sample_size(tx, txmask);
	print_log(LOG, "[%s:%d] configure device\n", __func__, __LINE__);
	return 0;
}


int write_to_device_buffer(const void *data, int size){
	int16_t *p_dat, *p_end;
	ptrdiff_t p_inc;
	const struct iio_block *txblock;
	int err = 0;
	mutex_dev.lock();
	txblock = iio_stream_get_next_block(txstream);
	// print_log(CONSOLE, "%s:%d - %p\n", __func__, __LINE__, txblock);
	err = iio_err(txblock);
	if (err) {
		print_log(LOG_DEVICE, "ERROR: Unable to send block");
		return -1;
	}
	p_inc = tx_sample_sz;
	// print_log(CONSOLE, "tx_sample_sz = %d\n", tx_sample_sz);
	p_end = (int16_t*)iio_block_end(txblock);
	int iter = 0;
	float *dataf = (float*)data; 
	for (p_dat = (int16_t*)iio_block_first(txblock, tx0_i); p_dat < p_end, iter < size;
			p_dat += p_inc / sizeof(*p_dat), iter += 2) {
		// p_dat[0] = 4000; /* Real (i) */
		// p_dat[1] = 4000; /* Imag (q) */
		// print_log(CONSOLE, "[%d] %f %f\n", iter, dataf[iter], dataf[iter + 1]);
#if 1
		p_dat[0] = dataf[iter];
		p_dat[1] = dataf[iter + 1];
#else
		p_dat[0] = dataf[iter] * 2000;
		p_dat[1] = dataf[iter + 1] * 2000;
#endif
		// DEBUG_LINE
	}
	mutex_dev.unlock();
	return 0;
}

int read_to_device_buffer(const void *data, int size){
	int16_t *p_dat, *p_end;
	ptrdiff_t p_inc;
	const struct iio_block *rxblock;
	p_inc = rx_sample_sz;
	mutex_dev.lock();
	rxblock = iio_stream_get_next_block(rxstream);
	p_end = (int16_t*)iio_block_end(rxblock);
	int err = 0;
	
	err = iio_err(rxblock);
	if (err) {
		print_log(LOG_DEVICE, "ERROR: Unable to receive block");
		return -1;
	}
	float *buffer = (float*)data;
	int iter = 0;
	for (p_dat = (int16_t*)iio_block_first(rxblock, rx0_i); p_dat < p_end, iter < size;
			p_dat += p_inc / sizeof(*p_dat), iter++) {
		/* Example: swap i and q */
		int16_t i = p_dat[0];
		int16_t q = p_dat[1];
		buffer[0] = i;
		buffer[1] = q;
		buffer += 2;
		// if(buffer[0] > 4000 || buffer[0] < -4000)
		// 	print_log(LOG_DATA, "%f, %f\n", (float)i, (float)q);
		// print_log(CONSOLE, "%f, %f\n", (float)i, (float)q);
	}
	mutex_dev.unlock();
	return 0;
}
