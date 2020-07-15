#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <getopt.h>
#include <libusb.h>
#include "dfu.h"

size_t parse_size_unit(char *size, size_t len) {
	size_t ret = 0;
	for(size_t i = 0; i<len; i++) {
		if('0' < size[i] && size[i] < '9') {
			ret *= 10;
			ret += size[i] - '0';
		} else {
			if(size[i] == 'k' || size[i] == 'K') {
				ret *= (1 << 10);
			} else if(size[i] == 'm' || size[i] == 'M') {
				ret *= (1 << 20);
			} else if(size[i] == 'g' || size[i] == 'G') {
				ret *= (1 << 30);
			}
			break;
		}
	}

	return ret;
}

int main(int argc, char *argv[]) {
	struct libusb_device *stm_dev;
	struct libusb_device_handle *handle;
	int r = 0;

	unsigned int address = 0x08000000;
	size_t page_size = 16 * (1 << 10);
	char *filename = NULL;
	FILE *fp = NULL;
	size_t total_len = 0;
	char opt = '\0';
	while((opt = getopt(argc, argv, "a:f:p:")) != -1) {
		size_t arg_len = 0;
		arg_len = strlen(optarg);
		switch(opt) {
		case 'a':
			sscanf(optarg, "%x", &address);
			break;
		case 'f':
			filename = malloc(arg_len * sizeof(char));
			sscanf(optarg, "%s", filename);
			break;
		case 'p':
			page_size = parse_size_unit(optarg, arg_len);
			break;
		default:
			fprintf(stderr, "Unrecognized argument\n");
			return -1;
		}
	}
	if(!filename) {
		fprintf(stderr, "Please specify a binary file\n");
		return -1;
	}

	fp = fopen(filename, "rb");
	if(!fp) {
		fprintf(stderr, "File not found\n");
		return -1;
	}

	fseek(fp, 0L, SEEK_END);
	total_len = ftell(fp);
	fseek(fp, 0L, SEEK_SET);

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
	unsigned int base_address = address;
	struct stm_dfu_status status = {0, {0, 0, 0}, 0, 0};
	size_t block_size = 2048;
	uint8_t buf[2048];
	while(status.bState != 0x02) {
		stm_dfu_clr_status(handle);
		stm_dfu_get_status(handle, &status);
	}
	stm_dfu_set_address_pointer(handle, address);
	stm_dfu_get_status(handle, &status);

	while(current_address < address + total_len) {
		fseek(fp, current_address - address, SEEK_SET);
		size_t count = fread(buf, 1, block_size, fp);
		for(size_t i = count; i<block_size; i++)buf[i] = 0xff;

		while(status.bState != 0x02) {
			stm_dfu_clr_status(handle);
			stm_dfu_get_status(handle, &status);
		}
		size_t block_num = ((current_address - base_address) / block_size) + 2;
		stm_dfu_write_memory(handle, buf, block_num, block_size);
		stm_dfu_get_status(handle, &status);

		current_address += block_size;
		if(current_address - base_address >= page_size) {
			base_address = current_address;
			while(status.bState != 0x02) {
				stm_dfu_clr_status(handle);
				stm_dfu_get_status(handle, &status);
			}
			stm_dfu_set_address_pointer(handle, base_address);
			stm_dfu_get_status(handle, &status);
		}
	}

	if(fp) {
		fclose(fp);
	}
	if(filename) {
		free(filename);
	}
	libusb_close(handle);
	libusb_exit(NULL);

	return 0;
}

