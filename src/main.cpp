


// #define CONDITION_TARGET_PROGRAM 3

#if CONDITION_TARGET_PROGRAM != 10

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

#if defined(_WIN64)
#include "windows.h"
#endif

#include "header.hpp"

// #include "output/output.hpp"


// #include "generate_packet/generate_packet.h"
#include "phy/phy.hpp"

#ifdef M_FOUND_LIBIIO

#include "trx/device_api.hpp"
#endif

#include "target.hpp"
#include "model/modelling.hpp"
#include "loaders/load_data.hpp"
#include "ipc/managment_ipc.hpp"
#include "phy/ofdm_modulation.hpp"

#endif

#define LOG_FILE "../log.log"

#define SIZE_DATA 1000
#define STRINGIFY(x) #x

enum class ARGV_CONSOLE: int {
    ARGV_IP_DEVICE = 1,
    ARGV_FILE_DATA,
    ARGV_TARGET_PROGRAM,
    ARGV_FILE_CONFIG,
    ARGV_MAX
};

// struct target_exec {
//     std::string target;

// };



struct config_program {
    std::string file_log;

};
void print_input_param() {
    printf(
        "ARGV: "
        "<ip> <filename data> <target>"
        "\n"
    );
}

config_program configure(int argc, char *argv[]) {
    if(argc < static_cast<int>(ARGV_CONSOLE::ARGV_MAX)){
        print_input_param();
        exit(0);
    }
    const char *file_conf = argv[static_cast<int>(ARGV_CONSOLE::ARGV_FILE_CONFIG)];
    std::ifstream file(file_conf);
    config_program param; 
    if(!file.is_open()) {
        printf("Error open: %s\n", file_conf);
        return param;
    }
    std::string buffer;
    file >> buffer;
    if(buffer == "log_file") {
        file >> param.file_log;
        std::cout<<param.file_log<<"\n";
    }
    file.close();
    return param;
}

int target(int argc, char *argv[]) {
    if(argc == static_cast<int>(ARGV_CONSOLE::ARGV_MAX) + 1) {
        return modelling_signal(argv[static_cast<int>(ARGV_CONSOLE::ARGV_MAX)][0]);
    }
    print_log(CONSOLE, "No option for modelling_signal\n");
    return 0;
}
int ofdm_model(int argc, char *argv[]);
#if M_FOUND_LIBIIO
int trx_test(int argc, char *argv[]);
int test_RX(int argc, char *argv[]);
int test_TX(int argc, char *argv[]);
int realtime_RX(int argc, char *argv[]);
#endif
int test_ipc(int argc, char *argv[]);


int main(int argc, char *argv[]){
    config_program param = configure(argc, argv);
    srand(time(NULL));

    init_log(param.file_log.c_str());

    std::map<std::string, std::function<int(int, char**)>> target_exec = {
        {"model_practice", target},
        {"ofdm_model", ofdm_model},
        {"fpss", finding_pss},
#if M_FOUND_LIBIIO
        {"trx", trx_test},
        {"rx", test_RX},
        {"tx", test_TX},
        {"realtime", realtime_RX},
#endif
        {"test_ipc", test_ipc}
    };


    auto find_taget_exec = target_exec.find(
        argv[static_cast<int>(ARGV_CONSOLE::ARGV_TARGET_PROGRAM)]);
    if(find_taget_exec != target_exec.end()) {
        print_log(CONSOLE, "call target: %s\n", find_taget_exec->first.c_str());
        find_taget_exec->second(argc, argv);
    } else {
        print_log(CONSOLE, "No target: %s\n", 
            argv[static_cast<int>(ARGV_CONSOLE::ARGV_TARGET_PROGRAM)] );
    }
    

    print_log(CONSOLE, "End program\n");
    deinit_log();
    return 0;
}







/*Убрать*/
#ifndef M_FOUND_LIBIIO
#define M_FOUND_LIBIIO
#endif

#ifdef M_FOUND_LIBIIO


