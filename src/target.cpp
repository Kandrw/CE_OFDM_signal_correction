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
#include "phy/modulation.hpp"



int ofdm_model(int argc, char *argv[]){

    // if(argc < static_cast<int>(ARGV_CONSOLE::ARGV_MAX)){
    //     print_log(CONSOLE, "Error: Not enough arguments: <ip> <filename data>\n");
    //     return -1;
    // }
    const char filename[] = "../data/data_test.txt";// = argv[static_cast<int>(ARGV_CONSOLE::ARGV_FILE_DATA)];
    const char *file_conf = argv[static_cast<int>(ARGV_CONSOLE::ARGV_FILE_CONFIG)];
    config_program param = configure(file_conf);
    // init_log(param.file_log.c_str());
    init_log("../log_ofdm_model.log");
    
#define TEST_DATA 1

#if TEST_DATA == 0
    char test_data[] = "Test sfadfg asf afd";
    
#elif TEST_DATA == 1   
    char test_data[200];
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
    VecSymbolMod samples_tx;
    
    OFDM_symbol samples = generate_frame_phy(data, param_phy, samples_tx);
    print_log(LOG_DATA, "samples_tx = %d\n", samples_tx.size());
    // exit(0);
    // addPowerOFDM(samples, param_ofdm.power);
    VecSymbolMod s = OFDM_convertion_one_thread(samples);
    // modelling_channel(s);
    samples = samples_join_OFDM(s, 
        param_ofdm.count_subcarriers + param_ofdm.cyclic_prefix, s.size());

    VecSlotsOFDM slots = create_slots(samples, param.ofdm_params);

#if 1
    VecSymbolMod samples_rx(cfg1.block_size);
    int size_b;
    u_char *buffer = read_file_data(filename, &size_b);
    memcpy(samples_rx.data(), buffer, size_b);
    print_log(CONSOLE, "size samples_rx: %d\n", samples_rx.size());
#endif

    // bit_sequence *read_data = decode_frame_phy(samples, param_phy, true);
    print_log(CONSOLE, "[%s:%d]\n", __func__, __LINE__);
    VecSymbolMod rx_sample = OFDM_demodulator(samples, param_phy.param_ofdm, true);
    print_log(CONSOLE, "[%s:%d] rx_sample - %d\n", __func__, __LINE__, rx_sample.size());




#if 1

    for(int i = 0; i < samples_tx.size(); ++i) {
        print_log(LOG_DATA, "%f %f\t\t-\t\t%f %f\n",
            samples_tx[i].real(), samples_tx[i].imag(),
            rx_sample[i].real(), rx_sample[i].imag()
            );
    } print_log(LOG_DATA, "\n");
#endif
    bit_sequence *read_data = demodulation_mapper(rx_sample, param_phy.type_modulation);
    
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
    deinit_log();
    return 0;
}




int finding_pss(int argc, char *argv[]){

    // if(argc < static_cast<int>(ARGV_CONSOLE::ARGV_MAX)){
    //     print_log(CONSOLE, "Error: Not enough arguments: <ip> <filename data>\n");
    //     return -1;
    // }
    
    const char *file_conf = argv[static_cast<int>(ARGV_CONSOLE::ARGV_FILE_CONFIG)];

    config_program param = configure(file_conf);
    init_log(param.file_log.c_str());

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

    VecSymbolMod samples_tx;
    OFDM_symbol samples = generate_frame_phy(data, param_phy, samples_tx);
    
    // exit(0);
    // addPowerOFDM(samples, param_ofdm.power);
#if 1
    VecSlotsOFDM slots = create_slots(samples, param.ofdm_params);
    write_OFDM_slots("../data/slots.bin", slots, slots.size());
#endif


    VecSymbolMod s = slots_OFDM_convertion_one_thread(slots);


    // VecSymbolMod s = OFDM_convertion_one_thread(samples);
    addPower(s, 4000);
    modelling_channel(s);
#if 0
    samples = samples_join_OFDM(s, 
        param_ofdm.count_subcarriers + param_ofdm.cyclic_prefix, s.size());

    VecSlotsOFDM slots = create_slots(samples, param.ofdm_params);
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
    deinit_log();
    return 0;
}

