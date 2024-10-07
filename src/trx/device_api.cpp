#include "device_api.hpp"

#include <errno.h>
#include <cstring>
#include <ctime>
#include <unistd.h>
#include <thread>

#ifdef M_FOUND_LIBIIO
#include <iio.h>
#endif


#include "ad9361/device_ad9361.hpp"
#include <output.hpp>
// #include "../output/output.hpp"

// #define IIO_ENSURE(expr) { \
// 	if (!(expr)) { \
// 		(void) print_log(ERROR_OUT, "assertion failed (%s:%d)\n", __FILE__, __LINE__); \
// 		(void) abort(); \
// 	} \
// }
struct iio_device *DeviceTRX::tx_dev = NULL;
struct iio_device *DeviceTRX::rx_dev = NULL;

unsigned int DeviceTRX::samples_count_rx = 5760;
unsigned int DeviceTRX::samples_count_tx = 5760;


static std::thread thr;

static int run_while_send_samples = false;
static int end_while_send_samples = true;



// struct iio_context *DeviceTRX::ctx = NULL;
    // static struct iio_channel *rx0_i;
    // static struct iio_channel *rx0_q;
    // static struct iio_channel *tx0_i;
    // static struct iio_channel *tx0_q;
    // static struct iio_buffer  *rxbuf;
    // static struct iio_buffer  *txbuf;
    // static struct iio_stream  *rxstream;
    // static struct iio_stream  *txstream;
    // static struct iio_channels_mask *rxmask;
    // static struct iio_channels_mask *txmask;


template<typename T> bool cmp_flags(T a, T b)
{
    int A = static_cast<int>(a);
    int B = static_cast<int>(b);
    print_log(LOG, "[%s:%d] A = %d, B = %d\n", __func__, __LINE__, A, B);
    if(A == (B & A))
        return true;
    return false;
}

int DeviceTRX::initialization(config_device &cfg_param){
    print_log(CONSOLE, "[%s:%d] op = %d\n", __func__, __LINE__, cfg_param.operation);

    if(cmp_flags(iodev::RX, cfg_param.operation)){
        rx_dev = context_rx(cfg_param.ip, cfg_param.rx_cfg);
        if(!rx_dev){
            print_log(ERROR_OUT, "[%s, %d] Error create rx_dev: %s\n", __func__, __LINE__, cfg_param.ip);
            return -1;
        }
        if(init_rx_buffer()){
            print_log(LOG, "[%s:%d] Error init_rx_buffer\n", __func__, __LINE__);
            return -1;
        }
        
        print_log(LOG, "[%s:%d] create buffer RX\n", __func__, __LINE__);
    }
    if(cmp_flags(iodev::TX, cfg_param.operation)){
        tx_dev = context_tx(cfg_param.ip, cfg_param.tx_cfg);

        if(!tx_dev){
            print_log(ERROR_OUT, "[%s, %d] Error create tx_dev: %s\n", cfg_param.ip);
            return -1;
        }
        if(init_tx_buffer()){
            print_log(LOG, "[%s:%d] Error init_tx_buffer\n", __func__, __LINE__);
            return -1;
        }
        print_log(LOG, "[%s:%d] create buffer TX\n", __func__, __LINE__);
    }
    return 0;
}
int DeviceTRX::deinitialization(){
    print_log(CONSOLE, "[%s:%d]\n", __func__, __LINE__);
    if(run_while_send_samples){
        run_while_send_samples = false;
        // thr.join();
        while(!end_while_send_samples);
    }
    
    shutdown();
    return 0;
}

int DeviceTRX::init_tx_buffer(){
    return device_create_buffer( static_cast<int>(iodev::TX), *tx_dev, samples_count_tx, false);
}

int DeviceTRX::init_rx_buffer(){
    return device_create_buffer( static_cast<int>(iodev::RX), *rx_dev, samples_count_rx, false);
}

int DeviceTRX::send_samples(void *samples, size_t size){
    if(tx_dev){
        return write_to_device_buffer(samples, size);
    }
        
    print_log(CONSOLE, "[%s:%d] Error: no init txbuf\n", __func__, __LINE__);
    return -1;
}
int thread_send_samples(void *samples, size_t size){
    int i = 0;
    while(run_while_send_samples){
        write_to_device_buffer(samples, size);
        print_log(LOG, "[%s:%d] [%d] send\n", __func__, __LINE__, i++);
        
    }
    end_while_send_samples = true;
    return 0;
}
int DeviceTRX::while_send_samples(void *samples, size_t size){
    if(tx_dev){
        if(!run_while_send_samples){
            run_while_send_samples = true;
            end_while_send_samples = false;
            thr = std::thread(thread_send_samples, samples, size);
            thr.detach();
            sleep(1);
            return 0;
        }
        print_log(CONSOLE, "[%s:%d] Error: already run\n", __func__, __LINE__);
        // thr = std::threadthread_send_samples}; //thread_send_samples( samples, size);
        return 0;
    }
    // run_while_send_samples = false;
    print_log(CONSOLE, "[%s:%d] Error: no init txbuf\n", __func__, __LINE__);
    return -1;
}

int DeviceTRX::recv_samples(void *samples, size_t size){
    if(rx_dev){
        return read_to_device_buffer(samples, size);
    } 
    print_log(CONSOLE, "[%s:%d] Error: no init rxbuf\n", __func__, __LINE__);
    return -1;
}




