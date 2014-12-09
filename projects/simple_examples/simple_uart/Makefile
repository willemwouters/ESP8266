# Base directory for the compiler
TOOLCHAIN_BASE ?= bsp/xtensa-2
TOOLCHAIN_INCLUDE ?= $(TOOLCHAIN_BASE)/include
SDK_BASE ?= bsp/esp_0_9_3
SDK_INCLUDE ?= $(SDK_BASE)/include $(SDK_BASE)/include/json

BUILD_BASE	= build
FW_BASE		= firmware

ESPTOOL		?= esptool.py
ESPPORT		?= /dev/ttyACM0

FW_TOOL		?= /usr/bin/esptool
TARGET		= simple_uart
export PATH := $(TOOLCHAIN_BASE)/bin:$(PATH)

# which modules (subdirectories) of the project to include in compiling
MODULES		= src  src/tty

# all MODULES/include will be included
EXTRA_INCDIR	= \
		$(SDK_INCLUDE) \
		$(TOOLCHAIN_INCLUDE)

# libraries used in this project, mainly provided by the SDK
LIBS		= c gcc gcov hal pp phy net80211 lwip wpa ssl main

# compiler flags using during compilation of source files
CFLAGS		= -Os -ggdb -std=c99 -Werror -Wpointer-arith -Wundef -Wall -Wl,-EL -fno-inline-functions -nostdlib -mlongcalls -mtext-section-literals  -D__ets__ -DICACHE_FLASH

# linker flags used to generate the main object file
LDFLAGS		= -nostdlib -Wl,--no-check-sections -u call_user_start -Wl,-static

# linker script used for the above linkier step
LD_SCRIPT	= eagle.app.v6.ld

# various paths from the SDK used in this project
SDK_LIBDIR	= lib
TOOLCHAIN_LIBDIR	= lib
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


####
#### no user configurable options below here
####

SRC_DIR		:= $(MODULES)
BUILD_DIR	:= $(addprefix $(BUILD_BASE)/,$(MODULES))

SDK_LIBDIR	:= $(addprefix $(SDK_BASE)/,$(SDK_LIBDIR))
TOOLCHAIN_LIBDIR	:= $(addprefix $(TOOLCHAIN_BASE)/,$(TOOLCHAIN_LIBDIR))

SDK_INCDIR	:= $(addprefix -I,$(SDK_INCLUDE))

SRC		:= $(foreach sdir,$(SRC_DIR),$(wildcard $(sdir)/*.c))
OBJ		:= $(patsubst %.c,$(BUILD_BASE)/%.o,$(SRC))
LIBS		:= $(addprefix -l,$(LIBS))
APP_AR		:= $(addprefix $(BUILD_BASE)/,$(TARGET)_app.a)
TARGET_OUT	:= $(addprefix $(BUILD_BASE)/,$(TARGET).out)

LD_SCRIPT	:= $(addprefix -T$(SDK_BASE)/$(SDK_LDDIR)/,$(LD_SCRIPT))

INCDIR	:= $(addprefix -I,$(SRC_DIR))
EXTRA_INCDIR	:= $(addprefix -I,$(EXTRA_INCDIR))
MODULE_INCDIR	:= $(addsuffix /include,$(INCDIR))

FW_FILE_1	:= $(addprefix $(FW_BASE)/,$(FW_FILE_1).bin)
FW_FILE_2	:= $(addprefix $(FW_BASE)/,$(FW_FILE_2).bin)

Q :=
vecho := @true


vpath %.c $(SRC_DIR)

define compile-objects
$1/%.o: %.c
	$(vecho) "CC $$<"
	$(Q) $(CC) $(INCDIR) $(MODULE_INCDIR) $(EXTRA_INCDIR) $(SDK_INCDIR) $(CFLAGS)  -c $$< -o $$@
endef

.PHONY: all checkdirs clean

all: checkdirs $(TARGET_OUT) $(FW_FILE_1) $(FW_FILE_2)

$(FW_FILE_1): $(TARGET_OUT) firmware
	$(vecho) "FW $@"
	$(Q) $(FW_TOOL) -eo $(TARGET_OUT) $(FW_FILE_1_ARGS)

$(FW_FILE_2): $(TARGET_OUT) firmware
	$(vecho) "FW $@"
	$(Q) $(FW_TOOL) -eo $(TARGET_OUT) $(FW_FILE_2_ARGS)

$(TARGET_OUT): $(APP_AR)
	$(vecho) "LD $@"
	$(Q) $(LD) -L$(SDK_LIBDIR) -L$(TOOLCHAIN_LIBDIR) $(LD_SCRIPT) $(LDFLAGS) -Wl,--start-group $(LIBS) $(APP_AR) -Wl,--end-group -o $@

$(APP_AR): $(OBJ)
	$(vecho) "AR $@"
	$(Q) $(AR) cru $@ $^

checkdirs: $(BUILD_DIR) $(FW_BASE)

$(BUILD_DIR):
	$(Q) mkdir -p $@

firmware:
	$(Q) mkdir -p $@

flash: $(FW_FILE_1) $(FW_FILE_2)
	-$(ESPTOOL) --port $(ESPPORT) write_flash 0x00000 firmware/0x00000.bin
	sleep 3
	-$(ESPTOOL) --port $(ESPPORT) write_flash 0x40000 firmware/0x40000.bin

clean:
	$(Q) rm -f $(APP_AR)
	$(Q) rm -rf $(BUILD_BASE)	
	$(Q) rm -f $(TARGET_OUT)
	$(Q) rm -f $(FW_FILE_1)
	$(Q) rm -f $(FW_FILE_2)
	$(Q) rm -rf $(FW_BASE)

$(foreach bdir,$(BUILD_DIR),$(eval $(call compile-objects,$(bdir))))