void print_cfg1(stream_cfg &txcfg){
    print_log(LOG_DEVICE, "bw_hz = %lld, fs_hz = %lld, lo_hz = %lld, rfport = %s\n", 
        txcfg.bw_hz, txcfg.fs_hz, txcfg.lo_hz, txcfg.rfport
    );
}
void print_VecSymbolMod1( VecSymbolMod &vec) {
    for(int i = 0; i < (int)vec.size(); ++i) {
        print_log(CONSOLE, "%f + %fi  ", vec[i].real(), vec[i].imag());
    }
    print_log(CONSOLE, "\n");
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
    OFDM_symbol samples = generate_frame_phy(data, param_phy);
    // exit(0);

    config_device cfg1 = {
        ip_device,
        {MHZ(2), MHZ(2.5), GHZ(1.9), "A_BALANCED"},
        {MHZ(2), MHZ(2.5), GHZ(1.9), "A"},
        1024 * 1024,
    };
    print_cfg1(cfg1.tx_cfg);
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




int test_TX(int argc, char *argv[]){

#if 1
    const char *ip_device = argv[static_cast<int>(ARGV_CONSOLE::ARGV_IP_DEVICE)];
    const char *filename = argv[static_cast<int>(ARGV_CONSOLE::ARGV_FILE_DATA)];
#else
    const char *ip_device = "ip:192.168.3.1";
    const char *filename = "../data/data_test.txt";

#endif
    print_log(LOG, "address dev: %s\n", ip_device);

#if 1
    char test_data[] = "Test m";
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
    // OFDM_params param_ofdm = {
    //     .count_subcarriers = 64,
    //     .pilot = {5, 5},
    //     .step_RS = 8,
    //     .def_interval = 16,
    //     .cyclic_prefix = 15,
    //     .power = 3000,
    // };
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
    print_log(LOG_DATA, "size = %d, data: %s\n", data.size, data.buffer);
    OFDM_symbol samples = generate_frame_phy(data, param_phy);
    // exit(0);
    VecSlotsOFDM slots = create_slots(samples);
    // addPower(s, 4000);
    config_device cfg1 = {
        ip_device,
        {MHZ(2), MHZ(2.5), GHZ(1.9), "A_BALANCED"},
        {MHZ(2), MHZ(2.5), GHZ(1.9), "A"},
        1024 * 1024,
    };
    print_cfg1(cfg1.tx_cfg);
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
    DeviceTRX::while_send_samples(slots);
    
    while(1){}
    DeviceTRX::deinitialization();
    
    print_log(CONSOLE, "End %s\n", __func__);

    return 0;
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
    OFDM_symbol samples = generate_frame_phy(data, param_phy);
    // exit(0);

    config_device cfg1 = {
        ip_device,
        {MHZ(2), MHZ(2.5), GHZ(1.9), "A_BALANCED"},
        {MHZ(2), MHZ(2.5), GHZ(1.9), "A"},
        1024 * 1024,
    };
    print_cfg1(cfg1.tx_cfg);
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
        print_log(CONSOLE, "Error: Not enough arguments: <ip> <filename data>\n");
        return -1;
    }
#if 1
    const char *ip_device = argv[static_cast<int>(ARGV_CONSOLE::ARGV_IP_DEVICE)];
    const char *filename = argv[static_cast<int>(ARGV_CONSOLE::ARGV_FILE_DATA)];
    print_log(LOG, "Device addres: %s\n", ip_device);
#else
    const char *ip_device = "ip:192.168.2.1";
    const char *filename = "../data/data_test.txt";

#endif
    OFDM_params param_ofdm = {
        .count_subcarriers = 64,
        .pilot = {4000, 4000},

    };
    ParamsPhy param_phy = {
        .type_modulation = TypeModulation::QPSK,
        .param_ofdm = param_ofdm,
    };
    config_device cfg1 = {
        ip_device,
        {MHZ(2), MHZ(2.5), GHZ(1.9), "A_BALANCED"},
        {MHZ(2), MHZ(2.5), GHZ(1.9), "A"},
        1024 * 1024,
    };
    print_cfg1(cfg1.tx_cfg);
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
    


    // Sleep(4 * 1000);

    
    // while(1) {
        
    //     // result = DeviceTRX::recv_samples(samples_rx, cfg1.block_size);

    // }
    int command = 0;
    int attemps;
    while(1) {
        

#if 0
        char test_data[] = "Test message";
#else    
        char test_data[50];
        for(int i = 0; i < (int)sizeof(test_data); ++i){
            // test_data[i] = (rand() % 200);
            test_data[i] = (rand() % 20) + 70;
        }
        test_data[sizeof(test_data) - 1] = 0;
#endif
        write_file_bin_data(filename, test_data, sizeof(test_data));

        bit_sequence data;

        data.buffer = read_file_data(filename, &data.size);
        std::vector<data_array*> arr;
        // OFDM_symbol samples = generate_frame_phy(data, param_phy);
        std::string str = "Test complex";
        data_array d2;// = data_array(str.size(), (const u_char*)str.c_str(), 8 * samples.size(), 
            //static_cast<u_char>(TYPE_ARRAY::TYPE_COMPLEX_FLOAT), (u_char*)&samples[0]);
        

        result = DeviceTRX::recv_samples(samples_rx, cfg1.block_size);
        data_array d3 = data_array(str.size(), (const u_char*)str.c_str(), 8 * samples_rx.size(), 
            static_cast<u_char>(TYPE_ARRAY::TYPE_COMPLEX_FLOAT), (u_char*)&samples_rx[0]);
        print_log(LOG, "rx data: %d\n", samples_rx.size());
        write_file_bin_data("../data/rx_sample.bin", 
            (void*)&samples_rx[0], samples_rx.size() * 2 * 4);
        arr.push_back(&d3);

        // receiver_OFDM(samples_rx, param_ofdm);
        OFDM_symbol ofdms;
        // result = receiver_OFDM(samples_rx, param_ofdm, ofdms);

        time_counting_start();
        full_data_arrays("realtime rx", arr);
        time_counting_end(CONSOLE, __func__);

        if(attemps > 0) {
            attemps--;
        } else {
            command = 0;
        }

        if(command != 12) {
            print_log(CONSOLE, "%s:%d: input:", __func__, __LINE__);
            std::cin >> command;
            if(command == 12) {
                attemps = 10;
            }
        } else {
            // sleep(1)
        }

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
}



#if CONDITION_TARGET_PROGRAM == 10


// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * libiio - AD9361 IIO streaming example
 *
 * Copyright (C) 2014 IABG mbH
 * Author: Michael Feilen <feilen_at_iabg.de>
 **/

#include <stdbool.h>
#include <stdint.h>
#include <string.h>
#include <signal.h>
#include <stdio.h>
#include <iio.h>

/* helper macros */
#define MHZ(x) ((long long)(x*1000000.0 + .5))
#define GHZ(x) ((long long)(x*1000000000.0 + .5))

#define IIO_ENSURE(expr) { \
	if (!(expr)) { \
		(void) fprintf(stderr, "assertion failed (%s:%d)\n", __FILE__, __LINE__); \
		(void) abort(); \
	} \
}

