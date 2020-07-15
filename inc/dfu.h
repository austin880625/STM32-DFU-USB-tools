#ifndef DFU_H
#define DFU_H

#include <stdio.h>
#include <libusb.h>

struct __attribute__((__packed__)) stm_dfu_status {
	uint8_t bStatus;
	uint8_t bwPollTimeout[3];
	uint8_t bState;
	uint8_t iString;
};

int stm_find_device(struct libusb_device **stm_dev);
int stm_dfu_get(struct libusb_device_handle *handle, uint8_t *data);
int stm_dfu_read_memory(struct libusb_device_handle *handle, uint8_t *data, size_t block_num, size_t len);
int stm_dfu_write_memory(struct libusb_device_handle *handle, uint8_t *data, size_t block_num, size_t len);
int stm_dfu_set_address_pointer(struct libusb_device_handle *handle, unsigned int address);
int stm_dfu_erase(struct libusb_device_handle *handle, int mass, unsigned int address);
int stm_dfu_get_status(struct libusb_device_handle *handle, struct stm_dfu_status *status);
int stm_dfu_clr_status(struct libusb_device_handle *handle);
int stm_dfu_abort(struct libusb_device_handle *handle);

#endif
