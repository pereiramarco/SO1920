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
    if (argc==1) {
        while (1) {
            write(1,"ARGUS$=|> ",10);
            fifo=open("userin",O_WRONLY,0666);  
            if ((n=read(0,buff,MAX))) {
                if (!strcmp(buff,"quit\n")) break;
                write(fifo,buff,n);
                output=open("userout",O_RDONLY,0666);
                while ((n=read(output,receive,SEND))) {
                    write(1,receive,n);
                }
                close(output);
            }
            else  {
                write(1,"\n",1);
            }
            close(fifo);
        }
    }
    else {
        char s[MAX];
        s[0]='\0';
        if (!strcmp(argv[1],"-e")) {
            strcat(s,"executar");
        }
        else if (!strcmp(argv[1],"-i")) {
            strcat(s,"tempo-inactividade");
        }
        else if (!strcmp(argv[1],"-m")) {
            strcat(s,"tempo-execucao");
        }
        else if (!strcmp(argv[1],"-l")) {
            strcat(s,"listar");
        }
        else if (!strcmp(argv[1],"-t")) {
            strcat(s,"terminar");
        }
        else if (!strcmp(argv[1],"-r")) {
            strcat(s,"historico");
        }
        else if (!strcmp(argv[1],"-h")) {
            strcat(s,"ajuda");
        }
        else if (!strcmp(argv[1],"-o")) {
            strcat(s,"output");
        }
        else if (!strcmp(argv[1],"-c")) {
            strcat(s,"clean");
        }
        else if (!strcmp(argv[1],"-b")) {
            strcat(s,"backup");
        }
        else if (!strcmp(argv[1],"-f")) {
            strcat(s,"fill");
        }
        if (argc==3) {
            strcat(s," ");
            if (!strcmp(argv[1],"-e") || !strcmp(argv[1],"-b")) strcat(s,"'");
            strcat(s,argv[2]);
            if (!strcmp(argv[1],"-e") || !strcmp(argv[1],"-b")) strcat(s,"'");
        }
        strcat(s,"\n");
        fifo=open("userin",O_WRONLY,0666);  
        write(fifo,s,strlen(s)); 
        close(fifo); 
        output=open("userout",O_RDONLY,0666);
        while ((n=read(output,receive,SEND)))
            write(1,receive,n); 
        close(output); 
    }

}