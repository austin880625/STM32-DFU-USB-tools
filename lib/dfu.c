#include "dfu.h"

int stm_find_device(struct libusb_device **stm_dev) {
	libusb_device **devs;
	libusb_device *dev;
	ssize_t cnt;
	int i;
	*stm_dev = NULL;
	cnt = libusb_get_device_list(NULL, &devs);
	if(cnt < 0) {
		return -1;
	}

	while((dev = devs[i++]) != NULL) {
		struct libusb_device_descriptor desc;
		int r = libusb_get_device_descriptor(dev, &desc);
		if (r < 0) {
			fprintf(stderr, "failed to get device descriptor");
			break;
		}

		if(desc.idVendor == 0x0483 && desc.idProduct == 0xDF11) {
			*stm_dev = dev;
			break;
		}
	}

	libusb_free_device_list(devs, 1);
	if(*stm_dev == NULL) {
		return -1;
	}
	return 0;
}

int stm_dfu_get(struct libusb_device_handle *handle, uint8_t *data) {
	int r = libusb_control_transfer(handle, 0xA1, 0x02, 0, 0, data, 0, 1000);
	r = libusb_control_transfer(handle, 0xA1, 0x02, 0, 0, data, 4, 1000);
	if(r < 0) {
		fprintf(stderr, "failed to perform get with error %d\n", r);
		return -1;
	}

	return 0;
}

int stm_dfu_read_memory(struct libusb_device_handle *handle, uint8_t *data, size_t block_num, size_t len) {
	int r = libusb_control_transfer(handle, 0xA1, 0x02, block_num, 0, data, len, 1000);
	if(r < 0) {
		fprintf(stderr, "failed to perform read_memory with error %d\n", r);
		return -1;
	}

	return 0;
}

int stm_dfu_write_memory(struct libusb_device_handle *handle, uint8_t *data, size_t block_num, size_t len) {
	int r = libusb_control_transfer(handle, 0x21, 0x01, block_num, 0, data, len, 1000);
	if(r < 0) {
		fprintf(stderr, "failed to perform write_memory with error %d\n", r);
		return -1;
	}

	return 0;
}

int stm_dfu_set_address_pointer(struct libusb_device_handle *handle, unsigned int address) {
	uint8_t data[5] = {0x21,
		(uint8_t)(address),
		(uint8_t)(address >> 8),
		(uint8_t)(address >> 16),
		(uint8_t)(address >> 24)};
	int r = libusb_control_transfer(handle, 0xA1, 0x02, 0, 0, data, 0, 1000);
	r = libusb_control_transfer(handle, 0x21, 0x01, 0, 0, data, 5, 1000);
	if(r < 0) {
		fprintf(stderr, "failed to perform set_address_pointer with error %d\n", r);
		return -1;
	}
	
	return 0;
}

int stm_dfu_erase(struct libusb_device_handle *handle, int mass, unsigned int address) {
	uint8_t data[5] = {0x41,
		(uint8_t)(address),
		(uint8_t)(address >> 8),
		(uint8_t)(address >> 16),
		(uint8_t)(address >> 24)};
	int wLength = (mass ? 1 : 5);
	int r = libusb_control_transfer(handle, 0xA1, 0x02, 0, 0, data, 0, 1000);
	r = libusb_control_transfer(handle, 0x21, 0x01, 0, 0, data, wLength, 1000);
	if(r < 0) {
		fprintf(stderr, "failed to perform erase with error %d\n", r);
		return -1;
	}

	return 0;
}

int stm_dfu_get_status(struct libusb_device_handle *handle, struct stm_dfu_status *status) {
	// the value of bmRequest is 0x21 in STM32 DFU reference, and 0xA1 in USB DFU spec. 0xA1 works here
	int r = libusb_control_transfer(handle, 0xA1, 0x03, 0, 0, (uint8_t*)status, 6, 1000);
	if(r < 0) {
		fprintf(stderr, "failed to perform get_status with error %d\n", r);
		return -1;
	}

	return 0;
}

int stm_dfu_clr_status(struct libusb_device_handle *handle) {
	int r = libusb_control_transfer(handle, 0x21, 0x04, 0, 0, NULL, 0, 1000);
	if(r < 0) {
		fprintf(stderr, "failed to perform clr_status with error %d\n", r);
		return -1;
	}

	return 0;
}

int stm_dfu_abort(struct libusb_device_handle *handle) {
	uint8_t data[4] = {0};
	int r = libusb_control_transfer(handle, 0x21, 0x06, 0, 0, data, 0, 1000);
	if(r < 0) {
		fprintf(stderr, "failed to perform abort with error %d\n", r);
		return -1;
	}
	
	return 0;
}