int ofdm_reception(int argc, char *argv[]){
    // const char filename[] = "../data/module_ofdm_reception/data_test.txt";// = argv[static_cast<int>(ARGV_CONSOLE::ARGV_FILE_DATA)];
#if 1
    const char *filename = argv[static_cast<int>(ARGV_CONSOLE::ARGV_FILE_DATA)];
#else
    const char *filename = "../data/data_test.txt";
#endif

    const char *file_conf = argv[static_cast<int>(ARGV_CONSOLE::ARGV_FILE_CONFIG)];

    config_program param = configure(file_conf);
    ParamsPhy param_phy = {
        .type_modulation = param.type_modulation,
        .param_ofdm = param.ofdm_params,
    };
    // init_log(param.file_log.c_str());
    init_log("../log_processing.log");
    
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
    bit_sequence data_tx;
    data_tx.buffer = read_file_data(filename, &data_tx.size);
    if(data_tx.size == 0){
        print_log(CONSOLE, "[ERROR] [main.cpp] main: empty data_bin, EXIT\n");
        return -1;
    }
#if 1
    OFDM_symbol ofdms_rx;
    result = receiver_OFDM(samples_rx, param_phy.param_ofdm, ofdms_rx);
#endif

#if 1
    // bit_sequence *read_data = nullptr;
    // if(result == 0)
    //     read_data = decode_frame_phy(ofdms, param_phy);
    
    print_log(CONSOLE, "[%s:%d]\n", __func__, __LINE__);
    VecSymbolMod rx_sample = OFDM_demodulator(ofdms_rx, param_phy.param_ofdm, false);
    print_log(CONSOLE, "[%s:%d] rx_sample - %d\n", __func__, __LINE__, rx_sample.size());


    write_file_bin_data("../data/samples_modulation_rx.bin", 
        (void*)&rx_sample[0], rx_sample.size() * sizeof(mod_symbol));


    bit_sequence *read_data = demodulation_mapper(rx_sample, param_phy.type_modulation);

    if(read_data) {
        for(int i = 0; i < data_tx.size; ++i) {
            print_log(LOG_DATA, "%u - %u\n", data_tx.buffer[i], read_data->buffer[i]);
        }
        int count_error = calc_bit_error(data_tx, *read_data);
        print_log(CONSOLE, "count error = %d/%d\n", count_error, read_data->size * 8);
        print_log(LOG_DATA, "read_data: size = %d, data: %s\n", read_data->size, read_data->buffer);
        print_log(CONSOLE, "input size: %d, output size: %d\n", data_tx.size, read_data->size);
        print_log(CONSOLE, "End %s\n", __func__);
    }
#endif
    deinit_log();
    return 0;
}



