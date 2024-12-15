#pragma once
#include "../configure/config_parse.hpp"
#include "../trx/types_trx.hpp"


#define STATUS_ACCESS 0
#define STATUS_FAIL -1
#define EXIT_IF_FAIL(status) {if(status && STATUS_FAIL) return status;}


namespace ATTR_SERVICE {


    struct config_com {
        config_program param;
        config_device cfg_dev;
    };

    int init_config_com(int argc, char *argv[], config_com &cfg);
    int init_log_system(const config_com &cfg);
    int deinit_log_system(const config_com &cfg);
    int init_bss(config_com &cfg);
    int deinit_bss(config_com &cfg);
};


