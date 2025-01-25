#include "components.hpp"

#include <unistd.h>
#include <signal.h>

#include <yaml-cpp/yaml.h>

#define RUN_BSS true


static bool run_bss = !RUN_BSS;

enum class ARGV_CONSOLE {
    ARGV_FILE_CONFIG = 2
};

void exit_and_clear_resource() {

}

int bss_program(int argc, char *argv[]) {

    int status = STATUS_ACCESS;
    int result;
    const char *file_conf = argv[static_cast<int>(ARGV_CONSOLE::ARGV_FILE_CONFIG)];
    // sigset_t signalSet;
    // int sig;
    ATTR_SERVICE::context ctx_bss = {200};
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
    
    while(run_bss) {
        // sigwait(&signalSet, &sig);
        sleep(1);
    }
    print_log(CONSOLE, "End bss\n");

    ATTR_SERVICE::deinit_system(ctx_bss);
    ATTR_SERVICE::deinit_log_system();
    return status;
}

