

#include <iostream>
#include <cstdlib>
#include <ctime>
#include <cstring>

#include <fstream>
#include <vector>

#include <output.hpp>
#include <memory>
#include <map>
#include <functional>

// #include <output.hpp>
// #include <complex_container.hpp>
// #include <signal_processing.hpp>
// #include <configure.hpp>

// #include "configure/config_parse.hpp"


#include "bss_ue/service.hpp"
#include "bss_ue/channel_model.hpp"

#define TARGET_POS 1

#define print_msg(...) printf(__VA_ARGS__)

int main(int argc, char *argv[]){

#if 0
    printf("argc = %d\n", argc);
    for(int i = 0; i < argc; ++i) {
        printf("%s ", argv[i]);
    } printf("\n");
    return 0;
#endif

    std::map<std::string, std::function<int(int, char**)>> target_exec = {
        {"bss", bss_program},
        {"ue", ue_program},
        {"channel_model", channel_model},
        {"monitor", monitor_signal},
        
    };
    auto find_target_exec = target_exec.find(argv[TARGET_POS]);
    if(find_target_exec != target_exec.end()) {
        find_target_exec->second(argc, argv);
    } else {
        print_msg("no targets: %s\n", argv[TARGET_POS]);
        print_msg("targets:\n");
        for(auto const &t : target_exec) {
            print_msg("\t%s\n", t.first.c_str());
        }
    }
    return 0;
}






