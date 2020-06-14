#!/bin/bash
make cliente
gcc -o p1 p1.c
gcc -o p2 p2.c
sleep 0.5
./argus -c #cleaning all data for fresh start
echo -e "\n"
./argus -r #shows history (should be empty)
echo -e "\n"
./argus -l #shows list (should be empty)
echo -e "\n"
./argus -e 'ls | sleep 1000' #task that takes a lot of time
echo -e "\n"
sleep 1
./argus -t 1 #terminates task that takes a lot
echo -e "\n"
./argus -e 'sleep 3 | ./p2' #task that writes and isnt first
echo -e "\n"
./argus -e 'cut -f7 -d: /etc/passwd | uniq | wc -l' #executes 2 piped task
echo -e "\n"
./argus -e 'ls > ali' #sends output to file
echo -e "\n"
sleep 1
./argus -e 'wc < ali > aqui' #reads and send to file
echo -e "\n"
./argus -l #lists running proceses (should be only task 2 running)
echo -e "\n"
./argus -i 3 #sets inactivity to 3
echo -e "\n"
sleep 1
./argus -e './p1' #runs program that prints every 2 seconds (wont end)
echo -e "\n"
./argus -e './p2' #runs program that prints every 4 seconds (will end without any prints)
echo -e "\n"
./argus -m 1 #execution time to 1
echo -e "\n"
sleep 1
./argus -e 'sleep 1000' #runs command that takes a lot
echo -e "\n"
./argus -l #lists tasks (should be only 2 and/or 6 and/or 7 running and/or 8 running)
echo -e "\n"
./argus -o 3 #shows output of task 3
sleep 5 #sleeps to make sure that tasks 7 and 8 end
echo -e "\n"
./argus -t 6 #terminates task 6 because it would never end
echo -e "\n"
sleep 1
./argus -o 6 #shows output of task 6 (all the prints she made until she was killed)
echo -e "\n"
./argus -r #shows history (all tasks should appear here except task 2)
echo -e "\n" 
./argus -h #shows help
echo -e "\n"
./argus -b #creates backup
echo -e "\n"
./argus -t 2 #ends task 2 that was writing every 4 seconds since the beginning
./argus -l
make clean #cleans all created files