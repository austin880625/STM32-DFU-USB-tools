#include <stdio.h>
#include <stdlib.h>
#include <getopt.h>
#include <libusb.h>
#include "dfu.h"

int main(int argc, char *argv[]) {
	struct libusb_device *stm_dev;
	struct libusb_device_handle *handle;
	int r = 0;

	char opt = '\0';

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

	struct stm_dfu_status status = {0, {0, 0, 0}, 0, 0};
	while(status.bState != 0x02) {
		stm_dfu_clr_status(handle);
		stm_dfu_get_status(handle, &status);
	}
	stm_dfu_set_address_pointer(handle, 0x08000000);
	stm_dfu_get_status(handle, &status);

	while(status.bState != 0x02) {
		stm_dfu_clr_status(handle);
		stm_dfu_get_status(handle, &status);
	}
	stm_dfu_leave(handle);
	stm_dfu_get_status(handle, &status);

	libusb_close(handle);
	libusb_exit(NULL);

	return 0;
}
