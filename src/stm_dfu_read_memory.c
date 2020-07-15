#include <stdio.h>
#include <stdlib.h>
#include <getopt.h>
#include <libusb.h>
#include "dfu.h"

int main(int argc, char *argv[]) {
	struct libusb_device *stm_dev;
	struct libusb_device_handle *handle;
	int r = 0;

	unsigned int address = 0x08000000;
	size_t total_len = 16;
	char opt = '\0';
	while((opt = getopt(argc, argv, "a:s:")) != -1) {
		switch(opt) {
		case 'a':
			sscanf(optarg, "%x", &address);
			break;
		case 's':
			sscanf(optarg, "%d", &total_len);
			if(total_len < 0) {
				total_len = 16;
			}
			break;
		default:
			fprintf(stderr, "Unrecognized argument\n");
			return -1;
		}
	}

	r = libusb_init(NULL);
	if(r < 0) {
		return r;
	}

	r = stm_find_device(&stm_dev);
	if(r < 0) {
		fprintf(stderr, "Cannot find a STM32 device\n");
		return r;
	}

	r = libusb_open(stm_dev, &handle);
	if(r < 0) {
		fprintf(stderr, "Failed to open the device\n");
		return r;
	}

	unsigned int current_address = address;
	struct stm_dfu_status status = {0, {0, 0, 0}, 0, 0};
	uint8_t buf[2048];
	size_t block_size = 2048;
	if(total_len < block_size) {
		block_size = total_len;
	}
	while(status.bState != 0x02) {
		stm_dfu_clr_status(handle);
		stm_dfu_get_status(handle, &status);
	}
	stm_dfu_set_address_pointer(handle, address);
	stm_dfu_get_status(handle, &status);

	while(current_address < address + total_len) {
		while(status.bState != 0x02) {
			stm_dfu_clr_status(handle);
			stm_dfu_get_status(handle, &status);
		}
		size_t block_num = ((current_address - address) / block_size) + 2;
		r = stm_dfu_read_memory(handle, buf, block_num, block_size);
		stm_dfu_get_status(handle, &status);
		for(int i = 0; i<block_size; i+=16) {
			printf("%08x: ", current_address);
			for(int j=0; j<16; j+=2) {
				printf("%02x%02x ", buf[i+j], buf[i+j+1]);
			}
			printf("\n");
			current_address += 16;
		}

	}

	libusb_close(handle);
	libusb_exit(NULL);

	return 0;
}
