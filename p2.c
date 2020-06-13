#include <stdio.h>
#include <unistd.h>

void main() {
    while (1) {
        sleep(4);
        write(1,"i slept 4 seconds\n",18);
    }
}