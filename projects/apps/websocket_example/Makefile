TOOLCHAIN_BASE := ./
MODULES_BASE := ../../

SDK_CLOSED := ./bsp/esp_1_4_0/
SDK_BASE := ../../modules/modules_1_4_0/
CUSTOM_LIBS := 
LIBS :=  c gcc gcov hal pp phy net80211 lwip wpa ssl main driver
OPTIMIZE_FLAG = -O2  ## ANY VALUE OVERWRITES -O2, ex: -c
CFLAGS := -DLWIP_OPEN_SRC



include ../../Makefile


src/html_data:
	$(ROOT_DIR)/../sdk/mkfs.py -s html/ -d src/html_data.c
    
html: src/html_data

