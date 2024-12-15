#include "debug_trx.hpp"

#include <output.hpp>
#include <complex_container.hpp>
#include <signal_processing.hpp>

#include "../header.hpp"
#include "../ipc/managment_ipc.hpp"
#include "device_api.hpp"

#include "../loaders/load_data.hpp"
#include "../configure/config_parse.hpp"

using namespace DIGITAL_SIGNAL_PROCESSING;
using namespace DEVICE_PHY;

static OFDM_symbol generate_frame_phy(bit_sequence &bits, ParamsPhy &param, VecSymbolMod &samples_tx){
    print_log(CONSOLE, "[%s:%d]\n", __func__, __LINE__);
    VecSymbolMod samples = modulation_mapper(bits, param.type_modulation);
    samples_tx = samples;
    // VecSymbolMod ofdm_samples = 
    OFDM_symbol ofdms = OFDM_modulator(samples, param.param_ofdm);
    print_log(LOG, "Create %d ofdm symbol\n", ofdms.size());
     
    
    return ofdms;
}

static bit_sequence *decode_frame_phy(OFDM_symbol &samples, ParamsPhy &param, bool fprefix = true){
    print_log(CONSOLE, "[%s:%d]\n", __func__, __LINE__);
    VecSymbolMod rx_sample = OFDM_demodulator(samples, param.param_ofdm, fprefix);
    print_log(CONSOLE, "[%s:%d] rx_sample - %d\n", __func__, __LINE__, rx_sample.size());
    return demodulation_mapper(rx_sample, param.type_modulation);
    // return nullptr;
}

void print_cfg_dev(stream_cfg &txcfg){
    print_log(LOG_DEVICE, "bw_hz = %lld, fs_hz = %lld, lo_hz = %lld, rfport = %s\n", 
        txcfg.bw_hz, txcfg.fs_hz, txcfg.lo_hz, txcfg.rfport
    );
}

int test_RX(int argc, char *argv[]){

#if 1
    const char *ip_device = argv[static_cast<int>(ARGV_CONSOLE::ARGV_IP_DEVICE)];
    const char *filename = argv[static_cast<int>(ARGV_CONSOLE::ARGV_FILE_DATA)];
#else
    const char *ip_device = "ip:192.168.3.1";
    const char *filename = "../data/data_test.txt";

#endif

#if 1
    char test_data[] = "Test message";
#else    
    char test_data[310];
    for(int i = 0; i < (int)sizeof(test_data); ++i){
        // test_data[i] = (rand() % 200);
        test_data[i] = (rand() % 20) + 70;
    }
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

    };
    ParamsPhy param_phy = {
        .type_modulation = TypeModulation::QPSK,
        .param_ofdm = param_ofdm,
    };
    print_log(LOG_DATA, "size = %d, data: %s\n", data.size, data.buffer);
    VecSymbolMod samples_tx;
    OFDM_symbol samples = generate_frame_phy(data, param_phy, samples_tx);
    // exit(0);

    config_device cfg1 = {
        ip_device,
        {MHZ(2), MHZ(2.5), GHZ(1.9), "A_BALANCED"},
        {MHZ(1.5), MHZ(2.5), GHZ(1.9), "A"},
        1024 * 1024,
    };
    print_cfg_dev(cfg1.tx_cfg);
    // return -1;
    if(DeviceTRX::initialization(cfg1)){
        print_log(CONSOLE, "[%s:%d] Error: initialization, exit program\n",
            __func__, __LINE__);

        return -1;
    }
    int result;

#if 1
    VecSymbolMod samples_rx;
    while(1) {
        result = DeviceTRX::recv_samples(NULL, 0);
    }
    print_log(LOG, "[%s:%d] RX: result = %d\n", __func__, __LINE__, result);
#endif

#if 0

    bit_sequence *read_data = decode_frame_phy(samples, param_phy);
    print_log(LOG_DATA, "read_data: size = %d, data: %s\n", read_data->size, read_data->buffer);
    print_log(CONSOLE, "input size: %d, output size: %d\n", data.size, read_data->size);
    delete[] read_data;
#endif
    // write_file_bin_data(
    //             FILE_NAME_SAVE_MODULATION, &samples_mod[0], 
    //             samples_mod.size() * sizeof(VecSymbolMod::value_type) );
    DeviceTRX::deinitialization();
    
    print_log(CONSOLE, "End %s\n", __func__);
    return 0;
}

