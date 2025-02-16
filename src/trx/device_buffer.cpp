#include "device_buffer.hpp"

#include <errno.h>
#include <cstring>
#include <ctime>
#include <unistd.h>
#include <thread>

#include "device_api.hpp"
#include "ofdm.hpp"
#include "ring_buffer.hpp"
#include "types_trx.hpp"

#include <output.hpp>

enum TASK_ID {
    READ_DEV = 0,
    WRITE_DEV,
    ENCODE_MSG,
    DECODE_MSG,
    
};

enum CON_THREAD {
    NONE = 0,
    ACTION,
    SLEEP,
    SHIFT
};

typedef RingBuffer RingStruct;

bool RUN_CON = 0;

static RingMsg *ring_send_msg = nullptr;
static RingMsg *ring_recv_msg = nullptr;

static RingSamples *ring_send_samples = nullptr;
static RingSamples *ring_recv_samples = nullptr;

context *DeviceBuffer::ctx_dev = nullptr;

static const int count_task = 4;

static int shift_time_ring[count_task] = {0, 0, 0, 0};
static int con_shift_time_ring[count_task] = {NONE, NONE, NONE, NONE};

static std::thread t_write_dev;
static std::thread t_enc_msg;
static std::thread t_read_dev;
static std::thread t_dec_msg;



void thread_read_device();
void thread_write_device();
void thread_decode_msg();
void thread_encode_msg();


int DeviceBuffer::initialization(context &ctx) {   
    // int size_buffer_phy =  
    ring_send_samples = new RingSamples(
        ctx.cfg_buf.tx.size, 1,
        sizeof(phy_buffer) + ctx.cfg_device.tx_cfg.block_size * sizeof(mod_symbol));
    ring_recv_samples = new RingSamples(
        ctx.cfg_buf.rx.size, 1,
        sizeof(phy_buffer) + ctx.cfg_device.rx_cfg.block_size * sizeof(mod_symbol));
    ring_send_msg = new RingMsg(
        ctx.cfg_buf.tx.size, 1, sizeof(msg_buffer));
    ring_recv_msg = new RingMsg(
        ctx.cfg_buf.rx.size, 1, sizeof(msg_buffer));
    
    ctx_dev = &ctx;
    RUN_CON = true;
    t_write_dev = std::thread(thread_write_device);
    t_read_dev = std::thread(thread_read_device);
    t_enc_msg = std::thread(thread_encode_msg);

    for(int i = 0; i < count_task; i++) {
        con_shift_time_ring[i] = ACTION;
    }
    // t_read_dev = std::thread(thread_write_device);
    
    
    return DEVICE_PHY::DeviceTRX::initialization(ctx.cfg_device);
}

int DeviceBuffer::deinitialization(context &ctx) {
    return DEVICE_PHY::DeviceTRX::deinitialization();
}

int DeviceBuffer::send_msg(const msg_buffer *msg) {
    return ring_send_msg->Push(msg);
}

int DeviceBuffer::recv_msg(msg_buffer *msg) {
    return ring_recv_msg->Pop(msg);
}

const config_buffer *DeviceBuffer::get_cfg_buf() {
    return &ctx_dev->cfg_buf;
}

context *DeviceBuffer::get_context() {
    return ctx_dev;
}

void thread_read_device() {
    int wait_time = DeviceBuffer::get_cfg_buf()->rx.interval_of_with_dev;
    int (*read_radio_ptr)(void*, size_t) = DEVICE_PHY::DeviceTRX::recv_samples;
    while(RUN_CON) {
        usleep(wait_time);
        switch (con_shift_time_ring[READ_DEV])
        {
        case NONE:
            break;
        case ACTION:
            ring_recv_samples->Push(read_radio_ptr);
            break;
        case SLEEP:

            break;
        case SHIFT:

            break;
        default:
            break;
        }

    }
}

void thread_write_device() {
    int wait_time = DeviceBuffer::get_cfg_buf()->tx.interval_of_with_dev;
    int (*write_radio_ptr)(const void*, size_t) = DEVICE_PHY::DeviceTRX::send_samples;
    while(RUN_CON) {
        usleep(wait_time);
        switch (con_shift_time_ring[WRITE_DEV])
        {
        case NONE:
            break;
        case ACTION:
            ring_send_samples->Pop(write_radio_ptr);
            break;
        case SLEEP:

            break;
        case SHIFT:

            break;
        default:
            break;
        }
    }
}

void thread_decode_msg() {

    u_char *buffer = new u_char[ sizeof(phy_buffer) +
        DeviceBuffer::get_context()->cfg_device.rx_cfg.block_size * sizeof(mod_symbol)];
    phy_buffer *phy_h = (phy_buffer*)buffer;
    u_char *data = sizeof(phy_buffer) + buffer; 
    while(RUN_CON) {
        switch (con_shift_time_ring[DECODE_MSG])
        {
        case NONE:
            break;
        case ACTION:
            
            ring_recv_samples->Pop(buffer);

            

            break;
        case SLEEP:

            break;
        case SHIFT:

            break;
        default:
            break;
        }
    }
    delete[] buffer;
}

void thread_encode_msg() {

    u_char *buffer = new u_char[ sizeof(phy_buffer) +
        DeviceBuffer::get_context()->cfg_device.tx_cfg.block_size * sizeof(mod_symbol)];
    phy_buffer *phy_h = (phy_buffer*)buffer;
    u_char *data = sizeof(phy_buffer) + buffer;
    msg_buffer msg;
    VecSymbolMod samples;
    while(RUN_CON) {
        switch (con_shift_time_ring[ENCODE_MSG])
        {
        case NONE:
            break;
        case ACTION:
            if(ring_send_msg->Pop(&msg)) {
                samples = convert_msg_to_samples(
                    *DeviceBuffer::get_context(), msg.data, msg.size);
                phy_h->size = samples.size() * sizeof(mod_symbol);
                
                memcpy(data, (void*)&samples[0], phy_h->size); 
                
                ring_send_samples->Push(buffer);
            }
            break;
        case SLEEP:

            break;
        case SHIFT:

            break;
        default:
            break;
        }
    }
    delete[] buffer;
}


















