#include "channel_model.hpp"

#include <complex>
#include <random>
#include <vector>
#include <cmath>
#include <cstring>
#include <unistd.h>
#include <algorithm>
#include <fftw3.h>
#include <thread>
#include <semaphore.h>
#include <stdarg.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <csignal>

#include <complex>
#include <output.hpp>
#include <complex_container.hpp>
#include <signal_processing.hpp>
#include <configure.hpp>

#include "components.hpp"
#include "../model/modelling.hpp"
#include "../ipc/managment_ipc.hpp"

#define DEBUG_MONITOR

#define SEMAPHORE_NAME "/semaphore_channel_model"
#define SHARED_MEMORY "/shared_memory_channel_model"
#define SIZE_SH_MEMORY 1000000


using namespace CHANNEL_MODEL;

typedef unsigned int uint;

struct param_model {
    uint buffer_size;
    uint time_update_buffer;
    uint delete_elem_cycle;
};

enum class ARGV_CONSOLE {
    ARGV_FILE_CONFIG = 2
};

param_model prm_mod;

static std::thread thr;
static sem_t *semaphore = nullptr;
// static std::mutex mutex_buffer;
static VecSymbolMod buffer_channel_model; 
static bool running;
static bool init_status = false;
static u_char *ptr_sh_mem = nullptr;
static int shm_fd = -1;

static void print_log_channel(int out, const char* format, ...) {
    va_list ap;
    va_start(ap, format);
    print_log(out, format, ap);
    va_end(ap);
}

void channel_phy() {
    
}

void dump_buffer() {
#ifdef DEBUG_MONITOR
    std::string str = "dump";
    data_array d3 = data_array(str.size(), (const u_char*)str.c_str(), 8 * buffer_channel_model.size(), 
            static_cast<u_char>(TYPE_ARRAY::TYPE_COMPLEX_FLOAT), (u_char*)&buffer_channel_model[0]);
    std::vector<data_array*> arr;
    arr.push_back(&d3);
    time_counting_start();
    full_data_arrays("model channel", arr);
    time_counting_end(CONSOLE, __func__);
    std::string asd = "dump";
    send_ipc(12, 4, 12, (u_char*)asd.c_str());

    msg_header msg;
    recv_ipc(&msg, 0, NULL);
#endif
}

static void update_channel(int size) {
    uint new_size = prm_mod.buffer_size - size;
    buffer_channel_model.resize(new_size);
    VecSymbolMod noise = MODEL_COMPONENTS::generate_noise_by_SNR(size, 10);
    buffer_channel_model.insert(buffer_channel_model.begin(), noise.begin(), noise.end());
    print_log_channel(LOG_DATA, "[%s] size buf: %d, shift: %d\n",
        __func__, buffer_channel_model.size(), size);
}

int channel_change_over_time() {

    while(running) {
        sem_wait(semaphore);
        update_channel(prm_mod.delete_elem_cycle);
        dump_buffer();
        sem_post(semaphore);
        usleep(prm_mod.time_update_buffer);
    }
}

int CHANNEL_MODEL::model_channel_init(ATTR_SERVICE::context &cfg_dev) {

    semaphore = sem_open(SEMAPHORE_NAME, 0);
    if (semaphore == SEM_FAILED) {
        perror("sem_open");
        return STATUS_FAIL;
    }
    shm_fd = shm_open(SHARED_MEMORY, O_RDWR, 0);
    if (shm_fd == -1) {
        perror("shm_open");
        return STATUS_FAIL;
    }
    if (ftruncate(shm_fd, SIZE_SH_MEMORY) == -1) {
        perror("ftruncate");
        return STATUS_FAIL;
    }
    ptr_sh_mem = (u_char *)mmap(NULL, SIZE_SH_MEMORY,
        PROT_READ | PROT_WRITE, MAP_SHARED, shm_fd, 0);

    if (ptr_sh_mem == MAP_FAILED) {
        perror("mmap");
        return STATUS_FAIL;
    }
    return STATUS_ACCESS;
}

int CHANNEL_MODEL::model_channel_deinit(ATTR_SERVICE::context &cfg_dev) {
    munmap(ptr_sh_mem, SIZE_SH_MEMORY);
    close(shm_fd);
    sem_close(semaphore);
    return STATUS_ACCESS;
}

