#pragma once






#include <iio.h>

#include "types_trx.hpp"


enum class iodev { RX = 1, TX, TRX };


class DeviceFunc{


};


struct device_param{
    const char *tx_dev;
    const char *rx_dev;
};

struct config_device{
    const char *ip;
    iodev operation;
    stream_cfg rx_cfg;
    stream_cfg tx_cfg; 
    
};

class DeviceTRX{

public:

    static int initialization(config_device &cfg_param);

    static int deinitialization();
    static int send_samples(void *samples, size_t size);
    static int while_send_samples(void *samples, size_t size);
    static int recv_samples(void *samples, size_t size);
private:
    static int init_tx_buffer();
    static int init_rx_buffer();
    
private:
    
    // std::thread *thr = nullptr;
    // static std::thread thr;

    static unsigned int samples_count_rx;
    static unsigned int samples_count_tx;
    
    static struct iio_device *tx_dev;
	static struct iio_device *rx_dev;

};







