TOOLCHAIN_BASE := ./
MODULES_BASE := ../../

SDK_CLOSED := ./bsp/esp_1_5_0/
SDK_BASE := ../../modules/modules_1_5_0/
CUSTOM_LIBS := 
LIBS := c gcc gcov hal pp phy net80211 lwip_p wpa ssl main driver crypto

CFLAGS := -Werror  -DLWIP_OPEN_SRC
TARGET		= led_shirt


ifeq	($(MAKECMDGOALS), small)
	CFLAGS += -DREALROWS=7 -DVERTICAL=1
endif

ifeq	($(MAKECMDGOALS), ver)
	CFLAGS += -DVERTICAL=1
endif


ver: all
	
include ../../Makefile
