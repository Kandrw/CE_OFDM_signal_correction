#include "target.hpp"

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

#include "header.hpp"
#include "phy/phy.hpp"
#ifdef M_FOUND_LIBIIO

#include "trx/device_api.hpp"
#endif

#include "model/modelling.hpp"
#include "loaders/load_data.hpp"
#include "ipc/managment_ipc.hpp"
#include "phy/ofdm_modulation.hpp"
#include "phy/signal_processing.hpp"
#include "config_parse.hpp"




int ofdm_model(int argc, char *argv[]){

    // if(argc < static_cast<int>(ARGV_CONSOLE::ARGV_MAX)){
    //     print_log(CONSOLE, "Error: Not enough arguments: <ip> <filename data>\n");
    //     return -1;
    // }
    const char filename[] = "../data/data_test.txt";// = argv[static_cast<int>(ARGV_CONSOLE::ARGV_FILE_DATA)];

#define TEST_DATA 0

#if TEST_DATA == 0
    char test_data[] = "Test m";
    
#elif TEST_DATA == 1   
    char test_data[90];
    for(int i = 0; i < sizeof(test_data); ++i){
        test_data[i] = (rand() % 200);
        // test_data[i] = (rand() % 20) + 70;
    }
    test_data[sizeof(test_data) - 1] = 0;
#elif TEST_DATA == 2
    char test_data[9] = {
        (char)0b00000001,
        (char)0b00100011,
        (char)0b01000101,
        (char)0b01100111,
        (char)0b10001001,
        (char)0b10101011,
        (char)0b11001101,
        (char)0b11101111
    };

    test_data[sizeof(test_data) - 1] = 0;
#endif
    write_file_bin_data(filename, test_data, sizeof(test_data));

    // Sleep(4 * 1000);
    bit_sequence data;

    data.buffer = read_file_data(filename, &data.size);
    if(data.size == 0){
        print_log(CONSOLE, "[ERROR] [main.cpp] main: empty data_bin, EXIT\n");
        return -1;
    }
    OFDM_params param_ofdm = {
        .count_subcarriers = 64,
        .pilot = {5, 5},
        .step_RS = 8,
        .def_interval = 16,
        .cyclic_prefix = 15,
        .power = 3000,
    };
    ParamsPhy param_phy = {
        .type_modulation = TypeModulation::QPSK,
        .param_ofdm = param_ofdm,
    };
    config_device cfg1 = {
        "None",
        {MHZ(2), MHZ(2.5), GHZ(1.9), "A_BALANCED"},
        {MHZ(2), MHZ(2.5), GHZ(1.9), "A"},
        1024 * 1024,
    };
    print_log(LOG_DATA, "size = %d, data: %s\n", data.size, data.buffer);
    OFDM_symbol samples = generate_frame_phy(data, param_phy);
    // exit(0);
    // addPowerOFDM(samples, param_ofdm.power);
    VecSymbolMod s = OFDM_convertion_one_thread(samples);
    // modelling_channel(s);
    samples = samples_join_OFDM(s, 
        param_ofdm.count_subcarriers + param_ofdm.cyclic_prefix, s.size());

    VecSlotsOFDM slots = create_slots(samples);

#if 1
    VecSymbolMod samples_rx(cfg1.block_size);
    int size_b;
    u_char *buffer = read_file_data(filename, &size_b);
    memcpy(samples_rx.data(), buffer, size_b);
    print_log(CONSOLE, "size samples_rx: %d\n", samples_rx.size());
    

#endif


    bit_sequence *read_data = decode_frame_phy(samples, param_phy);
    
    if(read_data) {
        for(int i = 0; i < data.size; ++i) {
            print_log(CONSOLE, "%u - %u\n", data.buffer[i], read_data->buffer[i]);
        }
        int count_error = calc_bit_error(data, *read_data);
        print_log(CONSOLE, "count error = %d/%d\n", count_error, read_data->size * 8);
        print_log(LOG_DATA, "read_data: size = %d, data: %s\n", read_data->size, read_data->buffer);
        print_log(CONSOLE, "input size: %d, output size: %d\n", data.size, read_data->size);
        print_log(CONSOLE, "End %s\n", __func__);
    }
    return 0;
}




