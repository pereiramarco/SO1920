#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>

#define MAX 256

void main(int argc, char* argv[]) {
    int fifo,n;
    char buff[MAX+1];
    fifo=open("fifo",O_WRONLY | O_APPEND | O_CREAT,0666);
    if (argc==1) {
        while (1) {
            write(1,"ARGUS$=|> ",10);
            if ((n=read(0,buff,MAX)))
                write(fifo,buff,n);
            else break;
        }
    }
    else {
        int tam=strlen(argv[2])+strlen(argv[1]);
        tam+=3;
        char s[tam];
        s[0]='\0';
        strcat(s,argv[1]);
        strcat(s," '\0");
        strcat(s,argv[2]);
        strcat(s,"'\0");
        write(fifo,s,tam);    
    }

}