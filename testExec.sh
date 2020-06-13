#!/bin/bash
make cliente
sleep 0.5
./argus -c
echo -e "\n"
./argus -e 'ls | grep argus | wc | sleep 1000'
echo -e "\n"
sleep 1
./argus -t 1
echo -e "\n"
./argus -e 'ls | wc'
echo -e "\n"
./argus -e 'ls > ali'
echo -e "\n"
sleep 1
./argus -e 'wc < ali > aqui'
echo -e "\n"
./argus -l
echo -e "\n"
./argus -i 2
echo -e "\n"
./argus -e 'sleep 1 | sleep 1 | sleep 1 | echo "hey" | sleep 1000'
echo -e "\n"
./argus -m 1
echo -e "\n"
./argus -e 'sleep 1000'
echo -e "\n"
./argus -l
echo -e "\n"
./argus -o 1
echo -e "\n"
sleep 6
echo -e "\n"
./argus -o 5
echo -e "\n"
./argus -r
echo -e "\n" 
./argus -h
echo -e "\n"
./argus -b
echo -e "\n"
./argus -c
echo -e "\n"
./argus -r
echo -e "\n"
./argus -l
