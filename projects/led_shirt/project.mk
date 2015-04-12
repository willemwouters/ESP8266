#
# Copyright (c) 2015, Lambertus Gorter <l.gorter@gmail.com>
# All rights reserved.
#
# The project configuration
#

# What to build
BUILD_TYPE      ?= debug
PROJECT_MODULES := user
#SDK_MODULES     := lwip freertos udhcp core linker
#SDK_MODULES := $(addprefix sdk_rtos/,$(SDK_MODULES))
SDK_MODULES     := core linker lwip upgrade 
SDK_MODULES := $(addprefix sdk_9_4/,$(SDK_MODULES)) xtensa driver_lib tcp_utils
OTHER_MODULES   := 
INCDIR  += $(abspath modules/xtensa/include)

# Build types
ifeq ($(BUILD_TYPE),debug)
CFLAGS        := -Og -g -Wpointer-arith -Wundef -Werror 
LDFLAGS       := 
else ifeq ($(BUILD_TYPE),release)
CFLAGS        := -O2 -Wpointer-arith -Wundef -Werror
LDFLAGS       := 
else
$(error build type $(BUILD_TYPE) not supported)
endif

