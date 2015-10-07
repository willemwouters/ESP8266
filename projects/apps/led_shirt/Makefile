TOOLCHAIN_BASE := ./
MODULES_BASE := ../../

SDK_CLOSED := ./bsp/esp_1_4_0/
SDK_BASE := ../../modules/modules_1_4_0/
CUSTOM_LIBS := 
LIBS := c gcc gcov hal pp phy net80211 lwip wpa ssl main driver

CFLAGS := -Werror  -DLWIP_OPEN_SRC
TARGET		= led_shirt


ifeq	($(MAKECMDGOALS), small)
	CFLAGS = $(CFLAGS) -DREALROWS=7
endif

include ../../Makefile
