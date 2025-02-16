#include "components.hpp"

#include <cstring>
#include <ctime>
#include <unistd.h>

// #include "ofdm.hpp"
// #include "../trx/device_api.hpp"
#include "channel_model.hpp"
#include "../loaders/load_data.hpp"
#include "../trx/device_buffer.hpp"

using namespace ATTR_SERVICE;

#define TMP_BUFFER_RX 80000 * sizeof(mod_symbol)
#define TMP_BUFFER_TX 20000 * sizeof(mod_symbol)

#define SIZE_PREAMBULA 10
#define SIZE_BUFFER 1024

#include <output.hpp>
#include <configure.hpp>

struct dev_components {
    int (*init_dev)(context &cfg_dev);
    int (*deinit_dev)(context &cfg_dev);
    int (*send_dev)(const msg_buffer *msg);
    int (*recv_dev)(msg_buffer *msg);
    
};

static dev_components dev_com;

static config_device convert_config_device(const YAML::Node &cfg) {
    config_device cfg_out = {
        .rx_cfg = {
            .bw_hz = cfg["device_phy"]["rx"]["bw_hz"].as<long long>(),
            .fs_hz = cfg["device_phy"]["rx"]["fs_hz"].as<long long>(),
            .lo_hz = cfg["device_phy"]["rx"]["lo_hz"].as<long long>(),
            .block_size = cfg["device_phy"]["rx"]["block_size"].as<unsigned int>(),
            .power_gain = cfg["device_phy"]["rx"]["power_gain"].as<unsigned int>(),
        },
        .tx_cfg = {
            .bw_hz = cfg["device_phy"]["tx"]["bw_hz"].as<long long>(),
            .fs_hz = cfg["device_phy"]["tx"]["fs_hz"].as<long long>(),
            .lo_hz = cfg["device_phy"]["tx"]["lo_hz"].as<long long>(),
            .block_size = cfg["device_phy"]["tx"]["block_size"].as<unsigned int>(),
            .power_gain = cfg["device_phy"]["tx"]["power_gain"].as<unsigned int>(),
        },
        0
    };
    memcpy(cfg_out.ip, cfg["address"].as<std::string>().c_str(),
        sizeof(cfg_out.ip));
    memcpy(cfg_out.rx_cfg.rfport,
        cfg["device_phy"]["rx"]["rfport"].as<std::string>().c_str(),
        sizeof(cfg_out.rx_cfg.rfport));
    memcpy(cfg_out.tx_cfg.rfport,
        cfg["device_phy"]["tx"]["rfport"].as<std::string>().c_str(),
        sizeof(cfg_out.tx_cfg.rfport));
    return cfg_out;
}

static config_x iter_read_cfg_x(const YAML::Node &cfg, const char *con) {
    config_x out = {
        .size = cfg["phy"][con]["ring_buffer_sample"]["size"].as<unsigned int>(),
        .thread_count = cfg["phy"][con]["ring_buffer_sample"]["thread"].as<unsigned int>(),
        .interval_of_with_dev = cfg["phy"][con]["interval_of_with_dev"].as<unsigned int>(),
    };
    return out;
}

static config_buffer convert_dev_buffer(const YAML::Node &cfg) {
    config_buffer cfg_out = {
        .rx = iter_read_cfg_x(cfg, "rx"),
        .tx = iter_read_cfg_x(cfg, "tx"),
    };
    return cfg_out;
}

