#include "components.hpp"

#include <iostream>
#include <string>

#include <yaml-cpp/yaml.h>


#define RUN_UE true

static bool run_eu = !RUN_UE;

enum class ARGV_CONSOLE {
    ARGV_FILE_CONFIG = 2
};

int ue_program(int argc, char *argv[]) {
    int status = STATUS_ACCESS;
    int result;
    const char *file_conf = argv[static_cast<int>(ARGV_CONSOLE::ARGV_FILE_CONFIG)];
    
    ATTR_SERVICE::context ctx_ue = {500};
    ctx_ue.cfg = YAML::LoadFile(file_conf);

    printf("\t%s\n", file_conf);
    std::cout<<ctx_ue.cfg<<"\n";
    EXIT_IF_FAIL(ATTR_SERVICE::init_log_system(ctx_ue.cfg["log_file"].as<std::string>().c_str()));
    EXIT_IF_FAIL(ATTR_SERVICE::init_components(ctx_ue.cfg["env"].as<std::string>()));
    EXIT_IF_FAIL(ATTR_SERVICE::init_system(ctx_ue));
    
    run_eu = RUN_UE;
    std::string cmd = "";

    // char buffer[] = "TestTestTestTestTestTestTestTestTestTestTestTestTestTestTestTestTestTestasdasdasasasas";
    char buffer[] = "Test";
    while(run_eu) {
        std::cout<<"command: ";
        std::cin >> cmd;
        std::cout<<"command len - "<<cmd.size()<<"\n";
        if(cmd == "exit") {
            break;
        }
        if(cmd == "while") {
            for(int i = 0; i < 10000; ++i) {
                ATTR_SERVICE::send_msg(ctx_ue, buffer, sizeof(buffer));
            }
        }
        // for(int i = 0; i < 10; ++i)
        ATTR_SERVICE::send_msg(ctx_ue, buffer, sizeof(buffer));
    }
    print_log(CONSOLE, "End ue id%d\n", ctx_ue.id);

    ATTR_SERVICE::deinit_system(ctx_ue);
    ATTR_SERVICE::deinit_log_system();
    return status;
}





