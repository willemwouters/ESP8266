#!/bin/bash
arr1=( 10 1 48 49 50 51 52 )

myfunc() {
out=""
echo "Choose 1. for data     Choose 0. for text"
#read chose
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

else
read arr
	for value in ${arr1[*]}
	do
	out+="\x"
	out+=$(printf "%x" $value)
	done

fi


#echo "echo -ne $out | nc -q1 192.168.4.1 80"
echo -ne $out | nc -u -q1 192.168.4.1 8080
sleep 0.025
myfunc
}

myfunc
