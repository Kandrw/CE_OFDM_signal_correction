#pragma once

#include <yaml-cpp/yaml.h>
#include <signal_processing.hpp>

#define MHZ(x) ((long long)(x*1000000.0 + .5))
#define GHZ(x) ((long long)(x*1000000000.0 + .5))

/* common RX and TX streaming params */
struct stream_cfg {
    long long bw_hz; // Analog banwidth in Hz
    long long fs_hz; // Baseband sample rate in Hz
    long long lo_hz; // Local oscillator frequency in Hz
    char rfport[20]; // Port name
    unsigned int block_size;
    long long power_gain;
};

struct config_device{
    char ip[32];
    stream_cfg rx_cfg;
    stream_cfg tx_cfg; 
    unsigned int block_size;
};


struct config_x {
    unsigned int size;
    unsigned int thread_count;
    unsigned int interval_of_with_dev;
};

struct config_buffer {
    config_x rx;
    config_x tx;
};

enum class STATUS_BUFFER {
    SUCCESS,
    FILLED_IN,
    FAIL,
};

struct msg_buffer {
    int id;
    int size;
    void *data;
};


enum class ENV_DEV {
    SDR,
    MODEL
};


struct context {
    u_int16_t id;
    YAML::Node cfg;
    DIGITAL_SIGNAL_PROCESSING::OFDM_params ofdm_param;
    config_device cfg_device;
    config_buffer cfg_buf;
};


