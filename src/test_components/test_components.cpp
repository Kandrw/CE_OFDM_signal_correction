#include "test_components.hpp"

#include "../configure/config_parse.hpp"


void test_cfg(const char *filename) {
    config_program cfg = configure(filename);
    print_configure(cfg);
}


static int components(int argc, char *argv[]) {
    char target = argv[static_cast<int>(ARGV_CONSOLE::ARGV_MAX)][0];
    switch (target)
    {
    case '0':
        test_cfg(argv[static_cast<int>(ARGV_CONSOLE::ARGV_FILE_CONFIG)]);
        break;
    default:
        print_log(CONSOLE, "No target for program\n");
        break;
    }
    return 0;
}

int test_components(int argc, char *argv[]) {

    if(argc == static_cast<int>(ARGV_CONSOLE::ARGV_MAX) + 1) {
        return components(argc, argv);
    }
    print_log(CONSOLE, "No option for test_components\n");
    return 0;
}









































































































