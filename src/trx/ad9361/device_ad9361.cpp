#include "device_ad9361.hpp"

#include <stdbool.h>
#include <stdint.h>
#include <string.h>
#include <signal.h>
#include <stdio.h>
#include <iio.h>


#include "../types_trx.hpp"

#include <output.hpp>

#define IIO_ENSURE(expr) { \
	if (!(expr)) { \
		(void) fprintf(stdout, "assertion failed (%s:%d)\n", __FILE__, __LINE__); \
		(void) abort(); \
	} \
}

enum iodev { RX = 1, TX };

/* static scratch mem for strings */
char tmpstr[64];

static struct iio_context *ctx   = NULL;
static struct iio_channel *rx0_i = NULL;
static struct iio_channel *rx0_q = NULL;
static struct iio_channel *tx0_i = NULL;
static struct iio_channel *tx0_q = NULL;
static struct iio_buffer  *rxbuf = NULL;
static struct iio_buffer  *txbuf = NULL;

static u_int64_t samples_count_rx;
static u_int64_t samples_count_tx;


static bool stop;

/* cleanup and exit */
void shutdown()
{
	print_log(LOG_DEVICE, "* Destroying buffers\n");
	if (rxbuf) { iio_buffer_destroy(rxbuf); }
	if (txbuf) { iio_buffer_destroy(txbuf); }

	print_log(LOG_DEVICE, "* Disabling streaming channels\n");
	if (rx0_i) { iio_channel_disable(rx0_i); }
	if (rx0_q) { iio_channel_disable(rx0_q); }
	if (tx0_i) { iio_channel_disable(tx0_i); }
	if (tx0_q) { iio_channel_disable(tx0_q); }

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
	errchk(iio_channel_attr_write_longlong(chn, what, val), what);
}

