#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <signal.h>

#define MAX 256
#define split " "


/*dúvidas
*/

//pid[MAX][3] matriz que contem no indice 0 o pid do pai, no indice 1 o numero da tarefa no restante o pid do filho vivo
int tempo_inatividade,tempo_execucao,pid[MAX][3],ntarefa,acabadas;
char history[MAX][MAX],comand[MAX][MAX];

int getPIDOfTask(int task) {
    for (int i=0;i<MAX;i++) {
        if (pid[i][1]==task) {
            int r=pid[i][0];
            return r;
        }
    }
}

int getIndexOfPID(int pidd) {
    for (int i=0;i<MAX;i++) {
        if (pid[i][0]==pidd)
            return i;
    }
    return -1;
}

void addProcess(int p,int pai) {
    int i=getIndexOfPID(pai);
    pid[i][2]=p;
}

void chld_died_handler() {
    int e,pai = wait(&e);
    int paiID=getIndexOfPID(pai);
    if (WIFEXITED(e)) e=WEXITSTATUS(e);
    switch (e) {
        case (0) :
            sprintf(history[acabadas++],"#%d, concluida: %s\n",pid[paiID][1],comand[pid[paiID][1]]);
            break;
        case (9):
            sprintf(history[acabadas++],"#%d, terminada: %s\n",pid[paiID][1],comand[pid[paiID][1]]);
            break;
        case (15):
            sprintf(history[acabadas++],"#%d, max execução: %s\n",pid[paiID][1],comand[pid[paiID][1]]);
            break;
        default:
        break;
    }
    pid[paiID][0]=0;
    pid[paiID][1]=0;
    pid[paiID][2]=0;
}

void terminate(int s) {
    int k=getpid();
    int p=getIndexOfPID(k);
    int i;
    int sig;
    if (s==SIGUSR1) 
        sig=SIGKILL;
    else 
        sig=SIGTERM;
    if (p!=-1) {
        //TO-DO save result
        if (pid[p][2]) {
            kill(pid[p][2],SIGKILL);
            wait(NULL);
        }
        kill(pid[p][0],sig);
    }
}

int addToPIDList(int pidd) {
    int i,r=-1;
    for (i=0;i<MAX;i++) {
        if (!pid[i][0]) {
            pid[i][0]=pidd;
            pid[i][1]=ntarefa;
            ntarefa+=1;
            r=pid[i][1];
            break;
        }
    }
    return r;
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
        int c=0,p[2],k,n;
        if (tam>1) {
            signal(SIGALRM,terminate);
            signal(SIGUSR1,terminate);
            for (i=0;splitedinput[1][i];i++)
                splitedinput[1][i]=splitedinput[1][i+1];
            for (i=0;splitedinput[tam-1][i+1];i++);
            splitedinput[tam-1][i]='\0';
            for (i=1;i<tam;i++,c++) {
                for (j=0;i<tam && strcmp(splitedinput[i],"|");i++,j++) {
                    comando[c][j]=splitedinput[i];
                }
                comando[c][j]=NULL;
            }
            if (!(k=fork())) {
                addToPIDList(getpid());
                signal(SIGCHLD,SIG_IGN);
                if (tempo_execucao>0) {
                    alarm(tempo_execucao);
                }   
                pipe(p);
                if (!(i=fork())) {
                    close(p[0]);
                    if (c-1>0) dup2(p[1],1);
                    close(p[1]);
                    execvp(comando[0][0],comando[0]);
                    puts("exit");
                    exit(0);
                }
                pid[getIndexOfPID(getpid())][2]=i;
                waitpid(i,NULL,0);
                for (i=1;i<c-1;i++) {
                    dup2(p[0],0);
                    close(p[1]);
                    close(p[0]);
                    pipe(p);
                    if (!(k=fork())) {
                        dup2(p[1],1);
                        close(p[1]);
                        close(p[0]);
                        execvp(comando[i][0],comando[i]);
                        puts("error");
                        exit(0);
                    }
                    pid[getIndexOfPID(getpid())][2]=k;
                    waitpid(k,NULL,0);
                }
                if (c-1>0) {
                    dup2(p[0],0);
                    close(p[1]);
                    close(p[0]);
                    execvp(comando[c-1][0],comando[c-1]);
                    puts("error");
                    exit(0);
                }
                exit(0);
            }
            i=addToPIDList(k);
            for (int j=1;j<=tam-1;j++) {
                strcat(comand[i],splitedinput[j]);
                strcat(comand[i]," ");
            }
            printf("iniciou em pid %d a tarefa %d de comando %s\n",k,i,comand[i]);
        }
        else r=1;
    }
    else if (!strcmp(splitedinput[0],"listar") || !strcmp(splitedinput[0],"-l")) {
        puts("listar");
        write(1,"######TASKS######\n",18);
        char c[MAX];
        for (i=0;i<MAX;i++) {
            if (pid[i][0]) {
                c[0]='\0';
                sprintf(c,"Tarefa %d em execução!\n",pid[i][1]);
                write(1,c,strlen(c));
            }
        }
    }
    else if (!strcmp(splitedinput[0],"terminar") || !strcmp(splitedinput[0],"-t")) {
        puts("termina");
        if (tam>1) {
            int p=getPIDOfTask(atoi(splitedinput[1]));
            kill(p,SIGUSR1);
        }
        else r=1;
    }
    else if (!strcmp(splitedinput[0],"historico") || !strcmp(splitedinput[0],"-r")) {
        puts("historico");
        for (i=0;i<acabadas;i++) {
            write(1,history[i],strlen(history[i]));
        }
    }
    else if (!strcmp(splitedinput[0],"ajuda") || !strcmp(splitedinput[0],"-h")) {
        puts("ajuda");
        printf("Executar task: -e ou executar\nMudar tempo inatividade: -i ou tempo-inatividade\nMudar tempo execução: -m ou tempo-execucao\nListar tarefas: -l ou listar\nTerminar tarefa: -t ou terminar\n");
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
        pid[n][0]=0;
        pid[n][1]=0;
        pid[n][2]=0;
        comand[n][0]='\0';
    }
    acabadas=0;
    mkfifo("fifo",0666);
    index = open("index",O_RDWR | O_APPEND | O_CREAT, 0666);
    log = open("log",O_RDWR | O_APPEND | O_CREAT, 0666);
    signal(SIGCHLD,chld_died_handler);
    while (1) {
        fifo = open("fifo",O_RDONLY);
        while ((n=read(fifo,linha,MAX))) {
            doStuff(linha);
        }
        puts("client closed");
    }
}