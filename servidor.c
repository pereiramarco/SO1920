#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>

#define MAX 256
#define split " "

int tempo_inatividade,tempo_execucao;

void doStuff(char * linha) {
    char * token;
    char * splitedinput[MAX];
    int i,tam,r=0,t;
    token=strtok(linha,"\n");
    token=strtok(token,split);
    for (i=0;i<MAX && token;i++) {
        splitedinput[i]=token;
        token=strtok(NULL,split);
    }
    tam=i;
    if (!strcmp(splitedinput[0],"tempo-inactividade") || !strcmp(splitedinput[0],"-i")) {
        puts("inatividade");
        if (tam==2 && (t=atoi(splitedinput[1]))) {
            tempo_inatividade=t;
        }
        else r=1;
    }
    else if (!strcmp(splitedinput[0],"tempo-execucao") || !strcmp(splitedinput[0],"-m")) {
        puts("maxtime");
        if (tam==2 && (t=atoi(splitedinput[1]))) {
            tempo_execucao=t;
        }
        else r=1;
    }
    else if (!strcmp(splitedinput[0],"executar") || !strcmp(splitedinput[0],"-e")) {
        puts("execucao");
    }
    else if (!strcmp(splitedinput[0],"listar") || !strcmp(splitedinput[0],"-l")) {
        puts("listar");
    }
    else if (!strcmp(splitedinput[0],"terminar") || !strcmp(splitedinput[0],"-t")) {
        puts("termina");
    }
    else if (!strcmp(splitedinput[0],"historico") || !strcmp(splitedinput[0],"-r")) {
        puts("historico");
    }
    else if (!strcmp(splitedinput[0],"ajuda") || !strcmp(splitedinput[0],"-h")) {
        puts("ajuda");
    }
    else if (!strcmp(splitedinput[0],"output") || !strcmp(splitedinput[0],"-o")) {
        puts("consultar output");
    }
    else r=1;
    if (r) {
        printf("INVALID INPUT\n");
    }

}

void main() {
    int n,index,log,fifo;
    char linha[256];
    tempo_inatividade=-1;
    tempo_execucao=-1;
    mkfifo("fifo",0666);
    index = open("index",O_RDWR | O_APPEND | O_CREAT, 0666);
    log = open("log",O_RDWR | O_APPEND | O_CREAT, 0666);
    while (1) {
        fifo = open("fifo",O_RDONLY);
        while ((n=read(fifo,linha,MAX))) {
            doStuff(linha);
            printf("inatividade:%d\n",tempo_inatividade);
            printf("execucao: %d\n",tempo_execucao);
        }
    }
}