
BUILD_BASE	= build
FW_BASE		= firmware

ESPTOOL		?= esptool.py
ESPPORT		?= /dev/ttyUSB0

FW_TOOL		?= /usr/bin/esptool

# select which tools to use as compiler, librarian and linker
CC		:= $(TOOLCHAIN_BASE)/bin/xtensa-lx106-elf-gcc
CPP		:= $(TOOLCHAIN_BASE)/bin/xtensa-lx106-elf-g++
AR		:= $(TOOLCHAIN_BASE)/bin/xtensa-lx106-elf-ar
LD		:= $(TOOLCHAIN_BASE)/bin/xtensa-lx106-elf-gcc