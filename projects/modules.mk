# Base directory for the compiler
TOOLCHAIN_BASE :=$(TOOLCHAIN_BASE)gcc/xtensa-lx106-elf_02
SDK_CLOSED :=$(SDK_CLOSED)
SDK_BASE :=$(SDK_BASE)
TOOLCHAIN_INCLUDE ?= $(TOOLCHAIN_BASE)/include 

SDK_EXTRA_INCLUDE := $(addprefix $(SDK_BASE)/../, $(LIBS))
SDK_EXTRA_INCLUDE := $(addsuffix /include, $(SDK_EXTRA_INCLUDE))

SDK_INCLUDE ?= $(SDK_BASE)/include include $(SDK_EXTRA_INCLUDE)
export PATH := $(TOOLCHAIN_BASE)/bin:$(PATH)

BUILD_BASE	= build
FW_BASE		= firmware

ESPTOOL		?= esptool.py
ESPPORT		?= /dev/ttyACM0

FW_TOOL		?= /usr/bin/esptool
TARGET		= simple_uart
export PATH := $(TOOLCHAIN_BASE)/bin:$(PATH)

# which modules (subdirectories) of the project to include in compiling
MODULES		= src

# all MODULES/include will be included
EXTRA_INCDIR	= \
		$(SDK_INCLUDE) \
		$(TOOLCHAIN_INCLUDE)

# libraries used in this project, mainly provided by the SDK
LIBS		:= $(CUSTOM_LIBS) c gcc gcov hal pp phy $(LIBS) 

# compiler flags using during compilation of source files
CFLAGS		:= $(CFLAGS) -g -O2 -Wpointer-arith -Wundef -Werror -Wl,-EL -fno-inline-functions -nostdlib -mlongcalls -mtext-section-literals  -D__ets__ -DICACHE_FLASH

# linker flags used to generate the main object file
LDFLAGS		= -nostdlib -Wl,--no-check-sections -u call_user_start -Wl,-static

# linker script used for the above linkier step
LD_SCRIPT	= eagle.app.v6.ld

# various paths from the SDK used in this project
SDK_LIBDIR_ORI	= lib_build
TOOLCHAIN_LIBDIR= lib
SDK_LDDIR	= ld
# these are the names and options to generate them
FW_FILE_1	= 0x00000
FW_FILE_1_ARGS	= -bo $@ -bs .text -bs .data -bs .rodata -bc -ec
FW_FILE_2	= 0x40000
FW_FILE_2_ARGS	= -es .irom0.text $@ -ec

# select which tools to use as compiler, librarian and linker
CC		:= $(TOOLCHAIN_BASE)/bin/xtensa-lx106-elf-gcc
AR		:= $(TOOLCHAIN_BASE)/bin/xtensa-lx106-elf-ar
LD		:= $(TOOLCHAIN_BASE)/bin/xtensa-lx106-elf-gcc

