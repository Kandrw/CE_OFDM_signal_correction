#include "components.hpp"

#define RUN_BSS true


static bool run_bss = !RUN_BSS;


int bss_program(int argc, char *argv[]) {

    int status = STATUS_ACCESS;
    int result;
    // const char *file_conf = argv[static_cast<int>(ARGV_CONSOLE::ARGV_FILE_CONFIG)];
    ATTR_SERVICE::config_com cfg_bss;
    EXIT_IF_FAIL(ATTR_SERVICE::init_config_com(argc, argv, cfg_bss));

    print_configure(cfg_bss.param);

    EXIT_IF_FAIL(ATTR_SERVICE::init_log_system(cfg_bss));
    EXIT_IF_FAIL(ATTR_SERVICE::init_bss(cfg_bss));


    
    while(run_bss) {
        // result = DeviceTRX::recv_samples(samples_rx, cfg1.block_size);

    }
    print_log(CONSOLE, "End bss\n");


    EXIT_IF_FAIL(ATTR_SERVICE::deinit_bss(cfg_bss));
    ATTR_SERVICE::deinit_log_system(cfg_bss);
    return status;
}

