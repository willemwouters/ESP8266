/usr/bin/esptool -eo eagle.app.v6.out -bo 0x00000.bin -bs .text -bs .data -bs .rodata -bc -ec
/usr/bin/esptool -eo eagle.app.v6.out -es .irom0.text 0x40000.bin -ec
sudo esptool.py --port /dev/ttyACM0 write_flash 0x00000 0x00000.bin
sudo esptool.py --port /dev/ttyACM0 write_flash 0x40000 0x40000.bin