/* RX is input, TX is output */
enum iodev { RX, TX };

/* common RX and TX streaming params */
struct stream_cfg {
	long long bw_hz; // Analog banwidth in Hz
	long long fs_hz; // Baseband sample rate in Hz
	long long lo_hz; // Local oscillator frequency in Hz
	const char* rfport; // Port name
};

/* static scratch mem for strings */
static char tmpstr[64];

/* IIO structs required for streaming */
static struct iio_context *ctx   = NULL;
static struct iio_channel *rx0_i = NULL;
static struct iio_channel *rx0_q = NULL;
static struct iio_channel *tx0_i = NULL;
static struct iio_channel *tx0_q = NULL;
static struct iio_buffer  *rxbuf = NULL;
static struct iio_buffer  *txbuf = NULL;

static bool stop;

/* cleanup and exit */
static void shutdown(void)
{
	printf("* Destroying buffers\n");
	if (rxbuf) { iio_buffer_destroy(rxbuf); }
	if (txbuf) { iio_buffer_destroy(txbuf); }

	printf("* Disabling streaming channels\n");
	if (rx0_i) { iio_channel_disable(rx0_i); }
	if (rx0_q) { iio_channel_disable(rx0_q); }
	if (tx0_i) { iio_channel_disable(tx0_i); }
	if (tx0_q) { iio_channel_disable(tx0_q); }

	printf("* Destroying context\n");
	if (ctx) { iio_context_destroy(ctx); }
	exit(0);
}

static void handle_sig(int sig)
{
	printf("Waiting for process to finish... Got signal %d\n", sig);
	stop = true;
}

/* check return value of attr_write function */
static void errchk(int v, const char* what) {
	 if (v < 0) { fprintf(stderr, "Error %d writing to channel \"%s\"\nvalue may not be supported.\n", v, what); shutdown(); }
}

/* write attribute: long long int */
static void wr_ch_lli(struct iio_channel *chn, const char* what, long long val)
{
	errchk(iio_channel_attr_write_longlong(chn, what, val), what);
}

/* write attribute: string */
static void wr_ch_str(struct iio_channel *chn, const char* what, const char* str)
{
	errchk(iio_channel_attr_write(chn, what, str), what);
}

/* helper function generating channel names */
static char* get_ch_name(const char* type, int id)
{
	snprintf(tmpstr, sizeof(tmpstr), "%s%d", type, id);
	return tmpstr;
}

/* returns ad9361 phy device */
static struct iio_device* get_ad9361_phy(void)
{
	struct iio_device *dev =  iio_context_find_device(ctx, "ad9361-phy");
	IIO_ENSURE(dev && "No ad9361-phy found");
	return dev;
}

