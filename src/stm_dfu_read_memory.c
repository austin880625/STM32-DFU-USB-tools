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
	uint8_t buf[16];
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
		size_t block_num = ((current_address - address) / 16) + 2;
		stm_dfu_read_memory(handle, buf, block_num, 16);
		stm_dfu_get_status(handle, &status);
		printf("%08x:\t", current_address);
		for(int i=0; i<16; i+=2) {
			printf("%02x%02x ", buf[i], buf[i+1]);
		}
		printf("\n");

		current_address += 16;
	}

	libusb_close(handle);
	libusb_exit(NULL);

	return 0;
}