int realtime_RX(int argc, char *argv[]) {
    if(argc < static_cast<int>(ARGV_CONSOLE::ARGV_MAX)){
        printf("Error: Not enough arguments: <ip> <filename data>\n");
        return -1;
    }

    const char *file_conf = argv[static_cast<int>(ARGV_CONSOLE::ARGV_FILE_CONFIG)];
    config_program param = configure(file_conf);
    init_log(param.file_log.c_str());
    ParamsPhy param_phy = {
        .type_modulation = param.type_modulation,
        .param_ofdm = param.ofdm_params,
    };
    config_device cfg1 = {
        param.address.c_str(),
        {MHZ(2), MHZ(2.5), GHZ(1.9), "A_BALANCED"},
        {MHZ(1.5), MHZ(2.5), GHZ(1.9), "A"},
        1024 * 1024,
        // 200000,
    };
    print_cfg_dev(cfg1.tx_cfg);
    if(DeviceTRX::initialization(cfg1)){
        print_log(CONSOLE, "[%s:%d] Error: initialization, exit program\n",
            __func__, __LINE__);

        return -1;
    }
    print_log(CONSOLE, "[%s:%d] \n", __func__, __LINE__);
    if(init_ipc()) {
        print_log(CONSOLE, "Exit\n");
        return -1;
    }
    

    int result;
    VecSymbolMod samples_rx(cfg1.block_size);
    print_log(CONSOLE, "size = %d\n", samples_rx.size());
    
    int command = 0;
    int attemps = 0;
    while(1) {


        if(attemps > 0) {
            attemps--;
        } else {
            command = 0;
        }

        if(command != 12) {
            print_log(CONSOLE, "%s:%d: input:", __func__, __LINE__);
            std::cin >> command;
            if(command == 12) {
                attemps = 5;
            }
        } else {
            // sleep(1)
        }
        std::string str = "Test complex";
        result = DeviceTRX::recv_samples(samples_rx, cfg1.block_size);
        data_array d3 = data_array(str.size(), (const u_char*)str.c_str(), 8 * samples_rx.size(), 
            static_cast<u_char>(TYPE_ARRAY::TYPE_COMPLEX_FLOAT), (u_char*)&samples_rx[0]);
        print_log(LOG, "rx data: %d\n", samples_rx.size());
        write_file_bin_data("../data/rx_sample.bin", 
            (void*)&samples_rx[0], samples_rx.size() * 2 * 4);
        std::vector<data_array*> arr;
        arr.push_back(&d3);
        time_counting_start();
        full_data_arrays("realtime rx", arr);
        time_counting_end(CONSOLE, __func__);

        if(command) {
            print_log(CONSOLE, "send\n");
            std::string asd = "asdfasfdsdf";
            send_ipc(command, 4, 12, (u_char*)asd.c_str());
            if(command == 10) {
                break;
            }
            if(command == 12) {
                msg_header msg;
                recv_ipc(&msg, 0, NULL);
            }
#if 0
            msg_header msg;
            recv_ipc(&msg, 0, NULL);
            for(int i = 0; i < sizeof(msg); ++i) {
        print_log(CONSOLE, "%d) %x %d\n",i,
         *((char*)(&msg) + i),  *((char*)(&msg) + i));
    }
    print_log(CONSOLE, "\nsizeof(msg_con) = %d\n", sizeof(msg));
            print_log(CONSOLE, "c = %d, t = %d, s = %d\n", msg.command, msg.type, msg.size_data_shm);
#endif
        }
    }

    deinit_ipc();
    DeviceTRX::deinitialization();
    print_log(CONSOLE, "End %s\n", __func__);
    deinit_log();
    return 0;
}