int CHANNEL_MODEL::read_channel(VecSymbolMod &samples, size_t size) {
    if(!init_status) {
        print_log_channel(ERROR_OUT, "Error: no initialization channel model\n");
        return -1;
    }
    sem_wait(semaphore);
    // std::copy(buffer_channel_model.end() - size, buffer_channel_model.end(),
    //     samples.begin());
    // update_channel(size);
    dump_buffer();
    sem_post(semaphore);
    return 0;
}

int CHANNEL_MODEL::write_channel(const VecSymbolMod &samples, size_t size) {
    if(!init_status) {
        print_log_channel(ERROR_OUT, "Error: no initialization channel model\n");
        return -1;
    }
    sem_wait(semaphore);

    // buffer_channel_model.insert(buffer_channel_model.begin(), samples.begin(), samples.begin() + size);
    // buffer_channel_model.resize(prm_mod.buffer_size);
    dump_buffer();
    sem_post(semaphore);
    return 0;
}

static int init_ipc_channel_model() {
    /*semaphore*/
    sem_unlink(SEMAPHORE_NAME);
    semaphore = sem_open(SEMAPHORE_NAME, O_CREAT, 0644, 1);
    if(semaphore == SEM_FAILED) {
        perror("sem_open");
        print_log_channel(ERROR_OUT, "Error create semaphore\n");
        return STATUS_FAIL;
    }
    /*shared memory*/
    shm_fd = shm_open(SHARED_MEMORY, O_CREAT | O_RDWR, 0644);
    if(shm_fd == -1) {
        perror("shm_open");
        print_log_channel(ERROR_OUT, "Error create shared memory\n");
        return STATUS_FAIL;
    }
    if (ftruncate(shm_fd, SIZE_SH_MEMORY) == -1) {
        perror("ftruncate");
        return STATUS_FAIL;
    }
    ptr_sh_mem = (u_char *)mmap(NULL, SIZE_SH_MEMORY,
        PROT_READ | PROT_WRITE, MAP_SHARED, shm_fd, 0);

    if (ptr_sh_mem == MAP_FAILED) {
        perror("mmap");
        return STATUS_FAIL;
    }
    return STATUS_ACCESS;
}

static int deinit_ipc_channel_model() {
    sem_close(semaphore);
    sem_unlink(SEMAPHORE_NAME);
    munmap(ptr_sh_mem, SIZE_SH_MEMORY);
    close(shm_fd);
    shm_unlink(SHARED_MEMORY);
    return STATUS_ACCESS;
}

void exit_program() {
    if(running == false) {
        return;
    }
    running = false;
#ifdef DEBUG_MONITOR
    std::string asd = "exit";
    send_ipc(10, 4, 12, (u_char*)asd.c_str());
    deinit_ipc();
#endif
    deinit_ipc_channel_model();
    print_log_channel(LOG, "Exit, terrminating channel model\n");
    ATTR_SERVICE::deinit_log_system();
}

void signalHandler(int signum) {
    exit_program();
    exit(signum);
}

int channel_model(int argc, char *argv[]) {
    const char *file_conf = argv[static_cast<int>(ARGV_CONSOLE::ARGV_FILE_CONFIG)];
    YAML::Node cfg = YAML::LoadFile(file_conf);
    if(!cfg["model_channel"]) {
        print_log_channel(ERROR_OUT, "Error: not found config for model channel\n");
        return STATUS_FAIL;
    }
    EXIT_IF_FAIL(ATTR_SERVICE::init_log_system("../log_channel.log"));
    EXIT_IF_FAIL(init_ipc_channel_model());
    signal(SIGINT, signalHandler);
    signal(SIGTERM, signalHandler);
#ifdef DEBUG_MONITOR
    if(init_ipc()) {
        print_log_channel(CONSOLE, "Exit, error init ipc debug monitor\n");
        return -1;
    }
#endif

    prm_mod.buffer_size = cfg["model_channel"]["buffer_channel"].as<int>();
    prm_mod.delete_elem_cycle = 10000;
    prm_mod.time_update_buffer = 100;
    buffer_channel_model = MODEL_COMPONENTS::generate_noise_by_SNR(
        prm_mod.buffer_size, 10);
    print_log_channel(LOG, "[%s:%d] set size buffer channel model: %d\n",
        __func__, __LINE__, buffer_channel_model.size());
    running = true;
    channel_change_over_time();

    exit_program();
    return 0;
}




















