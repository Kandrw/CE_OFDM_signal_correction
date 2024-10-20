#include "device_api.hpp"

#include <errno.h>
#include <cstring>
#include <ctime>
#include <unistd.h>
#include <thread>

#include "ad9361/device_ad9361.hpp"
#include <output.hpp>

static std::thread thr;

static int run_while_send_samples = false;
static int end_while_send_samples = true;

int DeviceTRX::initialization(config_device &cfg_param){
    print_log(CONSOLE, "[%s:%d]\n", __func__, __LINE__);
    int status = init_device_TRX(cfg_param);
    return status;
}

int DeviceTRX::deinitialization(){
    print_log(CONSOLE, "[%s:%d]\n", __func__, __LINE__);
    if(run_while_send_samples){
        run_while_send_samples = false;
        // thr.join();
        while(!end_while_send_samples);
    }
    shutdown();
    return 0;
}

int DeviceTRX::send_samples(void *samples, size_t size){
    return write_to_device_buffer(samples, size);
}

int thread_send_samples_slots(void *data){
    VecSlotsOFDM &slots = *(VecSlotsOFDM*)data;
    int i = 0;
    while(run_while_send_samples){
        for(int i = 0; i < slots.size(); i++) {
            void *pss = slots[i].PSS.data();
            write_to_device_buffer(pss, slots[i].PSS.size() * 2);
            // print_VecSymbolMod(slots[i].PSS);
            // write_to_device_buffer((void*)&slots[i].PSS[0], slots[i].PSS.size() * 2);
            
            for(int j = 0; j < slots[i].ofdms.size; ++j) {
                void *sample = slots[i].ofdms.symbol[j].data();
                write_to_device_buffer(sample, slots[i].ofdms.symbol[j].size() * 2);
            }
        }

        // write_to_device_buffer(samples, size);
        print_log(LOG, "[%s:%d] [%d] send\n", __func__, __LINE__, i++);
        
    }
    end_while_send_samples = true;
    return 0;
}

int thread_send_samples(void *samples, size_t size){
    int i = 0;
    while(run_while_send_samples){
        write_to_device_buffer(samples, size);
        print_log(LOG, "[%s:%d] [%d] send\n", __func__, __LINE__, i++);
        
    }
    end_while_send_samples = true;
    return 0;
}

int DeviceTRX::while_send_samples(void *samples, size_t size){
    if(1){
        if(!run_while_send_samples){
            run_while_send_samples = true;
            end_while_send_samples = false;
            thr = std::thread(thread_send_samples, samples, size);
            thr.detach();
            sleep(1);
            return 0;
        }
        print_log(CONSOLE, "[%s:%d] Error: already run\n", __func__, __LINE__);
        // thr = std::threadthread_send_samples}; //thread_send_samples( samples, size);
        return 0;
    }
    // run_while_send_samples = false;
    print_log(CONSOLE, "[%s:%d] Error: no init txbuf\n", __func__, __LINE__);
    return -1;
}

int DeviceTRX::while_send_samples(VecSlotsOFDM &slots){
    if(1){
        if(!run_while_send_samples){
            run_while_send_samples = true;
            end_while_send_samples = false;
            thr = std::thread(thread_send_samples_slots, (void*)&slots);
            thr.detach();
            sleep(1);
            return 0;
        }
        print_log(CONSOLE, "[%s:%d] Error: already run\n", __func__, __LINE__);
        // thr = std::threadthread_send_samples}; //thread_send_samples( samples, size);
        return 0;
    }
    // run_while_send_samples = false;
    print_log(CONSOLE, "[%s:%d] Error: no init txbuf\n", __func__, __LINE__);
    return -1;
}

int DeviceTRX::recv_samples(void *samples, size_t size){
    return read_to_device_buffer(samples, size);
}

int DeviceTRX::recv_samples(VecSymbolMod &samples, size_t size){
    if(size == 0) {
        read_to_device_buffer(nullptr, 0);
    }
    if(samples.size() < size) {
        samples.resize(size);
    }
    return read_to_device_buffer((void*)&samples[0], size);
}


