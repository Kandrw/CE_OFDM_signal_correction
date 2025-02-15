#include "components.hpp"

#include <unistd.h>
#include <signal.h>
#include <thread>

#include <yaml-cpp/yaml.h>

#define RUN_BSS true


static bool run_bss = !RUN_BSS;

enum class ARGV_CONSOLE {
    ARGV_FILE_CONFIG = 2
};

void exit_and_clear_resource() {

}


void broadcast_msg(context &ctx) {
    char buffer[] = "HELLO, msg from bss";
    while(true) {
        ATTR_SERVICE::send_msg(ctx, buffer, sizeof(buffer));
        usleep(100);
    }
}

int bss_program(int argc, char *argv[]) {

    int status = STATUS_ACCESS;
    int result;
    const char *file_conf = argv[static_cast<int>(ARGV_CONSOLE::ARGV_FILE_CONFIG)];
    // sigset_t signalSet;
    // int sig;
    context ctx_bss = {200};
    ctx_bss.cfg = YAML::LoadFile(file_conf);

    printf("\t%s\n", file_conf);
    std::cout<<ctx_bss.cfg<<"\n";
    EXIT_IF_FAIL(ATTR_SERVICE::init_log_system(ctx_bss.cfg["log_file"].as<std::string>().c_str()));
    EXIT_IF_FAIL(ATTR_SERVICE::init_components(ctx_bss.cfg["env"].as<std::string>()));
    EXIT_IF_FAIL(ATTR_SERVICE::init_system(ctx_bss));
    // sigemptyset(&signalSet);
    // sigaddset(&signalSet, SIGTERM);
    // sigaddset(&signalSet, SIGINT);
    // if (sigprocmask(SIG_BLOCK, &signalSet, NULL) < 0) {
    //     perror("sigprocmask");
    //     exit(EXIT_FAILURE);
    // }
    run_bss = RUN_BSS;
    char buffer[256];
    // std::thread thr = std::thread(broadcast_msg, std::ref(ctx_bss));
    while(run_bss) {
        // sigwait(&signalSet, &sig);
        ATTR_SERVICE::recv_msg(ctx_bss, buffer, sizeof(buffer));
        // sleep(1);
    }
    print_log(CONSOLE, "End bss\n");

    ATTR_SERVICE::deinit_system(ctx_bss);
    ATTR_SERVICE::deinit_log_system();
    return status;
}

