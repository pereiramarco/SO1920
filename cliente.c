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
    if (argc==1) {
        while (1) {
            fifo=open("fifo",O_WRONLY | O_APPEND | O_CREAT,0666);
            write(1,"ARGUS$=|> ",10);
            if ((n=read(0,buff,MAX)))
                write(fifo,buff,n);
            else  {
                close(fifo);
                write(1,"\n",1);
            }
        }
    }
    else {
        fifo=open("fifo",O_WRONLY | O_APPEND | O_CREAT,0666);
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

}