#pragma once

struct stream_cfg;



struct iio_device* context_tx(const char *addr_dev, struct stream_cfg &txcfg);
struct iio_device* context_rx(const char *addr_dev, struct stream_cfg &rxcfg);

void shutdown();

int device_create_buffer(
						int d, 
						struct iio_device &dev, 
						unsigned int samples_count, 
						bool cyclic);

int write_to_device_buffer(const void *data, int size);
int read_to_device_buffer(const void *data, int size);




