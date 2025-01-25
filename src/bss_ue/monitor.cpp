
#include "components.hpp"

#include <unistd.h>
#include <signal.h>
#include <thread>

#include <yaml-cpp/yaml.h>

#include "../ipc/managment_ipc.hpp"

enum class ARGV_CONSOLE {
    ARGV_FILE_CONFIG = 2
};

static bool run = false;
static std::thread thr;
static int con_command;
static void exit_program() {
    std::string asd = "exit";
    send_ipc(10, 4, 12, (u_char*)asd.c_str());
    deinit_ipc();
}

static void signalHandler(int signum) {
    exit_program();
    exit(signum);
}

void reading_command() {
    std::string command;
    while(run) {
        print_log(CONSOLE, "input: ");
        std::cin >> command;
        if(command == "exit") {
            con_command = 10;
            break;
        }
    }
}

int monitor_signal(int argc, char *argv[]) {
    int result;
    const char *file_conf = argv[static_cast<int>(ARGV_CONSOLE::ARGV_FILE_CONFIG)];
    ATTR_SERVICE::context ctx_m = {0};
    ctx_m.cfg = YAML::LoadFile(file_conf);
    EXIT_IF_FAIL(ATTR_SERVICE::init_log_system("../log_monitor.log"));
        
    EXIT_IF_FAIL(ATTR_SERVICE::init_components("sdr"));
    EXIT_IF_FAIL(ATTR_SERVICE::init_system(ctx_m));
    signal(SIGINT, signalHandler);
    signal(SIGTERM, signalHandler);
    if(init_ipc()) {
        print_log(CONSOLE, "Exit, error init ipc monitor\n");
        return -1;
    }
    DEBUG_LINE
    VecSymbolMod samples_rx(ctx_m.cfg["device_phy"]["rx"]["block_size"].as<unsigned int>());
    DEBUG_LINE
    run = true;
    thr = std::thread(reading_command);
    con_command = 12;
    while(run) {
        std::string str = "Test complex";
        result = ATTR_SERVICE::read_samples(samples_rx, samples_rx.size());
        data_array d3 = data_array(str.size(), (const u_char*)str.c_str(), 8 * samples_rx.size(), 
            static_cast<u_char>(TYPE_ARRAY::TYPE_COMPLEX_FLOAT), (u_char*)&samples_rx[0]);
        // print_log(LOG, "rx data: %d\n", samples_rx.size());
        write_file_bin_data("../data/rx_sample.bin", 
            (void*)&samples_rx[0], samples_rx.size() * 2 * 4);
        std::vector<data_array*> arr;
        arr.push_back(&d3);
        full_data_arrays("monitor", arr);

        std::string asd = "monitor";
        send_ipc(con_command, 4, 12, (u_char*)asd.c_str());
        if(con_command == 10) {
            run = false;
            break;
        }
    }
    thr.join();
    ATTR_SERVICE::deinit_system(ctx_m);
    ATTR_SERVICE::deinit_log_system();
}

