static DIGITAL_SIGNAL_PROCESSING::OFDM_params load_ofdm_params_in_yaml(
    const YAML::Node &cfg_all) {
    mod_symbol pilot;
    YAML::Node cfg = cfg_all["ofdm_parameters"];
    pilot = {cfg["pilot"][0].as<type_complex>(),
            cfg["pilot"][1].as<type_complex>()};
    DIGITAL_SIGNAL_PROCESSING::OFDM_params param = {
        .count_subcarriers = cfg["count_subcarriers"].as<u_int16_t>(),
        .pilot = pilot,
        .step_RS = cfg["step_RS"].as<u_int16_t>(),
        .def_interval = cfg["def_interval"].as<u_int16_t>(),
        .cyclic_prefix = cfg["cyclic_prefix"].as<u_int16_t>(),
        .power = cfg["power"].as<float>(),
        .count_ofdm_in_slot = cfg["count_ofdm_in_slot"].as<u_int8_t>(),
    };
    return param;
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

#define DEBUG_DUMP_DATA


int ATTR_SERVICE::init_components(const std::string &key) {
    if(key == "sdr") {
        dev_com = {
            .init_dev = DeviceBuffer::initialization,
            .deinit_dev = DeviceBuffer::deinitialization,
            .send_dev = DeviceBuffer::send_msg,
            .recv_dev = DeviceBuffer::recv_msg
        };
    } else if(key == "model") {
        dev_com = {
            .init_dev = CHANNEL_MODEL::model_channel_init,
            .deinit_dev = CHANNEL_MODEL::model_channel_deinit,
            // .send_dev = CHANNEL_MODEL::write_channel,
            // .recv_dev = CHANNEL_MODEL::read_channel
        };
    } else {
        print_log(ERROR_OUT, "Error set env: %s\n", key.c_str());
        return STATUS_FAIL;
    }
    print_log(LOG, "[%s:%d] set env: %s\n", __func__, __LINE__, key.c_str());
    return STATUS_ACCESS;
}

int ATTR_SERVICE::init_system(context &ctx_dev) {
    ctx_dev.cfg_device = convert_config_device(ctx_dev.cfg);
    ctx_dev.cfg_buf = convert_dev_buffer(ctx_dev.cfg);
    
    ctx_dev.ofdm_param = load_ofdm_params_in_yaml(ctx_dev.cfg);
    if(dev_com.init_dev(ctx_dev)){
        print_log(CONSOLE, "[%s:%d] Error: initialization device, exit program\n",
            __func__, __LINE__);
        return STATUS_FAIL;
    }
    return STATUS_ACCESS;
}

int ATTR_SERVICE::deinit_system(context &cfg_dev) {
    return dev_com.deinit_dev(cfg_dev);
}

int ATTR_SERVICE::send_msg(context &ctx, const void *buffer, int size) {

    u_char buffer_tx[SIZE_BUFFER] = {};
    u_char *data = buffer_tx + sizeof(header_phy);
    header_phy *hv1 = (header_phy*)buffer_tx;
    DIGITAL_SIGNAL_PROCESSING::TypeModulation mod =
        DIGITAL_SIGNAL_PROCESSING::string_to_TypeModulation(
        ctx.cfg["type_modulation"].as<std::string>());
    int size_tx = sizeof(header_phy) + size + SIZE_PREAMBULA;
    hv1->id = ctx.id;
    hv1->mod = static_cast<u_char>(mod);
    hv1->size = size;
    memcpy(data, (char*)buffer, size);

    write_file_bin_data("../data/dump_data/data_tx.bin", 
        (void*)buffer_tx, size_tx);

    // for(int i = 0; i < 29; ++i) {
    //     print_log(CONSOLE, "%d - %c | ", data[i], (char)data[i]);
    // }print_log(CONSOLE, "\n");
    
    print_log(LOG, "size data: %d\n", size + sizeof(header_phy));
    msg_buffer msg;
    msg.size = size_tx;
    msg.data = buffer_tx;
    DEBUG_LINE

    return dev_com.send_dev(&msg);
#if 0
    VecSymbolMod samples = convert_msg_to_samples(ctx, buffer_tx, size_tx);
    int res = write_samples(samples, samples.size());
    print_log(LOG, "tx: %d, res: %d\n", samples.size(), res);
#endif
}

static int calc_bit_error1(DIGITAL_SIGNAL_PROCESSING::bit_sequence &tx,
    DIGITAL_SIGNAL_PROCESSING::bit_sequence &rx) {
    int error = 0;
    
    // print_bit2((u_char*)tx.buffer, tx.size);
    // print_bit2((u_char*)rx.buffer, rx.size);
    for(int i = 0; i < tx.size; ++i) {
        u_char otx = tx.buffer[i];
        u_char orx = rx.buffer[i];
        print_log(LOG_DATA, "%d %d\n", otx, orx);
        for(int i2 = 0; i2 < 8; ++i2) {
            u_char bt = otx & 0b1;
            u_char br = orx & 0b1;
            
            if(bt != br) {
                error++;
            }
            otx >>= 1;
            orx >>= 1;
        }
    }
    return error;
}

int ATTR_SERVICE::recv_msg(context &ctx, void *buffer, int size) {
    u_char buffer_rx[SIZE_BUFFER] = {};
    u_char *data = buffer_rx + sizeof(header_phy);
    header_phy *hv1 = (header_phy*)buffer_rx;
    int size_rx = sizeof(buffer_rx);
    msg_buffer msg;
    if(dev_com.recv_dev(&msg)) {
        DEBUG_LINE
        print_log(LOG, "%s: data: %p, size = %d\n", __func__, msg.data, msg.size);
        hv1 = (header_phy*)msg.data;
        data = msg.data + sizeof(header_phy);
        print_log(LOG, "phy: size: %d, header: id: %d, size: %d, msg: %s\n",
            msg.size, hv1->id, hv1->size, (char*)data);

        delete[] msg.data;
    }
#if 0
    static VecSymbolMod samples(ctx.cfg_device.rx_cfg.block_size);
    print_log(LOG_DATA, "\n[%s:%d] buffer rx: %d\n", __func__, __LINE__, samples.size());
    clock_t start = clock();
    int res = read_samples(samples, samples.size()); 
    // usleep(200);
    // sleep(10);
    double rt =  (double) (clock() - start) / CLOCKS_PER_SEC;
    // print_log(LOG, "rx, res: %d, clock: %f\n", res, rt);
    start = clock();

    res = convert_samples_to_msg(ctx, samples, buffer_rx, size);
    if(res) {
        print_log(LOG, "header: id: %d, size: %d, msg: %s\n", hv1->id, hv1->size, (char*)data);
        for(int i = 0; i < 29; ++i) {
            print_log(CONSOLE, "%d - %c | ", data[i], (char)data[i]);
        }print_log(CONSOLE, "\n");
        DIGITAL_SIGNAL_PROCESSING::bit_sequence data_rx;
        data_rx.buffer = buffer_rx;
        data_rx.size = res;
        DIGITAL_SIGNAL_PROCESSING::bit_sequence data_tx;
        data_tx.buffer = read_file_data("../data/dump_data/data_tx.bin", &data_tx.size);
        // data_tx.buffer = read_file_data("../data/data_test.txt", &data_tx.size);
        
        int count_error = calc_bit_error1(data_tx, data_rx);
        print_log(CONSOLE, "count error = %d/%d\n", count_error, data_tx.size * 8);

    }
    rt =  (double) (clock() - start) / CLOCKS_PER_SEC;
    // print_log(LOG, "rx, res: %d, clock: %f\n", res, rt);

    return res;
#endif

}



