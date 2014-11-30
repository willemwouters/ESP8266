Notice: AT demo v0.19 is based on SDK v0.9.2.

(1)Not support updata through wifi:
compile:
./gen_misc.sh 

download:
blank.bin   0x7E000
eagle.app.v6.flash.bin       0x00000
eagle.app.v6.irom0text.bin   0x40000

(2)Support update through wifi (since AT_v0.18):
compile: 
./gen_misc_plus.sh 1

download:
blank.bin  0x7E000
boot.bin   0x00000
user1.bin  0x01000

Update steps
1.Make sure TE(terminal equipment) is in sta or sta+ap mode
ex. AT+CWMODE=3
    OK
    
    AT+RST

2.Make sure TE got ip address
ex. AT+CWJAP="ssid","12345678"
    OK

    AT+CIFSR
    192.168.1.134

3.Let's update
ex. AT+CIUPDATE
    +CIPUPDATE:1    found server
    +CIPUPDATE:2    connect server
    +CIPUPDATE:3    got edition
    +CIPUPDATE:4    start start

    OK

note. If there are mistakes in the updating, then break update and print ERROR.