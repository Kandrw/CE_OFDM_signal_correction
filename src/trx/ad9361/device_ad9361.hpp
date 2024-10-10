#pragma once

struct stream_cfg;

struct config_device;


void shutdown();

int init_device_TRX(config_device &cfg);

int write_to_device_buffer(const void *data, int size);
int read_to_device_buffer(const void *data, int size);




