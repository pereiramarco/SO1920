#include <stdio.h>
#include <unistd.h>

void main() {
    while (1) {
        sleep(2);
        write(1,"i slept 2 seconds\n",18);
    }
}