#include "channel_model.hpp"

#include <random>
#include <cstring>
#include <unistd.h>
#include <thread>
#include <mutex>
#include <stdarg.h>
#include <csignal>
#include <zmq.hpp>

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

#define SERVER_PORT 19856

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

enum class TYPE_MSG {
    SEND_DATA,
    GET_DATA
};

struct header {
    u_char type;
    uint size_data;
};

static param_model prm_mod;
static std::thread thr;
std::mutex mutex_buffer;
static VecSymbolMod buffer_channel_model; 
static bool running;
static zmq::socket_t socket;
static zmq::context_t context_zmq;
static void print_log_channel(int out, const char* format, ...) {
    va_list ap;
    va_start(ap, format);
    print_log(out, format, ap);
    va_end(ap);
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
        mutex_buffer.lock();
        update_channel(prm_mod.delete_elem_cycle);
        dump_buffer();
        mutex_buffer.unlock();
        usleep(prm_mod.time_update_buffer);
    }
}

int CHANNEL_MODEL::model_channel_init(context &cfg_dev) {
    context_zmq = zmq::context_t(1);
    socket = zmq::socket_t(context_zmq, ZMQ_REQ);
    std::string addr_send = "tcp://localhost:" + std::to_string(SERVER_PORT);
    socket.connect(addr_send);
    return STATUS_ACCESS;
}

int CHANNEL_MODEL::model_channel_deinit(context &cfg_dev) {

    return STATUS_ACCESS;
}

int CHANNEL_MODEL::read_channel(VecSymbolMod &samples, size_t size) {

    // std::copy(buffer_channel_model.end() - size, buffer_channel_model.end(),
    //     samples.begin());
    // update_channel(size);
    header msg;
    msg.type = static_cast<u_char>(TYPE_MSG::GET_DATA);
    msg.size_data = size;
    zmq::message_t message(sizeof(header));
    memcpy(message.data(), (void*)&msg, sizeof(header));
    socket.send(message, zmq::send_flags::none);
    zmq::message_t response_msg;
    header msg_resp;
    socket.recv(response_msg);
    memcpy((void*)&msg_resp, response_msg.data(), sizeof(header)/*?*/);
    if(msg_resp.type == static_cast<u_char>(TYPE_MSG::SEND_DATA)) {
        memcpy((void*)&samples, response_msg.data() + sizeof(header), msg.size_data * sizeof(mod_symbol));
        return msg.size_data;
    }
    return 0;
}

int CHANNEL_MODEL::write_channel(const VecSymbolMod &samples, size_t size) {
    header msg;
    msg.type = static_cast<u_char>(TYPE_MSG::SEND_DATA);
    msg.size_data = size;
    uint size_data = size * sizeof(mod_symbol);
    zmq::message_t message(sizeof(header) + size_data);
    memcpy(message.data(), (void*)&msg, sizeof(header));
    memcpy(message.data() + sizeof(header), samples.data(), size_data);
    socket.send(message, zmq::send_flags::none);
    // buffer_channel_model.insert(buffer_channel_model.begin(), samples.begin(), samples.begin() + size);
    // buffer_channel_model.resize(prm_mod.buffer_size);
    return size;
}

void listen_channel() {
    while(running) {
        zmq::message_t message_req;
        header msg_cmd;
        socket.recv(message_req);
        memcpy((void*)&msg_cmd, message_req.data(), sizeof(header));
        switch (msg_cmd.type) {
        case static_cast<u_char>(TYPE_MSG::GET_DATA):
            mutex_buffer.lock();
            {
                zmq::message_t message_resp(sizeof(header) + msg_cmd.size_data * sizeof(mod_symbol));
                header msg_resp;
                msg_resp.size_data = msg_cmd.size_data;
                msg_resp.type = static_cast<u_char>(TYPE_MSG::SEND_DATA);
                memcpy(message_resp.data(), (void*)&msg_resp, sizeof(header));
                uint shift = (prm_mod.buffer_size - msg_cmd.size_data) * sizeof(mod_symbol); 
                memcpy(message_resp.data() + sizeof(header),
                    buffer_channel_model.data() + shift, msg_cmd.size_data * sizeof(mod_symbol));
            }
            mutex_buffer.unlock();
            break;
        case static_cast<u_char>(TYPE_MSG::SEND_DATA):
            mutex_buffer.lock();
            {
                VecSymbolMod vec;
                mod_symbol *data = static_cast<mod_symbol*>(message_req.data() + sizeof(header));
                vec.assign(data, data + msg_cmd.size_data * sizeof(mod_symbol));
                buffer_channel_model.insert(buffer_channel_model.begin(), vec.begin(), vec.end());
                buffer_channel_model.resize(prm_mod.buffer_size);
            }
            mutex_buffer.unlock();
            break;
        default:
            break;
        }
    }
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
    std::string addr_send = "tcp://*:" + std::to_string(SERVER_PORT);
    context_zmq = zmq::context_t(1);
    socket = zmq::socket_t(context_zmq, ZMQ_REP);
    socket.bind(addr_send);
    
    running = true;
    channel_change_over_time();

    exit_program();
    return 0;
}




















