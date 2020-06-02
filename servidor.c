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
-> colocar int em string (posso usar sprintf?)(escrever int com write)
*/

//pid[MAX][MAX] matriz que contem no indice 0 o pid do pai, no indice 1 o numero da tarefa no indice 2 o numero de processos e nos restantes os pids dos filhos
int tempo_inatividade,tempo_execucao,pid[MAX][MAX],ntarefa;

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
    pid[i][pid[i][2]++]=p;
}

void tookTooLong(int s) {
    int k=getpid();
    int p=getIndexOfPID(k);
    puts("someone took a lot");
    if (p!=-1) {
        printf("demorou demasiado id %d\n",k);
        //TO-DO save result
        kill(-pid[p][0],SIGKILL);
        wait(NULL);
        pid[p][1]=0;
        pid[p][0]=0;
        pid[p][2]=3;
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
            if (!(k=fork())) {
                signal(SIGALRM,tookTooLong);
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
                    alarm(tempo_execucao);
                }
                if (!(i=fork())) {
                    close(p[0]);
                    if (c-1>0) dup2(p[1],1);
                    close(p[1]);
                    execvp(comando[0][0],comando[0]);
                    puts("exit");
                    exit(0);
                }
                if (c-1>0) waitpid(i,NULL,0);
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
                    waitpid(k,NULL,0);
                }
                if (c-1>0) {
                    dup2(p[0],0);
                    close(p[1]);
                    close(p[0]);
                    execvp(comando[c-1][0],comando[c-1]);
                    puts("falhou");
                    exit(0);
                }
                exit(0);
            }
            i=addToPIDList(k);
            printf("iniciou em pid: %d tarefa %d\n",k,i);
        }
        else r=1;
    }
    else if (!strcmp(splitedinput[0],"listar") || !strcmp(splitedinput[0],"-l")) {
        puts("listar");
        write(1,"######TASKS######\n",18);
        for (i=0;i<MAX;i++) {
            if (pid[i][0]) {
                //write(1,"Tarefa ",7); //how to put integer in string (can use sprintf??)
                //write(1," em execução!\n",14);
                printf("Tarefa %d em execução!\n",pid[i][1]);
            }
        }
    }
    else if (!strcmp(splitedinput[0],"terminar") || !strcmp(splitedinput[0],"-t")) {
        puts("termina");
        if (tam>1) {
            int p=getPIDOfTask(atoi(splitedinput[1]));
            if (p) {
                puts("vou matar");
            }
        }
        else r=1;
    }
    else if (!strcmp(splitedinput[0],"historico") || !strcmp(splitedinput[0],"-r")) {
        puts("historico");
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
        pid[n][2]=3;
        for (int j=3;j<MAX;j++)
            pid[n][j]=-1;
    }
    mkfifo("fifo",0666);
    index = open("index",O_RDWR | O_APPEND | O_CREAT, 0666);
    log = open("log",O_RDWR | O_APPEND | O_CREAT, 0666);
    while (1) {
        fifo = open("fifo",O_RDONLY);
        while ((n=read(fifo,linha,MAX))) {
            doStuff(linha);
        }
        puts("client closed");
    }
}