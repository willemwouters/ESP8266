# Base directory for the compiler
ifndef TOOLCHAIN_CUSTOM
    TOOLCHAIN_BASE :=$(TOOLCHAIN_BASE)/bsp/xtensa-2/
endif

SDK_CLOSED :=$(SDK_CLOSED)
SDK_BASE :=$(SDK_BASE)
TOOLCHAIN_INCLUDE ?= $(TOOLCHAIN_BASE)/include 

SDK_EXTRA_INCLUDE := $(addprefix $(SDK_BASE)/../, $(LIBS))
SDK_EXTRA_INCLUDE := $(addsuffix /include, $(SDK_EXTRA_INCLUDE))

SDK_INCLUDE ?= $(SDK_BASE)/include include $(SDK_EXTRA_INCLUDE)
export PATH := $(TOOLCHAIN_BASE)/bin:$(PATH)

ROOT_DIR:=$(shell dirname $(realpath $(lastword $(MAKEFILE_LIST))))

include $(ROOT_DIR)/settings.mk


ifndef TARGET
    TARGET := out
endif


export PATH := $(TOOLCHAIN_BASE)/bin:$(PATH)

# which modules (subdirectories) of the project to include in compiling
MODULES		= $(MODULES_USER) src

# all MODULES/include will be included
EXTRA_INCDIR	= \
		$(SDK_INCLUDE) \
		$(TOOLCHAIN_INCLUDE)


# libraries used in this project, mainly provided by the SDK
LIBS		:= $(CUSTOM_LIBS)  $(LIBS) 



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


####
#### no user configurable options below here
####

SRC_DIR		:= $(MODULES)
BUILD_DIR	:= $(addprefix $(BUILD_BASE)/,$(MODULES))

SDK_LIBDIR		:= $(addprefix $(SDK_BASE)/, $(SDK_LIBDIR_ORI))
SDK_LIBDIR_EXTRA	:= $(addprefix $(SDK_BASE)/../, $(CUSTOM_LIBS) )
SDK_LIBDIR_EXTRA_L	:=  $(addsuffix /$(SDK_LIBDIR_ORI), -L$(SDK_LIBDIR_EXTRA))


SDK_CLOSED_INCDIR := $(SDK_CLOSED)/include
SDK_CLOSED_LIBDIR := $(SDK_CLOSED)/lib
TOOLCHAIN_LIBDIR	:= $(addprefix $(TOOLCHAIN_BASE)/,$(TOOLCHAIN_LIBDIR)) -L$(TOOLCHAIN_BASE)/lib/gcc/


SDK_CLOSED_INCDIR	:= $(addprefix -I,$(SDK_CLOSED_INCDIR)) 
SDK_INCDIR	:= $(addprefix -I,$(SDK_INCLUDE))

SRC		:= $(foreach sdir,$(SRC_DIR),$(wildcard $(sdir)/*.c))
SRCPP   := $(foreach sdir,$(SRC_DIR),$(wildcard $(sdir)/*.cpp))

OBJ		:= $(patsubst %.c,$(BUILD_BASE)/%.o,$(SRC))
OBJPP	:= $(patsubst %.cpp,$(BUILD_BASE)/%.o,$(SRCPP))

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
vecho := @echo


vpath %.c $(SRC_DIR)
vpath %.cpp $(SRC_DIR)

define compile-objects
$1/%.o: %.cpp
	$(vecho) "CC $$<"
	$(Q) $(CPP) $(INCDIR) $(MODULE_INCDIR) $(EXTRA_INCDIR) $(SDK_INCDIR) $(SDK_CLOSED_INCDIR) $(CPPFLAGS)  -c $$< -o $$@

$1/%.o: %.c
	$(vecho) "CC $$<"
	$(Q) $(CC) $(INCDIR) $(MODULE_INCDIR) $(EXTRA_INCDIR) $(SDK_INCDIR) $(SDK_CLOSED_INCDIR) $(CFLAGS)  -c $$< -o $$@
endef

.PHONY: all checkdirs clean

small: all

all: checkdirs $(TARGET_OUT) $(FW_FILE_1) $(FW_FILE_2)


$(FW_FILE_1): $(TARGET_OUT) firmware
	$(vecho) "FW $@"
	$(Q) $(FW_TOOL) -eo $(TARGET_OUT) $(FW_FILE_1_ARGS)

$(FW_FILE_2): $(TARGET_OUT) firmware
	$(vecho) "FW $@"
	$(Q) $(FW_TOOL) -eo $(TARGET_OUT) $(FW_FILE_2_ARGS)

$(TARGET_OUT): $(APP_AR)
	$(vecho) "LD $@"
	$(Q) $(LD) -L$(SDK_LIBDIR) $(SDK_LIBDIR_EXTRA_L) -L$(TOOLCHAIN_LIBDIR) -L$(SDK_CLOSED_LIBDIR) $(LD_SCRIPT) $(LDFLAGS) -Wl,--start-group $(LIBS) $(APP_AR) -Wl,--end-group -o $@

$(APP_AR): $(OBJPP) $(OBJ)
	$(vecho) "AR $@"
	$(Q) $(AR) cru $@ $^

checkdirs: $(BUILD_DIR) $(FW_BASE)
	@echo Compiling submodules
	cd $(MODULES_BASE)/modules && make
	
$(BUILD_DIR):
	$(Q) mkdir -p $@

firmware:
	$(Q) mkdir -p $@

flash: $(FW_FILE_1) $(FW_FILE_2)
	-$(ESPTOOL) --port $(ESPPORT) write_flash 0x7C000 $(SDK_CLOSED)/bin/esp_init_data_default.bin  0x7E000 $(SDK_CLOSED)/bin/blank.bin 0x00000 firmware/0x00000.bin 0x40000 firmware/0x40000.bin

clean:
	$(Q) rm -f $(APP_AR)
	$(Q) rm -rf $(BUILD_BASE)	
	$(Q) rm -f $(TARGET_OUT)
	$(Q) rm -f $(FW_FILE_1)
	$(Q) rm -f $(FW_FILE_2)
	$(Q) rm -rf $(FW_BASE)

clean_modules: clean
	cd ../../modules && make clean

$(foreach bdir,$(BUILD_DIR),$(eval $(call compile-objects,$(bdir))))
