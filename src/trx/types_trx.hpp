#pragma once

// #define BLOCK_SIZE (1024 * 1024)
#define MHZ(x) ((long long)(x*1000000.0 + .5))
#define GHZ(x) ((long long)(x*1000000000.0 + .5))


// namespace DEVICE_PHY {

    /* common RX and TX streaming params */
    struct stream_cfg {
        long long bw_hz; // Analog banwidth in Hz
        long long fs_hz; // Baseband sample rate in Hz
        long long lo_hz; // Local oscillator frequency in Hz
        const char* rfport; // Port name
        unsigned int block_size;
    };

    struct config_device{
        const char *ip;
        stream_cfg rx_cfg;
        stream_cfg tx_cfg; 
        unsigned int block_size;
    };

// };