int finding_pss(int argc, char *argv[]){

    // if(argc < static_cast<int>(ARGV_CONSOLE::ARGV_MAX)){
    //     print_log(CONSOLE, "Error: Not enough arguments: <ip> <filename data>\n");
    //     return -1;
    // }
    const char filename[] = "../data/data_test.txt";// = argv[static_cast<int>(ARGV_CONSOLE::ARGV_FILE_DATA)];

#define TEST_DATA 0

#if TEST_DATA == 0
    char test_data[] = "Test m";
    
#elif TEST_DATA == 1   
    char test_data[90];
    for(int i = 0; i < sizeof(test_data); ++i){
        test_data[i] = (rand() % 200);
        // test_data[i] = (rand() % 20) + 70;
    }
    test_data[sizeof(test_data) - 1] = 0;
#elif TEST_DATA == 2
    char test_data[9] = {
        (char)0b00000001,
        (char)0b00100011,
        (char)0b01000101,
        (char)0b01100111,
        (char)0b10001001,
        (char)0b10101011,
        (char)0b11001101,
        (char)0b11101111
    };

    test_data[sizeof(test_data) - 1] = 0;
#endif
    write_file_bin_data(filename, test_data, sizeof(test_data));

    // Sleep(4 * 1000);
    bit_sequence data;

    data.buffer = read_file_data(filename, &data.size);
    if(data.size == 0){
        print_log(CONSOLE, "[ERROR] [main.cpp] main: empty data_bin, EXIT\n");
        return -1;
    }
#if 0
    OFDM_params param_ofdm = {
        .count_subcarriers = 64,
        .pilot = {0.7, 0.7},
        .step_RS = 8,
        .def_interval = 16,
        .cyclic_prefix = 15,
        .power = 3000,
    };
    ParamsPhy param_phy = {
        .type_modulation = TypeModulation::QPSK,
        .param_ofdm = param_ofdm,
    };
#endif
    const char *file_conf = argv[static_cast<int>(ARGV_CONSOLE::ARGV_FILE_CONFIG)];

    config_program param = configure(file_conf);
    ParamsPhy param_phy = {
        .type_modulation = param.type_modulation,
        .param_ofdm = param.ofdm_params,
    };
    config_device cfg1 = {
        "None",
        {MHZ(2), MHZ(2.5), GHZ(1.9), "A_BALANCED"},
        {MHZ(2), MHZ(2.5), GHZ(1.9), "A"},
        1024 * 1024,
    };
#if 1
    print_log(LOG_DATA, "size = %d, data: %s\n", data.size, data.buffer);
    OFDM_symbol samples = generate_frame_phy(data, param_phy);
    // exit(0);
    // addPowerOFDM(samples, param_ofdm.power);
#if 1
    VecSlotsOFDM slots = create_slots(samples);
    write_OFDM_slots("../data/slots.bin", slots, slots.size());
#endif


    VecSymbolMod s = slots_OFDM_convertion_one_thread(slots);


    // VecSymbolMod s = OFDM_convertion_one_thread(samples);
    addPower(s, 4000);
    modelling_channel(s);
#if 0
    samples = samples_join_OFDM(s, 
        param_ofdm.count_subcarriers + param_ofdm.cyclic_prefix, s.size());

    VecSlotsOFDM slots = create_slots(samples);
    write_OFDM_slots("../data/slots.bin", slots, slots.size());
#endif
#endif
    
#if 1
    VecSymbolMod samples_rx(cfg1.block_size);
    int size_b;
    u_char *buffer = read_file_data("../data/rx_sample.bin", &size_b);
    memcpy(samples_rx.data(), buffer, size_b);
    print_log(CONSOLE, "size samples_rx: %d\n", samples_rx.size());
#else
    VecSymbolMod samples_rx = s;
#endif
    int result = -1;
#if 1
    OFDM_symbol ofdms;
    result = receiver_OFDM(samples_rx, param_phy.param_ofdm, ofdms);
    // VecSymbolMod rx_ = OFDM_demodulator(ofdms, param_ofdm, false);
#endif

#if 1
    bit_sequence *read_data = nullptr;
    if(result == 0)
        read_data = decode_frame_phy(ofdms, param_phy);
    
    if(read_data) {
        for(int i = 0; i < data.size; ++i) {
            print_log(CONSOLE, "%u - %u\n", data.buffer[i], read_data->buffer[i]);
        }
        int count_error = calc_bit_error(data, *read_data);
        print_log(CONSOLE, "count error = %d/%d\n", count_error, read_data->size * 8);
        print_log(LOG_DATA, "read_data: size = %d, data: %s\n", read_data->size, read_data->buffer);
        print_log(CONSOLE, "input size: %d, output size: %d\n", data.size, read_data->size);
        print_log(CONSOLE, "End %s\n", __func__);
    }
#endif
    return 0;
}

int ofdm_reception(int argc, char *argv[]){
    const char filename[] = "../data/module_ofdm_reception/data_test.txt";// = argv[static_cast<int>(ARGV_CONSOLE::ARGV_FILE_DATA)];

    const char *file_conf = argv[static_cast<int>(ARGV_CONSOLE::ARGV_FILE_CONFIG)];

    config_program param = configure(file_conf);
    ParamsPhy param_phy = {
        .type_modulation = param.type_modulation,
        .param_ofdm = param.ofdm_params,
    };
    config_device cfg1 = {
        "None",
        {MHZ(2), MHZ(2.5), GHZ(1.9), "A_BALANCED"},
        {MHZ(2), MHZ(2.5), GHZ(1.9), "A"},
        1024 * 1024,
    };
    VecSymbolMod samples_rx(cfg1.block_size);
    int size_b;
    u_char *buffer = read_file_data("../data/rx_sample.bin", &size_b);
    memcpy(samples_rx.data(), buffer, size_b);
    print_log(CONSOLE, "size samples_rx: %d\n", samples_rx.size());

    int result = -1;
#if 1
    OFDM_symbol ofdms;
    result = receiver_OFDM(samples_rx, param_phy.param_ofdm, ofdms);
#endif

#if 0
    bit_sequence *read_data = nullptr;
    if(result == 0)
        read_data = decode_frame_phy(ofdms, param_phy);
    
    if(read_data) {
        for(int i = 0; i < data.size; ++i) {
            print_log(CONSOLE, "%u - %u\n", data.buffer[i], read_data->buffer[i]);
        }
        int count_error = calc_bit_error(data, *read_data);
        print_log(CONSOLE, "count error = %d/%d\n", count_error, read_data->size * 8);
        print_log(LOG_DATA, "read_data: size = %d, data: %s\n", read_data->size, read_data->buffer);
        print_log(CONSOLE, "input size: %d, output size: %d\n", data.size, read_data->size);
        print_log(CONSOLE, "End %s\n", __func__);
    }
#endif
    return 0;
}









