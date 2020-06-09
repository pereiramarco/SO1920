#!/bin/bash


echo "Marco"
./argus -e 'ls | wc | ls | ./p1'
sleep 1
./argus -t 1
./argus -e 'ls | wc'
./argus -e 'ls > ali'
sleep 1
./argus -e 'wc < ali > aqui'
./argus -i 5
./argus -m 2
./argus -e './p1'
./argus -l
./argus -o 1
./argus -r 
./argus -h
