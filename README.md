# STM32-DFU-USB-tools

This project implements most funtions of the USB DFU protocol specified in STMicroelectronic AN3156 application note.
It can be used to program the flash memory on STM32 microcontrollers over USB without IDEs/JTAG/SWD/vendor-provided utilities.

For more complete implementation please see [DfuSe](http://www.emcu.it/DFU-DfuSe/DFU-DfuSe.html) or [dfu-util](http://dfu-util.sourceforge.net/)

## Tested Environment

This project is developed and tested on the following platform:

Host side:

* Fedora 32, libusb 1.0.23

Device side:

* STM32F407ZGT6

## Requirements

To build this project, you'll need:

* `gcc`
* GNU `make`
* `libusb`
* `pkg-config`

The embedded bootloader of the STM32 device needs to support USB, check this out in the reference manual of your STM32 device(Usually in "Boot configuration" section). The BOOT pins also need to be selected properly(boot from system memory).

## Build Steps

Simply,

```sh
> make
```

the built binaries are then located in folder `bin`.

## Commands

### `stm_dfu_get_status`

GETSTATUS command of the USB DFU protocol, which will print the current DFU state and command status.

### `stm_dfu_read_memory [-a <address>] [-s <size>]`

Read memory content at specified address and with given size.

options:

* `-a <address>`: address in hex(without "0x") to start reading, default 08000000.
* `-s <size>`: size to be read, default 16.

### `stm_dfu_erase_memory [-a <address of the page/sector>] [-m]`

Erase the content of the flash memory at the specified page/sector

options:
* `-a <address>`: address in hex(without "0x") of the page/sector, default 08000000.
* `-m`: add this argument for mass erase

### `stm_dfu_write_memory -f <file> [-a <address in hex>] [-p <page/sector size>]`

Write the binary file into the speciefied memory address. The page that the memory address points to should be erased first.

options:
* `-f <file>` (required): The binary file going to be written into the flash memory. Use properly linked and translated binary file, **NOT** in ELF/COFF format.
* `-a <address>`: address in hex(without "0x") of the page/sector, default 08000000.
* `-p <page/sector size>`: the size of the page/sector to write, can include units like `2k`, `16k`.


### `stm_dfu_leave`

Leave DFU mode. After executing this command, The device will reset and run application code pointed by reset handler in the vector table. The DFU commands are also not available before power reset.
