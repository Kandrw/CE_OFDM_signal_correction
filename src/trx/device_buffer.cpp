#include "device_buffer.hpp"

#include "device_api.hpp"
// #include "ofdm.hpp"


struct dev_components {
    int (*init_dev)(config_device &cfg_dev);
    int (*deinit_dev)(config_device &cfg_dev);
    int (*send_dev)(const VecSymbolMod &samples, size_t size);
    int (*recv_dev)(VecSymbolMod &samples, size_t size);
    
};

int DeviceBuffer::initialization(context &ctx) {
    return DEVICE_PHY::DeviceTRX::initialization(ctx.cfg_device);
}

int DeviceBuffer::deinitialization(context &ctx) {
    return DEVICE_PHY::DeviceTRX::deinitialization();
}

int DeviceBuffer::send_msg(const msg_buffer *msg) {
    // return DEVICE_PHY::DeviceTRX::send_samples((void*)msg->data, size * sizeof(mod_symbol));
}


int DeviceBuffer::recv_msg(msg_buffer *msg) {
    // return DEVICE_PHY::DeviceTRX::send_samples((void*)msg->data, size * sizeof(mod_symbol));
}
























