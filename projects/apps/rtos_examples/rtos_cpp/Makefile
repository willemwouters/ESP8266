TOOLCHAIN_BASE := ./bsp/xtensa-p
TOOLCHAIN_CUSTOM := true

MODULES_BASE := ../../

SDK_CLOSED := ./bsp/esp_rtos/
SDK_BASE := ../../modules/modules_rtos/
CUSTOM_LIBS := 
MODULES_USER := src/Thread src/Helpers src/Log src/UdpServer
LIBS :=  minic  gcc gcov hal pp phy net80211 freertos lwip wpa ssl main driver smartconfig

CFLAGS := -Wno-write-strings

include ../../Makefile
