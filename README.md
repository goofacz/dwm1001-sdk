# DWM1001 SDK

## Configuration
### Clone SDK repository

Make sure to clone all submodules
 
    git clone https://github.com/goofacz/dwm1001-sdk.git --recurse-submodules

### Download nRF SDK
Download and extract [nRF SDK v15.2.0](https://www.nordicsemi.com/Software-and-Tools/Software/nRF5-SDK)  and extarct it to `dwm1001-sdk/nrfsdk` directory:

    unzip nRF5_SDK_15.2.0_9412b96.zip -d dwm1001-sdk/nrfsdk/

Fix compilation issues (nRF SDk v15.2.0 doesn't compile with C++17):

    cd dwm1001-sdk/nrfsdk/nRF5_SDK_15.2.0_9412b96
    patch -p 0 --binary < ../fix-c++-compile-errors.patch

### Install GCC compiler for ARM Cortex architecture
DWM1001 SDK requires GCC supporting C++17 (v8.0.0 or newer) and newlib. 

Configure nRF SDK's Makefile at `dwm1001-sdk/nrfsdk/nRF5_SDK_15.2.0_9412b96/components/toolchain/gcc/Makefile.posix`

    GNU_INSTALL_ROOT = /usr/bin/
    GNU_VERSION = 8.2.0
    GNU_PREFIX = arm-none-eabi

### Download decadriver from DecaWeave
Dodnload [DW1000 Application Programming Interface with STM32F10x Application examples v 2.04](https://www.decawave.com/software/). Extract and copy it to `dwm1001-sdk/deca_driver/`:

    unzip dw1000_api_rev2p04-1 -d dwm1001-sdk/deca_driver/

### Download nRF Command Line Tools
Download [nRF Command Line Tools](https://www.nordicsemi.com/Software-and-Tools/Development-Tools/nRF5-Command-Line-Tools) for Linux and extract it:

    tar -xf nRF-Command-Line-Tools_9_8_1_Linux-x86_64.tar -C dwm1001-sdk/nrf_cmd_tools/

Make sure to add `dwm1001-sdk/nrf_cmd_tools/nrfjprog` to `PATH`

### Download J-Link Software and Documentation Pack
Download [J-Link Software and Documentation Pack](https://www.segger.com/downloads/jlink/#J-LinkSoftwareAndDocumentationPack) for Linux and extract it:

    tar -xf JLink_Linux_V642e_x86_64.tgz -C dwm1001-sdk/jlink

Add `dwm1001-sdk/jlink/JLink_Linux_V642e_x86_64` to `LD_LIBRARY_PATH`. Otherwise flashing will fail.

## Examples
There are few examples showing how to use SDK features.

### Building and flashing
Just go to example directory and type `make` to build it. In order to flash board type `make flash`.

If there are many DWM1001 connected to PC it is possible flash specific board with command `make flash BOARD_ID=identifier` where `identifier` is board (debugger) ID (see what `nrfjprog -i` does).

### CLI
SDK provides simple and extensible CLI implementation. 
