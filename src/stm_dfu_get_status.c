#include <stdio.h>
#include <libusb.h>
#include "dfu.h"

int main() {
	struct libusb_device *stm_dev;
	struct libusb_device_handle *handle;
	int r = 0;
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
	r = stm_dfu_clr_status(handle);
	r = stm_dfu_get_status(handle, &status);
	unsigned int bwPollTimeout = (status.bwPollTimeout[0] |
			status.bwPollTimeout[1] << 8 |
			status.bwPollTimeout[2] << 16);
	printf("bStatus: %02x\n", status.bStatus);
	printf("bwPollTimeout: %lu\n", bwPollTimeout);
	printf("bState: %02x\n", status.bState);

	libusb_close(handle);
	libusb_exit(NULL);

	return 0;
}
