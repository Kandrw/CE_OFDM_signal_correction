

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

#include <output.hpp>
#include <complex_container.hpp>
#include <signal_processing.hpp>


#include "target.hpp"
#include "model/modelling.hpp"
#include "loaders/load_data.hpp"
#include "ipc/managment_ipc.hpp"


#include "configure/config_parse.hpp"
#include "test_components/test_components.hpp"
#include "trx/debug_trx.hpp"

// #include "bss_ue/service.hpp"



int target(int argc, char *argv[]) {
    
    const char *file_conf = argv[static_cast<int>(ARGV_CONSOLE::ARGV_FILE_CONFIG)];

    config_program param = configure(file_conf);
    init_log(param.file_log.c_str());

    if(argc == static_cast<int>(ARGV_CONSOLE::ARGV_MAX) + 1) {
        return modelling_signal(argv[static_cast<int>(ARGV_CONSOLE::ARGV_MAX)][0]);
    }
    print_log(CONSOLE, "No option for modelling_signal\n");
    deinit_log();
    return 0;
}
int ofdm_model(int argc, char *argv[]);

int test_ipc(int argc, char *argv[]);


int main(int argc, char *argv[]){
    
    const char *file_conf = argv[static_cast<int>(ARGV_CONSOLE::ARGV_FILE_CONFIG)];

    // config_program param = configure(file_conf);
    // print_configure(param);
    srand(time(NULL));
    // init_log(param.file_log.c_str());

    std::map<std::string, std::function<int(int, char**)>> target_exec = {
        {"model_practice", target},
        {"ofdm_model", ofdm_model},
        {"fpss", finding_pss},
        {"ofdm_re", ofdm_reception},
        {"ofdm_model_noise", ofdm_model_add_noise},
#if M_FOUND_LIBIIO
        {"rx", test_RX},
        {"tx", test_TX},
        {"realtime", realtime_RX},
#endif
        {"test_ipc", test_ipc},
#if 0
        {"test_components", test_components}
#endif
#ifdef SERVICE
        {"bss", bss_program},
        {"ue", ue_program},
#endif
    };


    auto find_taget_exec = target_exec.find(
        argv[static_cast<int>(ARGV_CONSOLE::ARGV_TARGET_PROGRAM)]);
    if(find_taget_exec != target_exec.end()) {
        printf("call target: %s\n", find_taget_exec->first.c_str());
        find_taget_exec->second(argc, argv);
    } else {
        printf("No target: %s\n", 
            argv[static_cast<int>(ARGV_CONSOLE::ARGV_TARGET_PROGRAM)] );
        printf("targets:\n");
        for(auto t : target_exec) {
            printf("\t%s\n", t.first.c_str());
        }
    }
    

    printf("End program\n");
    // deinit_log();
    return 0;
}






#if 0
int main(int argc, char *argv[]){

    VecSymbolMod sdf;
    print_VecSymbolMod(stdout, sdf);
    std::cout<<__cplusplus<<"\n";

    printf("End\n");
}
#endif






