#pragma once

#include "types_trx.hpp"



class DeviceBuffer {
public:




    static int initialization(context &ctx);
    static int deinitialization(context &ctx);
    static int send_msg(const msg_buffer *msg);
    static int recv_msg(msg_buffer *msg);
    static const config_buffer *get_cfg_buf();
    static context *get_context();
    // STATUS_BUFFER push(const msg_buffer &msg);
    // STATUS_BUFFER pop(msg_buffer &msg);
    

private:
    static context *ctx_dev;
    // static struct config_buffer cfg_buf;


};



















