
BUILD_BASE	= build
FW_BASE		= firmware

ESPTOOL		?= esptool.py
ESPPORT		?= /dev/ttyUSB1

FW_TOOL		?= /usr/bin/esptool

# select which tools to use as compiler, librarian and linker
CC		:= $(TOOLCHAIN_BASE)/bin/xtensa-lx106-elf-gcc
CPP		:= $(TOOLCHAIN_BASE)/bin/xtensa-lx106-elf-g++
AR		:= $(TOOLCHAIN_BASE)/bin/xtensa-lx106-elf-ar
LD		:= $(TOOLCHAIN_BASE)/bin/xtensa-lx106-elf-gcc



ifndef OPTIMIZE_FLAG
    OPTIMIZE_FLAG :=-O2
endif
# compiler flags using during compilation of source files
CFLAGS		:= $(CFLAGS) -c $(OPTIMIZE_FLAG) -ggdb -std=gnu99 -Wpointer-arith -Wundef  -Wl,-EL -fno-inline-functions -nostdlib  -fno-exceptions -mlongcalls -mtext-section-literals  -D__ets__ -DICACHE_FLASH

CPPFLAGS := -g $(OPTIMIZE_FLAG) -Wpointer-arith -Wundef -Wl,-EL -fno-inline-functions -nostdlib -mlongcalls -mtext-section-literals -fno-exceptions -fno-rtti  -D__ets__ -DICACHE_FLASH
