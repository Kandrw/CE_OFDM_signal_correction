#include "components.hpp"

#include <cstring>

#include "../trx/device_api.hpp"

using namespace ATTR_SERVICE;

#define TMP_BUFFER_RX 80000 * sizeof(mod_symbol)
#define TMP_BUFFER_TX 20000 * sizeof(mod_symbol)

#include <output.hpp>
#include <complex_container.hpp>
#include <signal_processing.hpp>

int ATTR_SERVICE::init_config_com(int argc, char *argv[], config_com &cfg) {
    int status = STATUS_ACCESS;
    const char *file_conf = argv[static_cast<int>(ARGV_CONSOLE::ARGV_FILE_CONFIG)];
    cfg.param = configure(file_conf);
    cfg.cfg_dev = {
        .ip = cfg.param.address.c_str(),
        .rx_cfg = {MHZ(2), MHZ(2.5), GHZ(1.9), "A_BALANCED", TMP_BUFFER_RX},
        .tx_cfg = {MHZ(1.5), MHZ(2.5), GHZ(1.9), "A", TMP_BUFFER_TX},
        .block_size = 0,
    };
    return status;
}

int ATTR_SERVICE::init_log_system(const config_com &cfg) {
    if(init_log(cfg.param.file_log.c_str())) {
        return STATUS_FAIL;
    }
    return STATUS_ACCESS;
}

int ATTR_SERVICE::deinit_log_system(const config_com &cfg) {
    deinit_log();
    return STATUS_ACCESS;
}

int init_device(config_device &cfg_dev) {
    if(DeviceTRX::initialization(cfg_dev)){
        print_log(CONSOLE, "[%s:%d] Error: initialization device, exit program\n",
            __func__, __LINE__);
        return STATUS_FAIL;
    }
    return STATUS_ACCESS;
}

int deinit_device(config_device &cfg_dev) {
    DeviceTRX::deinitialization();
    return STATUS_ACCESS;
}

int ATTR_SERVICE::init_bss(config_com &cfg) {
    int status = STATUS_ACCESS;

    EXIT_IF_FAIL(init_device(cfg.cfg_dev));

    return status;
}

int ATTR_SERVICE::deinit_bss(config_com &cfg) {
    int status = STATUS_ACCESS;

    EXIT_IF_FAIL(deinit_device(cfg.cfg_dev));

    return status;
}








