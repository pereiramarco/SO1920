#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <signal.h>

#define MAX 256
#define split " "

int tempo_inatividade,tempo_execucao,pid[MAX],ntarefa;

void tookTooLong(int s) {
    int k=getpid();
    printf("estou a procura de %d\n",k);
    for (int i=0;i<MAX;i++) {
        if (pid[i]==k) {
            printf("demorou demasiado id %d\n",k);
            //TO-DO save result
            kill(pid[i],SIGKILL);
            pid[i]=0;
            break;
        }
    }
}

void addToPIDList(int pidd) {
    int i;
    for (i=0;i<MAX;i++) {
        if (!pid[i])
            pid[i]=pidd;
        else break;
    }
    ntarefa++;
}

void doStuff(char * linha) {
    char * token;
    char * splitedinput[MAX];
    char *comando[MAX][MAX];
    int i,tam,r=0,t,j=0;
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
            puts("mudou");
        }
        else r=1;
    }
    else if (!strcmp(splitedinput[0],"executar") || !strcmp(splitedinput[0],"-e")) {
        write(1,"Escolheu executar uma tarefa\n",29);
        int c=0,p[2],k;
        if (tam>1) {
            if (!fork()) {
                signal(SIGALRM,tookTooLong);
                k=getpid();
                printf("iniciou em pid: %d\n",k);
                addToPIDList(k);
                for (i=0;splitedinput[1][i];i++) {
                    splitedinput[1][i]=splitedinput[1][i+1];
                }
                for (i=0;splitedinput[tam-1][i+1];i++);
                splitedinput[tam-1][i]='\0';
                for (i=1;i<tam;i++,c++) {
                    for (j=0;i<tam && strcmp(splitedinput[i],"|");i++,j++) {
                        comando[c][j]=splitedinput[i];
                    }
                    comando[c][j]=NULL;
                }
                pipe(p);
                if (tempo_execucao>0) {
                    puts("vou alarmar");
                    alarm(tempo_execucao);
                }
                if (!fork()) {
                    close(p[0]);
                    if (c-1>0) dup2(p[1],1);
                    close(p[1]);
                    execvp(comando[0][0],comando[0]);
                    puts("exit");
                    exit(0);
                }
                for (i=1;i<c-1;i++) {
                    dup2(p[0],0);
                    close(p[1]);
                    close(p[0]);
                    pipe(p);
                    if (!fork()) {
                        dup2(p[1],1);
                        close(p[1]);
                        close(p[0]);
                        execvp(comando[i][0],comando[i]);
                        puts("error");
                        exit(0);
                    }
                }
                if (c-1>0) {
                    dup2(p[0],0);
                    close(p[1]);
                    close(p[0]);
                    execvp(comando[c-1][0],comando[c-1]);
                    puts("falhou");
                    exit(0);
                }
                pause();
                exit(0);
            }
        }
        else r=1;
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
    ntarefa=1;
    tempo_inatividade=-1;
    tempo_execucao=-1;
    for (n=0;n<MAX;n++) {
        pid[n]=0;
    }
    mkfifo("fifo",0666);
    index = open("index",O_RDWR | O_APPEND | O_CREAT, 0666);
    log = open("log",O_RDWR | O_APPEND | O_CREAT, 0666);
    while (1) {
        fifo = open("fifo",O_RDONLY);
        while ((n=read(fifo,linha,MAX))) {
            doStuff(linha);
        }
        puts("line read");
    }
}