#include "components.hpp"

#include <cstring>

#include "../trx/device_api.hpp"
#include "channel_model.hpp"

using namespace ATTR_SERVICE;

#define TMP_BUFFER_RX 80000 * sizeof(mod_symbol)
#define TMP_BUFFER_TX 20000 * sizeof(mod_symbol)

#define SIZE_PREAMBULA 10
#define SIZE_BUFFER 1024

#include <output.hpp>
#include <complex_container.hpp>
#include <signal_processing.hpp>
#include <configure.hpp>

// #define FILE_CONFIG 1

struct dev_components {
    int (*init_dev)(context &cfg_dev);
    int (*deinit_dev)(context &cfg_dev);
    int (*send_dev)(const VecSymbolMod &samples, size_t size);
    int (*recv_dev)(VecSymbolMod &samples, size_t size);
    
};

static dev_components dev_com;

static config_device convert_config_device(const YAML::Node &cfg) {
    config_device cfg_out = {
        .rx_cfg = {
            .bw_hz = cfg["device_phy"]["rx"]["bw_hz"].as<long long>(),
            .fs_hz = cfg["device_phy"]["rx"]["fs_hz"].as<long long>(),
            .lo_hz = cfg["device_phy"]["rx"]["lo_hz"].as<long long>(),
            .block_size = cfg["device_phy"]["rx"]["block_size"].as<unsigned int>()
        },
        .tx_cfg = {
            .bw_hz = cfg["device_phy"]["tx"]["bw_hz"].as<long long>(),
            .fs_hz = cfg["device_phy"]["tx"]["fs_hz"].as<long long>(),
            .lo_hz = cfg["device_phy"]["tx"]["lo_hz"].as<long long>(),
            .block_size = cfg["device_phy"]["tx"]["block_size"].as<unsigned int>()
        },
        1000000
    };
    memcpy(cfg_out.ip, cfg["address"].as<std::string>().c_str(), sizeof(cfg_out.ip));
    memcpy(cfg_out.rx_cfg.rfport, cfg["device_phy"]["rx"]["rfport"].as<std::string>().c_str(),
        sizeof(cfg_out.rx_cfg.rfport));
    memcpy(cfg_out.tx_cfg.rfport, cfg["device_phy"]["tx"]["rfport"].as<std::string>().c_str(),
        sizeof(cfg_out.tx_cfg.rfport));
    return cfg_out;
}

int ATTR_SERVICE::init_log_system(const char *file_log) {
    if(init_log(file_log)) {
        return STATUS_FAIL;
    }
    return STATUS_ACCESS;
}

int ATTR_SERVICE::deinit_log_system() {
    deinit_log();
    return STATUS_ACCESS;
}

static int init_device(context &cfg_dev) {
    config_device cfg = convert_config_device(cfg_dev.cfg);
    if(DEVICE_PHY::DeviceTRX::initialization(cfg)){
        print_log(CONSOLE, "[%s:%d] Error: initialization device, exit program\n",
            __func__, __LINE__);
        return STATUS_FAIL;
    }
    return STATUS_ACCESS;
}

static int deinit_device(context &cfg_dev) {
    DEVICE_PHY::DeviceTRX::deinitialization();
    return STATUS_ACCESS;
}

static int read_radio(VecSymbolMod &samples, size_t size) {
    return DEVICE_PHY::DeviceTRX::recv_samples(samples, size);
}

static int write_radio(const VecSymbolMod &samples, size_t size) {
    return DEVICE_PHY::DeviceTRX::send_samples((void*)&samples[0], size);
}

int ATTR_SERVICE::init_components(const std::string &key) {
    if(key == "sdr") {
        dev_com = {
            .init_dev = init_device,
            .deinit_dev = deinit_device,
            .send_dev = write_radio,
            .recv_dev = read_radio
        };
    } else if(key == "model") {
        dev_com = {
            .init_dev = CHANNEL_MODEL::model_channel_init,
            .deinit_dev = CHANNEL_MODEL::model_channel_deinit,
            .send_dev = CHANNEL_MODEL::write_channel,
            .recv_dev = CHANNEL_MODEL::read_channel
        };
    } else {
        print_log(ERROR_OUT, "Error set env: %s\n", key.c_str());
        return STATUS_FAIL;
    }
    print_log(LOG, "[%s:%d] set env: %s\n", __func__, __LINE__, key.c_str());
    return STATUS_ACCESS;
}

int ATTR_SERVICE::init_system(context &cfg_dev) {
    return dev_com.init_dev(cfg_dev);
}

int ATTR_SERVICE::deinit_system(context &cfg_dev) {
    return dev_com.deinit_dev(cfg_dev);
}

int ATTR_SERVICE::write_samples(const VecSymbolMod &samples, size_t size) {
    return dev_com.send_dev(samples, size);
}

int ATTR_SERVICE::read_samples(VecSymbolMod &samples, size_t size) {
    return dev_com.recv_dev(samples, size);
}



int ATTR_SERVICE::send_msg(const context &ctx, const void *buffer, int size) {

    u_char buffer_tx[SIZE_BUFFER] = {};
    u_char *data = buffer_tx;
    header_phy *hv1 = (header_phy*)buffer;
    int size_tx = sizeof(header_phy) + size + SIZE_PREAMBULA;
    hv1->id = ctx.id;
    hv1->mod = static_cast<u_char>(DIGITAL_SIGNAL_PROCESSING::string_to_TypeModulation(
        ctx.cfg["type_modulation"].as<std::string>()));
    hv1->size = size;
    memcpy(data, buffer, size);
    
    
}

int ATTR_SERVICE::recv_msg(const context &ctx, void *buffer, int size) {

    


}
