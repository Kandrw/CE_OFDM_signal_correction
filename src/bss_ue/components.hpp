#pragma once
#include "../configure/config_parse.hpp"
#include "../trx/types_trx.hpp"



#define STATUS_ACCESS 0
#define STATUS_FAIL 1
#define EXIT_IF_FAIL(func) {int status = func; if(status && STATUS_FAIL) return status;}


namespace ATTR_SERVICE {




    struct header_phy {
        u_int16_t id;
        u_char mod;
        
        u_int16_t size;
    };


    int init_components(const std::string &key);
    // int init_config_com(int argc, char *argv[], config_com &cfg);
    int init_log_system(const char *file_log);
    int deinit_log_system();

    int send_msg(context &ctx, const void *buffer, int size);
    int recv_msg(context &ctx, void *buffer, int size);

    int init_system(context &ctx_dev);
    int deinit_system(context &ctx_dev);
    int write_samples(const VecSymbolMod &samples, size_t size);
    int read_samples(VecSymbolMod &samples, size_t size);

};