/* write attribute: string */
static void wr_ch_str(struct iio_channel *chn, const char* what, const char* str)
{
	errchk(iio_channel_attr_write(chn, what, str), what);
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
bool cfg_ad9361_streaming_ch(struct stream_cfg *cfg, enum iodev type, int chid)
{
	struct iio_channel *chn = NULL;

	// Configure phy and lo channels
	print_log(LOG_DEVICE, "* Acquiring AD9361 phy channel %d\n", chid);
	if (!get_phy_chan(type, chid, &chn)) {	return false; }
	wr_ch_str(chn, "rf_port_select",     cfg->rfport);
	wr_ch_lli(chn, "rf_bandwidth",       cfg->bw_hz);
	wr_ch_lli(chn, "sampling_frequency", cfg->fs_hz);
	
#if 0
	iio_channel_attr_write(chn, "gain_control_mode", "fast_attack");
#else
	// iio_channel_attr_write(chn, "gain_control_mode", "slow_attack");
	iio_channel_attr_write(chn, "gain_control_mode", "manual");
	
	
	if (type == RX){   
    	wr_ch_lli(chn, "hardwaregain", 20); // RX gain
	}

	else {
		wr_ch_lli(chn, "hardwaregain", 0);
	}
#endif
	// Configure LO channel
	print_log(LOG_DEVICE, "* Acquiring AD9361 %s lo channel\n", type == TX ? "TX" : "RX");
	if (!get_lo_chan(type, &chn)) { return false; }
	wr_ch_lli(chn, "frequency", cfg->lo_hz);
	return true;
}

void print_cfg(stream_cfg &txcfg){
	print_log(LOG_DEVICE, "bw_hz = %lld, fs_hz = %lld, lo_hz = %lld, rfport = %s\n", 
		txcfg.bw_hz, txcfg.fs_hz, txcfg.lo_hz, txcfg.rfport
	);
}
struct iio_device* context_tx(const char *addr_dev, struct stream_cfg &txcfg){
		// Streaming devices
	struct iio_device *tx;


	// RX and TX sample counters
	// size_t nrx = 0;
	// size_t ntx = 0;
	print_cfg(txcfg);
	// Listen to ctrl+c and IIO_ENSURE
	// signal(SIGINT, handle_sig);


	// TX stream config
	// txcfg.bw_hz = MHZ(2); // 1.5 MHz rf bandwidth
	// txcfg.fs_hz = MHZ(2.5);   // 2.5 MS/s tx sample rate
	// txcfg.lo_hz = GHZ(2.3); // 2.3 GHz rf frequency
	// txcfg.rfport = "A"; // port A (select for rf freq.)


	print_log(LOG_DEVICE, "* Acquiring IIO context\n");
	if (!addr_dev) {
		IIO_ENSURE((ctx = iio_create_default_context()) && "No context");
	}
	else {
		IIO_ENSURE((ctx = iio_create_context_from_uri(addr_dev)) && "No context");
	}
	IIO_ENSURE(iio_context_get_devices_count(ctx) > 0 && "No devices");


	print_log(LOG_DEVICE, "* Acquiring AD9361 streaming devices\n");
	IIO_ENSURE(get_ad9361_stream_dev(TX, &tx) && "No tx dev found");


	print_log(LOG_DEVICE, "* Configuring AD9361 for streaming\n");

	IIO_ENSURE(cfg_ad9361_streaming_ch(&txcfg, TX, 0) && "TX port 0 not found");

	print_log(LOG_DEVICE, "* Initializing AD9361 IIO streaming channels\n");

	IIO_ENSURE(get_ad9361_stream_ch(TX, tx, 0, &tx0_i) && "TX chan i not found");
	IIO_ENSURE(get_ad9361_stream_ch(TX, tx, 1, &tx0_q) && "TX chan q not found");

	print_log(LOG_DEVICE, "* Enabling IIO streaming channels\n");

	iio_channel_enable(tx0_i);
	iio_channel_enable(tx0_q);

	print_log(LOG_DEVICE, "* Creating non-cyclic IIO buffers with 1 MiS\n");

	return tx;
}

struct iio_device* context_rx(const char *addr_dev, struct stream_cfg &rxcfg){
		// Streaming devices

	struct iio_device *rx;

	// RX and TX sample counters
	// size_t nrx = 0;
	// size_t ntx = 0;
	print_cfg(rxcfg);

	// Listen to ctrl+c and IIO_ENSURE
	// signal(SIGINT, handle_sig);

	// RX stream config
	// rxcfg.bw_hz = MHZ(2);   // 2 MHz rf bandwidth
	// rxcfg.fs_hz = MHZ(2.5);   // 2.5 MS/s rx sample rate
	// rxcfg.lo_hz = GHZ(2.3); // 2.3 GHz rf frequency
	// rxcfg.rfport = "A_BALANCED"; // port A (select for rf freq.)
	print_log(LOG_DEVICE, "[%s:%d]\n", __func__, __LINE__);
	print_log(LOG_DEVICE, "* Acquiring IIO context\n");
	if (!addr_dev) {
		IIO_ENSURE((ctx = iio_create_default_context()) && "No context");
	}
	else {
		IIO_ENSURE((ctx = iio_create_context_from_uri(addr_dev)) && "No context");
	}
	IIO_ENSURE(iio_context_get_devices_count(ctx) > 0 && "No devices");

	print_log(LOG_DEVICE, "* Configuring AD9361 for streaming\n");
	IIO_ENSURE(cfg_ad9361_streaming_ch(&rxcfg, RX, 0) && "RX port 0 not found");

	print_log(LOG_DEVICE, "* Acquiring AD9361 streaming devices\n");

	IIO_ENSURE(get_ad9361_stream_dev(RX, &rx) && "No rx dev found");

	print_log(LOG_DEVICE, "* Initializing AD9361 IIO streaming channels\n");
	IIO_ENSURE(get_ad9361_stream_ch(RX, rx, 0, &rx0_i) && "RX chan i not found");
	IIO_ENSURE(get_ad9361_stream_ch(RX, rx, 1, &rx0_q) && "RX chan q not found");

	print_log(LOG_DEVICE, "* Enabling IIO streaming channels\n");
	iio_channel_enable(rx0_i);
	iio_channel_enable(rx0_q);

	print_log(LOG_DEVICE, "* Creating non-cyclic IIO buffers with 1 MiS\n");

	return rx;
}

int device_create_buffer(int d, struct iio_device &dev, unsigned int samples_count, bool cyclic){
	int res = 0;
	print_log(LOG, "[%s:%d] START, d = %d\n", __func__, __LINE__, d);
	switch(d){
	case RX:
		rxbuf = iio_device_create_buffer(&dev, samples_count, cyclic);
		if (!rxbuf) {
			perror("Could not create RX buffer");
			res = -1;
		}
		print_log(LOG, "[%s:%d] create RX buffer\n", __func__, __LINE__);
		samples_count_rx = samples_count;
		break;
	case TX:
		txbuf = iio_device_create_buffer(&dev, samples_count, cyclic);
		if (!txbuf) {
			perror("Could not create TX buffer");
			res = -1;
		}
		samples_count_tx = samples_count;
		break;
		
	}
	return res;
}
int write_to_device_buffer1(const void *data, int size){
	u_int64_t i, i2;
	// u_int64_t shift_size = 0;
	ssize_t nbytes_tx;

	nbytes_tx = iio_buffer_push(txbuf);
	// print_log(LOG, "[%s:%d] \n", __func__, __LINE__);
	for(i = 0; i < (u_int64_t)size; i += samples_count_tx){
		for(i2 = i; i2 < i + samples_count_tx; i2 += 2){
			// print_log(LOG_DATA, "[%s:%d] %f + %f\n", __func__, __LINE__, *(float*)((float*)data + i2), *(float*)((float*)data + i2 + 1));
		}
		// print_log(LOG_DEVICE, "[%s:%d]\n", __func__, __LINE__);
		int16_t *buf_data = (int16_t *)iio_buffer_start(txbuf);
		memcpy(buf_data, (void*)((u_int16_t*)data + i), samples_count_tx);

		// iio_buffer_push(txbuf);
	}
	return 0;
}
int write_to_device_buffer(const void *data, int size){
	char *p_dat, *p_end;
    ptrdiff_t p_inc;
	ssize_t nbytes_tx;
	int *step = (int*)data;
	nbytes_tx = iio_buffer_push(txbuf);
	if (nbytes_tx < 0) { 
		print_log(LOG_DEVICE, "Error pushing buf %d\n", (int) nbytes_tx); 
		// shutdown();
		return -1; 
	}
	p_inc = iio_buffer_step(txbuf);
	p_end = (char*)iio_buffer_end(txbuf);
	int i = 0;
	for (p_dat = (char *)iio_buffer_first(txbuf, tx0_q); p_dat < p_end, i < size; p_dat += p_inc, i += 2) {
			
			((int16_t*)p_dat)[0] = *(step);
			((int16_t*)p_dat)[1] = *(step + 1);
			print_log(LOG_DEVICE, "[%s:%d] %d - %d\n", __func__, __LINE__, *step, *(step + 1));
		step += 2;
	}
	return 0;
}
int read_to_device_buffer(const void *data, int size){
	char *p_dat, *p_end;
    ptrdiff_t p_inc;
	ssize_t nbytes_rx;
	// int16_t buffer[(int)1e4];

	print_log(LOG, "[%s:%d] START\n", __func__, __LINE__);
	if(!rxbuf){
		print_log(LOG, "[%s:%d] Error: no init rxbuf\n", __func__, __LINE__);
		return -1;
	}
	nbytes_rx = iio_buffer_refill(rxbuf);
	if (nbytes_rx < 0) { print_log(CONSOLE, "Error refilling buf %d\n",(int) nbytes_rx); shutdown(); }

	p_inc = iio_buffer_step(rxbuf);
	p_end = (char*)iio_buffer_end(rxbuf);
	int count = 0;
	for (p_dat = (char *)iio_buffer_first(rxbuf, rx0_i); p_dat < p_end; p_dat += p_inc) {

		const int16_t i = ((int16_t*)p_dat)[0]; // Real (I)
		const int16_t q = ((int16_t*)p_dat)[1]; // Imag (Q)
		((int16_t*)p_dat)[0] = q;
		((int16_t*)p_dat)[1] = i;
		print_log(LOG_DATA, "[%s:%d] %f + %f\n", 
			__func__, __LINE__, (float)i, (float)q);
		// print_log(LOG_DATA, "[%s:%d] %d + %d\n", __func__, __LINE__, i, q);
		// printf("%d %d\n", i, q);
		count++;
	}
	print_log(LOG, "[%s:%d] count = %d\n", __func__, __LINE__, count);
	return 0;
}

void send_data(struct iio_device *dev, int16_t *data, size_t size) {
    struct iio_buffer *buf = iio_device_create_buffer(dev, size * sizeof(int16_t), false);
    
    if (buf) {
        int16_t *buf_data = (int16_t *)iio_buffer_start(buf);
        memcpy(buf_data, data, size * sizeof(int16_t));
        
        iio_buffer_push(buf);
        // iio_buffer_free(buf);
    }
}