/* finds AD9361 streaming IIO devices */
static bool get_ad9361_stream_dev(enum iodev d, struct iio_device **dev)
{
	switch (d) {
	case TX: *dev = iio_context_find_device(ctx, "cf-ad9361-dds-core-lpc"); return *dev != NULL;
	case RX: *dev = iio_context_find_device(ctx, "cf-ad9361-lpc");  return *dev != NULL;
	default: IIO_ENSURE(0); return false;
	}
}

/* finds AD9361 streaming IIO channels */
static bool get_ad9361_stream_ch(enum iodev d, struct iio_device *dev, int chid, struct iio_channel **chn)
{
	*chn = iio_device_find_channel(dev, get_ch_name("voltage", chid), d == TX);
	if (!*chn)
		*chn = iio_device_find_channel(dev, get_ch_name("altvoltage", chid), d == TX);
	return *chn != NULL;
}

/* finds AD9361 phy IIO configuration channel with id chid */
static bool get_phy_chan(enum iodev d, int chid, struct iio_channel **chn)
{
	switch (d) {
	case RX: *chn = iio_device_find_channel(get_ad9361_phy(), get_ch_name("voltage", chid), false); return *chn != NULL;
	case TX: *chn = iio_device_find_channel(get_ad9361_phy(), get_ch_name("voltage", chid), true);  return *chn != NULL;
	default: IIO_ENSURE(0); return false;
	}
}

/* finds AD9361 local oscillator IIO configuration channels */
static bool get_lo_chan(enum iodev d, struct iio_channel **chn)
{
	switch (d) {
	 // LO chan is always output, i.e. true
	case RX: *chn = iio_device_find_channel(get_ad9361_phy(), get_ch_name("altvoltage", 0), true); return *chn != NULL;
	case TX: *chn = iio_device_find_channel(get_ad9361_phy(), get_ch_name("altvoltage", 1), true); return *chn != NULL;
	default: IIO_ENSURE(0); return false;
	}
}

/* applies streaming configuration through IIO */
bool cfg_ad9361_streaming_ch1(struct stream_cfg *cfg, enum iodev type, int chid)
{
	struct iio_channel *chn = NULL;

	// Configure phy and lo channels
	printf("* Acquiring AD9361 phy channel %d\n", chid);
	if (!get_phy_chan(type, chid, &chn)) {	return false; }
	wr_ch_str(chn, "rf_port_select",     cfg->rfport);
	wr_ch_lli(chn, "rf_bandwidth",       cfg->bw_hz);
	wr_ch_lli(chn, "sampling_frequency", cfg->fs_hz);

	// Configure LO channel
	printf("* Acquiring AD9361 %s lo channel\n", type == TX ? "TX" : "RX");
	if (!get_lo_chan(type, &chn)) { return false; }
	wr_ch_lli(chn, "frequency", cfg->lo_hz);
	return true;
}

/* simple configuration and streaming */
/* usage:
 * Default context, assuming local IIO devices, i.e., this script is run on ADALM-Pluto for example
 $./a.out
 * URI context, find out the uri by typing `iio_info -s` at the command line of the host PC
 $./a.out usb:x.x.x
 */