int ofdm_model_add_noise(int argc, char *argv[]){

    // if(argc < static_cast<int>(ARGV_CONSOLE::ARGV_MAX)){
    //     print_log(CONSOLE, "Error: Not enough arguments: <ip> <filename data>\n");
    //     return -1;
    // }
    const char filename[] = "../data/ofdm_model_add_noise/data_test.txt";
    const char *file_conf = argv[static_cast<int>(ARGV_CONSOLE::ARGV_FILE_CONFIG)];
    config_program param = configure(file_conf);
    init_log(param.file_log.c_str());
#define TEST_DATA 1

#if TEST_DATA == 0
    char test_data[] = "Test m";
    
#elif TEST_DATA == 1   
    // char test_data[100];
    char test_data[100];
    
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
        .count_subcarriers = 128,
        .pilot = {0.7, 0.7},
        .step_RS = 8,
        .def_interval = 30,
        .cyclic_prefix = 40,
        .power = 3000,
        .count_ofdm_in_slot = 7,
    };
    config_program cfg = {
        .file_log = "",
        .address = "None",
        .type_modulation = TypeModulation::QPSK,
        // .type_modulation = TypeModulation::QAM16,
        .ofdm_params = param_ofdm
    };
    ParamsPhy param_phy = {
        .type_modulation = cfg.type_modulation,
        .param_ofdm = cfg.ofdm_params,
    };
    config_device cfg_dev = {
        "None",
        {MHZ(2), MHZ(2.5), GHZ(1.9), "A_BALANCED"},
        {MHZ(2), MHZ(2.5), GHZ(1.9), "A"},
        1024 * 1024,
    };
    write_config_file("../data/ofdm_model_add_noise/config.txt", cfg);
    print_log(LOG_DATA, "size = %d, data: %s\n", data.size, data.buffer);

    // OFDM_symbol ofdms = generate_frame_phy(data, param_phy, samples_tx);

    VecSymbolMod samples_tx = modulation_mapper(data, param_phy.type_modulation);
    
    write_file_bin_data("../data/samples_modulation_tx.bin", 
        (void*)&samples_tx[0], samples_tx.size() * sizeof(mod_symbol));

    OFDM_symbol ofdms = OFDM_modulator(samples_tx, param_phy.param_ofdm);
    print_log(LOG_DATA, "ofdms:\n");
    for(int i = 0; i < ofdms.size(); ++i) {
        print_log(LOG_DATA, "\t%d ofdm:\n", i + 1);
        print_VecSymbolMod(ofdms[i]);
    }
      
    // exit(0);
    // addPowerOFDM(samples, param_ofdm.power);
    // VecSymbolMod s = OFDM_convertion_one_thread(samples);
    // // modelling_channel(s);
    // samples = samples_join_OFDM(s, 
    //     param_ofdm.count_subcarriers + param_ofdm.cyclic_prefix, s.size());

    VecSlotsOFDM slots = create_slots(ofdms, param.ofdm_params);
    print_log(CONSOLE, "count slots: %d, count ofdm symbol: %d\n", slots.size(), ofdms.size());
    // addPower(slots[0].PSS, 2);
    
    addPowerSlots(slots, param_phy.param_ofdm.power);

    
    write_OFDM_slots("../data/slots_tx.bin", slots, slots.size());
    
    write_OFDM_slots("../data/ofdm_model_add_noise/slots_tx.bin", slots, slots.size());
    VecSymbolMod samples = slots_OFDM_convertion_one_thread(slots);
    write_file_bin_data("../data/samples_tx.bin", 
            (void*)&samples[0], samples.size() * sizeof(mod_symbol));
      
    // write_OFDM_slots("../data/ofdm_model_add_noise/signal_tx.bin", slots, slots.size());
    write_file_bin_data("../data/ofdm_model_add_noise/signal_tx.bin", 
            (void*)&samples[0], samples.size() * sizeof(mod_symbol));

    modelling_channel(samples);

    write_file_bin_data("../data/ofdm_model_add_noise/signal_rx.bin", 
            (void*)&samples[0], samples.size() * sizeof(mod_symbol));
    write_file_bin_data("../data/rx_sample.bin", 
            (void*)&samples[0], samples.size() * sizeof(mod_symbol));
    int result = -1;
    OFDM_symbol ofdms_rx;
    print_log(CONSOLE, "\n\ncall receiver_OFDM\n");
    result = receiver_OFDM(samples, param_phy.param_ofdm, ofdms_rx);
    // return 1;
    print_log(CONSOLE, "send: %d, recv: %d\n", slots[0].ofdms.size(), ofdms_rx.size());
#if 0
    VecSymbolMod samples_rx(cfg1.block_size);
    int size_b;
    u_char *buffer = read_file_data(filename, &size_b);
    memcpy(samples_rx.data(), buffer, size_b);
    print_log(CONSOLE, "size samples_rx: %d\n", samples_rx.size());
#endif
    print_log(CONSOLE, "[%s:%d]\n", __func__, __LINE__);
    VecSymbolMod rx_sample = OFDM_demodulator(ofdms_rx, param_phy.param_ofdm, false);
    print_log(CONSOLE, "[%s:%d] rx_sample - %d\n", __func__, __LINE__, rx_sample.size());


    write_file_bin_data("../data/samples_modulation_rx.bin", 
        (void*)&rx_sample[0], rx_sample.size() * 2 * 4);


#if 1
    print_log(LOG, "size tx: %d, size rx: %d\n", samples_tx.size(), rx_sample.size());
    for(int i = 0; i < samples_tx.size(); ++i) {
        print_log(LOG_DATA, "%f %f\t\t-\t\t%f %f\n",
            samples_tx[i].real(), samples_tx[i].imag(),
            rx_sample[i].real(), rx_sample[i].imag()
            );
    } print_log(LOG_DATA, "\n");
#endif
    // bit_sequence *read_data = decode_frame_phy(ofdms_rx, param_phy);
    bit_sequence *read_data = demodulation_mapper(rx_sample, param_phy.type_modulation);


#if 1
    if(read_data) {
        for(int i = 0; i < data.size; ++i) {
            print_log(LOG_DATA, "%u - %u\n", data.buffer[i], read_data->buffer[i]);
        }
        int count_error = calc_bit_error(data, *read_data);
        print_log(CONSOLE, "count error = %d/%d\n", count_error, read_data->size * 8);
        print_log(LOG_DATA, "read_data: size = %d, data: %s\n", read_data->size, read_data->buffer);
        print_log(CONSOLE, "input size: %d, output size: %d\n", data.size, read_data->size);
        print_log(CONSOLE, "End %s\n", __func__);
    }
#endif
    deinit_log();
    return 0;
}