int test_TX(int argc, char *argv[]){

    const char *file_conf = argv[static_cast<int>(ARGV_CONSOLE::ARGV_FILE_CONFIG)];
    config_program param = configure(file_conf);
    init_log(param.file_log.c_str());

#if 1
    const char *filename = argv[static_cast<int>(ARGV_CONSOLE::ARGV_FILE_DATA)];
#else
    const char *filename = "../data/data_test.txt";
#endif

#if 0
    char test_data[] = "Test m";
#else    
    char test_data[100];
    for(int i = 0; i < (int)sizeof(test_data); ++i){
        // test_data[i] = (rand() % 200);
        test_data[i] = (rand() % 20) + 70;
    }
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
#else

    // const char *file_conf = argv[static_cast<int>(ARGV_CONSOLE::ARGV_FILE_CONFIG)];

    // config_program param = configure(file_conf);
    ParamsPhy param_phy = {
        .type_modulation = param.type_modulation,
        .param_ofdm = param.ofdm_params,
    };

#endif
    print_log(LOG_DATA, "size = %d, data: %s\n", data.size, data.buffer);
    VecSymbolMod samples_tx;
    OFDM_symbol ofdms = generate_frame_phy(data, param_phy, samples_tx);
    VecSlotsOFDM slots = create_slots(ofdms, param.ofdm_params);
    print_log(CONSOLE, "count slots: %d, count ofdm symbol: %d\n", slots.size(), ofdms.size());
    addPowerSlots(slots, param.ofdm_params.power);
    print_log(CONSOLE, "add power: %d\n", param.ofdm_params.power);
    
    write_OFDM_slots("../data/slots_tx.bin", slots, slots.size());

    VecSymbolMod samples = slots_OFDM_convertion_one_thread(slots);
    write_file_bin_data("../data/samples_tx.bin", 
            (void*)&samples[0], samples.size() * sizeof(mod_symbol));
    // config_device cfg1 = {
    //     param.address.c_str(),
    //     {MHZ(2), MHZ(2.5), GHZ(1.9), "A_BALANCED"},
    //     {MHZ(1.5), MHZ(2.5), GHZ(1.9), "A"},
    //     // 1024 * 1024,
    //     160000
        
    // };

    config_device cfg1 = {
        param.address.c_str(),
        {MHZ(2), MHZ(2.5), GHZ(1.9), "A_BALANCED"},
        {MHZ(1.5), MHZ(2.5), GHZ(1.9), "A"},
        // 1024 * 1024,
        160000
        
    };

    print_cfg_dev(cfg1.tx_cfg);
    // return -1;
    if(DeviceTRX::initialization(cfg1)){
        print_log(CONSOLE, "[%s:%d] Error: initialization, exit program\n",
            __func__, __LINE__);

        return -1;
    }
    int result;
#if 0
    result = DeviceTRX::send_samples((void*)&samples[0], samples.size());

    print_log(LOG, "[%s:%d] TX: result = %d\n", __func__, __LINE__, result);
#endif
    
    // DeviceTRX::while_send_samples((void*)&samples[0], samples.size() * sizeof(mod_symbol));
    // DeviceTRX::while_send_samples(slots);
    // DeviceTRX::while_send_samples(samples);
    print_log(CONSOLE, "============================%d", samples.size());
    // for(int i = 0; i < 2; ++i) {
    //     VecSymbolMod s(samples.begin(), samples.end());
    //     samples.insert(samples.end(), s.begin(), s.end());
    // }
        
    print_log(CONSOLE, "----------------------------%d\n", samples.size());
    while(1) {
        DeviceTRX::send_samples((void*)&samples[0], samples.size() * sizeof(mod_symbol));
    }
    DeviceTRX::deinitialization();
    
    print_log(CONSOLE, "End %s\n", __func__);
    deinit_log();
    return 0;
}


int trx_test(int argc, char *argv[]){

#if 0
    if(argc < static_cast<int>(ARGV_CONSOLE::ARGV_MAX)){
        print_log(CONSOLE, "Error: Not enough arguments: <ip> <filename data>\n");
        return -1;
    }
    const char *ip_device = argv[static_cast<int>(ARGV_CONSOLE::ARGV_IP_DEVICE)];
    const char *filename = argv[static_cast<int>(ARGV_CONSOLE::ARGV_FILE_DATA)];
#else
    const char *ip_device = "ip:192.168.3.1";
    const char *filename = "../data/data_test.txt";

#endif


#if 1
    char test_data[] = "Test message";
#else    
    char test_data[310];
    for(int i = 0; i < (int)sizeof(test_data); ++i){
        // test_data[i] = (rand() % 200);
        test_data[i] = (rand() % 20) + 70;
    }
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

    };
    ParamsPhy param_phy = {
        .type_modulation = TypeModulation::QPSK,
        .param_ofdm = param_ofdm,
    };
    print_log(LOG_DATA, "size = %d, data: %s\n", data.size, data.buffer);
    VecSymbolMod samples_tx;
    OFDM_symbol samples = generate_frame_phy(data, param_phy, samples_tx);
    // exit(0);

    config_device cfg1 = {
        ip_device,
        {MHZ(2), MHZ(2.5), GHZ(1.9), "A_BALANCED"},
        {MHZ(2), MHZ(2.5), GHZ(1.9), "A"},
        1024 * 1024,
    };
    print_cfg_dev(cfg1.tx_cfg);
    // return -1;
    if(DeviceTRX::initialization(cfg1)){
        print_log(CONSOLE, "[%s:%d] Error: initialization, exit program\n",
            __func__, __LINE__);

        return -1;
    }
    int result;
#if 0
    result = DeviceTRX::send_samples((void*)&samples[0], samples.size());

    print_log(LOG, "[%s:%d] TX: result = %d\n", __func__, __LINE__, result);
#endif
    // DeviceTRX::while_send_samples((void*)&samples[0], samples.size());
#if 1
    VecSymbolMod samples_rx;
    result = DeviceTRX::recv_samples(NULL, 0);
    print_log(LOG, "[%s:%d] RX: result = %d\n", __func__, __LINE__, result);
#endif

#if 0
    print_VecSymbolMod(samples);
    
    bit_sequence *read_data = decode_frame_phy(samples, param_phy);
    print_log(LOG_DATA, "read_data: size = %d, data: %s\n", read_data->size, read_data->buffer);
    print_log(CONSOLE, "input size: %d, output size: %d\n", data.size, read_data->size);
    delete[] read_data;
#endif
    // write_file_bin_data(
    //             FILE_NAME_SAVE_MODULATION, &samples_mod[0], 
    //             samples_mod.size() * sizeof(VecSymbolMod::value_type) );
    DeviceTRX::deinitialization();
    
    print_log(CONSOLE, "End %s\n", __func__);
    return 0;
}