int main (int argc, char **argv)
{
	// Streaming devices
	struct iio_device *tx;
	struct iio_device *rx;
    const char *ip = "ip:192.168.3.1";
	// RX and TX sample counters    
	size_t nrx = 0;
	size_t ntx = 0;

	// Stream configurations
	struct stream_cfg rxcfg;
	struct stream_cfg txcfg;

	// Listen to ctrl+c and IIO_ENSURE
	signal(SIGINT, handle_sig);

	// RX stream config
	rxcfg.bw_hz = MHZ(2);   // 2 MHz rf bandwidth
	rxcfg.fs_hz = MHZ(2.5);   // 2.5 MS/s rx sample rate
	rxcfg.lo_hz = GHZ(2.5); // 2.5 GHz rf frequency
	rxcfg.rfport = "A_BALANCED"; // port A (select for rf freq.)

	// TX stream config
	txcfg.bw_hz = MHZ(1.5); // 1.5 MHz rf bandwidth
	txcfg.fs_hz = MHZ(2.5);   // 2.5 MS/s tx sample rate
	txcfg.lo_hz = GHZ(2.5); // 2.5 GHz rf frequency
	txcfg.rfport = "A"; // port A (select for rf freq.)

	printf("* Acquiring IIO context\n");
	if (argc == 1 && 0) {
		IIO_ENSURE((ctx = iio_create_default_context()) && "No context");
	}
	else if (argc == 2 || 1) {
		IIO_ENSURE((ctx = iio_create_context_from_uri(ip)) && "No context");
	}
	IIO_ENSURE(iio_context_get_devices_count(ctx) > 0 && "No devices");

	printf("* Acquiring AD9361 streaming devices\n");
	IIO_ENSURE(get_ad9361_stream_dev(TX, &tx) && "No tx dev found");
	IIO_ENSURE(get_ad9361_stream_dev(RX, &rx) && "No rx dev found");

	printf("* Configuring AD9361 for streaming\n");
	IIO_ENSURE(cfg_ad9361_streaming_ch1(&rxcfg, RX, 0) && "RX port 0 not found");
	IIO_ENSURE(cfg_ad9361_streaming_ch1(&txcfg, TX, 0) && "TX port 0 not found");

	printf("* Initializing AD9361 IIO streaming channels\n");
	IIO_ENSURE(get_ad9361_stream_ch(RX, rx, 0, &rx0_i) && "RX chan i not found");
	IIO_ENSURE(get_ad9361_stream_ch(RX, rx, 1, &rx0_q) && "RX chan q not found");
	IIO_ENSURE(get_ad9361_stream_ch(TX, tx, 0, &tx0_i) && "TX chan i not found");
	IIO_ENSURE(get_ad9361_stream_ch(TX, tx, 1, &tx0_q) && "TX chan q not found");

	printf("* Enabling IIO streaming channels\n");
	iio_channel_enable(rx0_i);
	iio_channel_enable(rx0_q);
	iio_channel_enable(tx0_i);
	iio_channel_enable(tx0_q);

	printf("* Creating non-cyclic IIO buffers with 1 MiS\n");
	rxbuf = iio_device_create_buffer(rx, 1024*1024, false);
	if (!rxbuf) {
		perror("Could not create RX buffer");
		shutdown();
	}
	txbuf = iio_device_create_buffer(tx, 1024*1024, false);
	if (!txbuf) {
		perror("Could not create TX buffer");
		shutdown();
	}

	printf("* Starting IO streaming (press CTRL+C to cancel)\n");
	while (!stop)
	{
		ssize_t nbytes_rx, nbytes_tx;
		char *p_dat, *p_end;
		ptrdiff_t p_inc;

		// Schedule TX buffer
		nbytes_tx = iio_buffer_push(txbuf);
		if (nbytes_tx < 0) { printf("Error pushing buf %d\n", (int) nbytes_tx); shutdown(); }

		// Refill RX buffer
		nbytes_rx = iio_buffer_refill(rxbuf);
		if (nbytes_rx < 0) { printf("Error refilling buf %d\n",(int) nbytes_rx); shutdown(); }

		// READ: Get pointers to RX buf and read IQ from RX buf port 0
		p_inc = iio_buffer_step(rxbuf);
		p_end = (char*)iio_buffer_end(rxbuf);
		for (p_dat = (char *)iio_buffer_first(rxbuf, rx0_i); p_dat < p_end; p_dat += p_inc) {
			// Example: swap i and q
			const int16_t i = ((int16_t*)p_dat)[0]; // Real (i)
			const int16_t q = ((int16_t*)p_dat)[1]; // Imag (q)
			// ((int16_t*)p_dat)[0] = q;
			// ((int16_t*)p_dat)[1] = i;
            printf("%d %d\n", i, q);
		}

		// WRITE: Get pointers to TX buf and write IQ to TX buf port 0
		p_inc = iio_buffer_step(txbuf);
		p_end = (char*)iio_buffer_end(txbuf);
		for (p_dat = (char *)iio_buffer_first(txbuf, tx0_i); p_dat < p_end; p_dat += p_inc) {
			// Example: fill with zeros
			// 12-bit sample needs to be MSB aligned so shift by 4
			// https://wiki.analog.com/resources/eval/user-guides/ad-fmcomms2-ebz/software/basic_iq_datafiles#binary_format
			((int16_t*)p_dat)[0] = 0 << 4; // Real (i)
			((int16_t*)p_dat)[1] = 0 << 4; // Imag (q)
		}

		// Sample counter increment and status output
		nrx += nbytes_rx / iio_device_get_sample_size(rx);
		ntx += nbytes_tx / iio_device_get_sample_size(tx);
		printf("\tRX %8.2f MSmp, TX %8.2f MSmp\n", nrx/1e6, ntx/1e6);
	}

	shutdown();

	return 0;
}


#endif /*CONDITION_TARGET_PROGRAM == 10*/



#endif /*M_FOUND_LIBIIO*/

