#!/bin/bash
arr1=( 0xFC 9 9 0x4c 0x65 0x64 0x41 0x63 0x63 0x65 0x73 0x73 0x74 0x65 0x73 0x74 0x31 0x32 0x33 0x34 0x35 )

myfunc() {
out=""
echo "Choose 1. for data     Choose 0. for text"
read chose
if [ -z "$chose" ];
then
chose="1"
fi
if [ $chose -eq "0" ];
then
	echo "Please enter text.."
	read arr
	out+="\x"
	out+=$(printf "%x" 0)
	for (( i=0; i<${#arr}; i++ )); do
  	  out+="\x"
	  out+=$(printf "%x" "'${arr:$i:1}")
	done
fi

if [ $chose -eq "2" ];
then
	out+="\x"
	out+=$(printf "%x" 255)
	out+=$(printf "%x" 255)
fi

if [ $chose -eq "1" ];
then
    read arr
	for value in ${arr1[*]}
	do
	out+="\x"
	out+=$(printf "%x" $value)
	done
fi

#echo "echo -ne $out | nc -q1 192.168.4.1 80"
echo -ne $out | nc -u -q1 224.0.0.1 8080
sleep 0.025
myfunc
}

myfunc
