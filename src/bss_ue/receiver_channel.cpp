#include "components.hpp"

#include <unistd.h>
#include <signal.h>
#include <thread>

#include <yaml-cpp/yaml.h>

#define RUN true

enum class ARGV_CONSOLE {
    ARGV_FILE_CONFIG = 2
};

static bool run = !RUN;

int recv_channel_ofdm(int argc, char *argv[]) {
    
    int status = STATUS_ACCESS;
    int result;
    const char *file_conf = argv[static_cast<int>(ARGV_CONSOLE::ARGV_FILE_CONFIG)];
    context ctx = {555};
    ctx.cfg = YAML::LoadFile(file_conf);
    
    printf("\t%s\n", file_conf);
    std::cout<<ctx.cfg<<"\n";
    EXIT_IF_FAIL(ATTR_SERVICE::init_log_system(ctx.cfg["log_file"].as<std::string>().c_str()));
    EXIT_IF_FAIL(ATTR_SERVICE::init_components(ctx.cfg["env"].as<std::string>()));
    EXIT_IF_FAIL(ATTR_SERVICE::init_system(ctx));
    run = RUN;
    char buffer[25600];
    while(run) {
        status = ATTR_SERVICE::recv_msg(ctx, buffer, sizeof(buffer));
        if(status) {
            print_log(CONSOLE, "Get msg\n");
        }
    }
    print_log(CONSOLE, "End recv_channel_ofdm\n");

    ATTR_SERVICE::deinit_system(ctx);
    ATTR_SERVICE::deinit_log_system();
    return status;
}
