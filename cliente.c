#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>

#define MAX 256
#define SEND 2048

void main(int argc, char* argv[]) {
    int fifo,n,output;
    char buff[MAX+1],receive[SEND];
    output=open("userout",O_RDONLY | O_CREAT,0666);
    fifo=open("userin",O_WRONLY | O_CREAT,0666);
    if (argc==1) {
        while (1) {
            write(1,"ARGUS$=|> ",10);
            if ((n=read(0,buff,MAX))) {
                write(fifo,buff,n);
            }
            else  {
                write(1,"\n",1);
            }
            if ((n=read(output,receive,SEND))) {
                write(1,receive,n);
            }
        }
    }
    else {
        char s[MAX];
        s[0]='\0';
        strcat(s,argv[1]);
        if (argc==3) {
            strcat(s," ");
            if (!strcmp(argv[1],"-e")) strcat(s,"'");
            strcat(s,argv[2]);
            if (!strcmp(argv[1],"-e")) strcat(s,"'");
        }
        strcat(s,"\n");
        write(1,s,strlen(s));
        write(fifo,s,strlen(s));    
    }
    close(output);
    close(fifo);

}