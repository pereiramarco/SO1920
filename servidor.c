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


/*todo
--> cat not working
*/

//pid[MAX][3] matriz que contem no indice 0 o pid do pai, no indice 1 o numero da tarefa no 2 o pid do filho vivo no 3 o pid do filho que verfica inatividade e no 4 o numero de comandos nessa tarefa
int tempo_inatividade,tempo_execucao,pid[MAX][5],ntarefa,indextoSave,logtoSave,history,fifo,general;
char comand[MAX][MAX];

void saveData() {
    char s[MAX];
    general=open("files/data",O_WRONLY | O_CREAT | O_TRUNC,0666);
    sprintf(s,"%d %d %d\n",tempo_execucao,tempo_inatividade,ntarefa);
    write(general,s,strlen(s));
}

void reloadData() {
    char s[MAX],*numero;
    general=open("files/data",O_RDONLY,0666);
    if (general==-1) return;
    read(general,s,MAX);
    numero=strtok(s,split);
    for (int i=0;numero;i++) {
        switch (i) {
            case (0):
                tempo_execucao=atoi(numero);
            break;
            case (1):
                tempo_inatividade=atoi(numero);
            break;
            case (2):
                ntarefa=atoi(numero);
            break;
        }
        numero=strtok(NULL,split);
    }
}
/**
 * Função que retorna o pid de uma tarefa
 * @param task tarefa dada
 * @return pid da tarefa task
 */ 
int getPIDOfTask(int task) {
    for (int i=0;i<MAX;i++) {
        if (pid[i][1]==task) {
            int r=pid[i][0];
            return r;
        }
    }
}

/**
 * Função que retorna o indice de um pid
 * @param pidd pid dado
 * @return indice do tarefa com pid pidd na matriz global pid
 */ 
int getIndexOfPID(int pidd) {
    for (int i=0;i<MAX;i++) {
        if (pid[i][0]==pidd)
            return i;
    }
    return -1;
}

/**
 * Função que adiciona um processo de pid p à tarefa de pid pai
 * @param p pid do filho
 * @param pai pid da tarefa pai
 */ 
void addProcess(int p,int pai) {
    int i=getIndexOfPID(pai);
    pid[i][2]=p;
}

/**
 * Salvaguarda a data de uma função para o logs e index
 * @param p pipe descriptor 
 * @param t task of the pipe
 */
void saveToLogs(int * p,int t,int died) {
    char string[MAX],indexC[MAX];
    int n,i=0,startsAt,total=0,fileTemp,c=getPIDOfTask(t);
    printf("este eh o valor %d\n",pid[getIndexOfPID(c)][4]);
    indexC[0]='\0';
    close(p[1]);
    sprintf(indexC,"%d",c);
    sprintf(string,"%d\n",died);
    fileTemp=open(indexC,O_WRONLY | O_CREAT,0666);
    write(fileTemp,string,2);
    if (!died || pid[getIndexOfPID(c)][4]>1) {
        while((n=read(p[0],string,MAX))) {
            write(fileTemp,string,n);
            i++;
        }
    }
    if (!i)
        write(fileTemp,"No output\n",10);
}


/**
 * Função que é ativada quando uma tarefa morre, desta forma pode guardar no histórico a forma como morreu e salvar no ficheiro log e index os dados da mesma
 */
void chld_died_handler() {
    int e,pai = wait(&e),n,beg=0;
    int paiID=getIndexOfPID(pai),task=pid[paiID][1];
    char c[MAX],fileToRem[MAX];
    c[0]='\0';
    fileToRem[0]='\0';
    if (WIFEXITED(e)) e=WEXITSTATUS(e);
    sprintf(fileToRem,"%d",pai);
    int tempFile=open(fileToRem,O_RDONLY);
    if (tempFile!=-1) {
        read(tempFile,c,2);
        e=atoi(c);
        beg=lseek(logtoSave,0,SEEK_END);
        while((n=read(tempFile,c,MAX))) {
            write(logtoSave,c,n);
        }
        c[0]='\0';
        sprintf(c,"%d %d %d\n",pid[paiID][1],beg,lseek(logtoSave,0,SEEK_END));
        write(indextoSave,c,strlen(c));
    }
    close(tempFile);
    remove(fileToRem);
    c[0]='\0';
    switch (e) {
        case(0):
            sprintf(c,"#%d, concluida: %s\n",task,comand[task]);
        break;
        case (1):
            sprintf(c,"#%d, terminada: %s\n",task,comand[task]);
        break;
        case (2):
            sprintf(c,"#%d, max execução: %s\n",task,comand[task]);
        break;
        default:
            sprintf(c,"#%d, max inatividade: %s\n",task,comand[task]);
        break;
        
    }
    write(history,c,strlen(c));
    for (int j=0;j<5;j++)
        pid[paiID][j]=0;
    puts("ta feito");
}

/**
 * Função que termina tarefa com um código diferente para poder depois reconhecer a forma como morreu
 * @param s sinal que ativou esta função
 */
void terminate(int s) {
    puts("terminando");
    int k=getpid();
    int p=getIndexOfPID(k);
    int i,died,send[2];
    switch (s) {
    case (SIGUSR1): 
        died=1;
    break;
    case (SIGALRM):
        died=2;
    break;
    case (SIGUSR2):
        died=3;
    break;
    }
    send[0]=0;
    send[1]=1;
    kill(pid[p][2],SIGSTOP);
    saveToLogs(send,pid[p][1],died);
    if (p!=-1) {
        if (pid[p][2]) {
            kill(pid[p][2],SIGKILL);
            wait(NULL);
            if (pid[p][3]) {
                kill(pid[p][3],SIGKILL);
                wait(NULL);
            }
        }
        kill(pid[p][0],SIGKILL);
        for (int j=0;j<5;j++)
            pid[p][j]=0;
    }
}
/**
 * Função que atualiza a matriz pid com um processo
 * @param pidd pid da tarefa a adicionar à matriz pid
 * @return numero de tarefa associada ao processo adicionado
 */
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

/**
 * Função que envia o sinal responsavel por inatividade
 */
void sendsig() {
    kill(getppid(),SIGUSR2);
}

/**
 * Função que trata do parsing e de escolher o que irá fazer consoante o que receba
 * @param linha string onde está guardado o input do user
 */
void doStuff(char * linha) {
    char * token;
    char * splitedinput[MAX];
    char *comando[MAX][MAX];
    int i,tam,r=0,t,j=0,m;
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
        int c=0,p[2],pi[2],k,n,fF=-1,tF=-1;
        char *toFile,*fromFile,linha[MAX],caracter[MAX];
        toFile=NULL;
        fromFile=NULL;
        if (tam>1) {
            signal(SIGALRM,terminate);
            signal(SIGUSR1,terminate);
            signal(SIGUSR2,terminate);
            for (i=0;splitedinput[1][i];i++)
                splitedinput[1][i]=splitedinput[1][i+1];
            for (i=0;splitedinput[tam-1][i+1];i++);
            splitedinput[tam-1][i]='\0';
            for (i=1;i<tam;i++,c++) {
                for (j=0;i<tam && strcmp(splitedinput[i],"|");i++,j++) {
                    if (i>0 && !strcmp(splitedinput[i-1],">")) {
                        toFile=splitedinput[i];
                    }
                    else if (i>0 && !strcmp(splitedinput[i-1],"<")) {
                        fromFile=splitedinput[i];
                    }
                    else if (strcmp(splitedinput[i],"<") && strcmp(splitedinput[i],">")) 
                        comando[c][j]=splitedinput[i];
                    else j--;
                }
                comando[c][j]=NULL;
            }
            if (!(k=fork())) {
                addToPIDList(getpid());
                pid[getIndexOfPID(getpid())][4]=c;
                signal(SIGCHLD,SIG_IGN);
                if (tempo_execucao>0) {
                    alarm(tempo_execucao);
                }   
                if (fromFile) {
                    fF=open(fromFile,O_RDONLY);
                    if (fF!=-1) {
                        dup2(fF,0);
                        close(fF);
                    }
                    else {
                        puts("deu asneira");
                        return;
                    }
                }             
                else {
                    if (!strcmp(comando[0][0],"cat") && fF==-1) {
                        dup2(fifo,0);
                        close(fifo);
                        kill(getppid(),SIGSTOP);
                    }
                }
                if (toFile) {
                    tF=open(toFile,O_RDWR | O_CREAT | O_TRUNC,0666);
                }
                pipe(p);
                //lançamento do primeiro comando de uma tarefa
                if (!(i=fork())) {
                    close(p[0]);
                    if (c==1 && tF!=-1) {
                        dup2(tF,1);
                        close(tF);
                    }
                    else 
                        dup2(p[1],1);
                    close(p[1]);
                    execvp(comando[0][0],comando[0]);
                    puts("exit");
                    exit(0);
                }
                pid[getIndexOfPID(getpid())][2]=i;
                close(p[1]);
                //verificar tempo inatividade para a primeira tarefa
                pipe(pi);
                if (!(i=fork())) {
                        if (tempo_inatividade>0) {
                            signal(SIGALRM,sendsig);
                            alarm(tempo_inatividade);
                        }
                        while((n=read(p[0],caracter,MAX))) {
                            write(pi[1],caracter,n);
                            if (tempo_inatividade>0) alarm(tempo_inatividade);
                        }
                        exit(0);
                    }
                //
                pid[getIndexOfPID(getpid())][3]=k;
                waitpid(k,NULL,0);
                waitpid(i,NULL,0);
                // lançar a execução de comandos encadeados por |
                for (i=1;i<c;i++) {
                    dup2(pi[0],0);
                    close(pi[1]);
                    close(pi[0]);
                    pipe(p);
                    if (!(k=fork())) {
                        if (i==c-1 && tF!=-1) {
                            dup2(tF,1);
                            close(tF);
                        }
                        else {
                            dup2(p[1],1);
                        }
                        close(p[1]);
                        close(p[0]);
                        execvp(comando[i][0],comando[i]);
                        puts("error");
                        exit(0);
                    }
                    // 
                    pid[getIndexOfPID(getpid())][2]=k;
                    close(p[1]);
                    // verificar inatividade
                    pipe(pi);
                    if (!(m=fork())) {
                        if (tempo_inatividade>0) {
                            signal(SIGALRM,sendsig);
                            alarm(tempo_inatividade);
                        }
                        while((n=read(p[0],caracter,MAX))) {
                            write(pi[1],caracter,n);
                            if (tempo_inatividade>0) alarm(tempo_inatividade);
                        }
                        exit(0);
                    }
                    //wait for the transaction of the info
                    pid[getIndexOfPID(getpid())][3]=m;
                    waitpid(m,NULL,0);
                    waitpid(k,NULL,0);
                }
                if (tF!=-1) {
                    lseek(tF,0,SEEK_SET);
                    printf("tou a escrever no pid o seguinte:\n");
                    while ((n=read(tF,linha,MAX))) {
                            printf("%s\n",linha);
                            write(pi[1],linha,n);
                    }
                    close(tF);
                }
                kill(getppid(),SIGCONT);
                saveToLogs(pi,pid[getIndexOfPID(getpid())][1],0);
                exit(0);
            }
            i=addToPIDList(k);
            pid[getIndexOfPID(k)][4]=c;
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
            if (p>0)
                kill(p,SIGUSR1);
            else puts("tarefa não existe");
        }
        else r=1;
    }
    else if (!strcmp(splitedinput[0],"historico") || !strcmp(splitedinput[0],"-r")) {
        puts("historico");
        char c[MAX];
        int n;
        lseek(history,0,SEEK_SET);
        while((n=read(history,c,MAX)))
            write(1,c,n);
    }
    else if (!strcmp(splitedinput[0],"ajuda") || !strcmp(splitedinput[0],"-h")) {
        puts("ajuda");
        printf("Executar task: -e ou executar 'comando1 | comando2 | ...'\nMudar tempo inatividade: -i ou tempo-inatividade n(segundos)\nMudar tempo execução: -m ou tempo-execucao n(segundos)\nListar tarefas: -l ou listar\nTerminar tarefa: -t ou terminar n(numero da tarefa)\nVer histórico: -r ou historico\nGuardar backup da info atual: -b ou backup\nApagar data atualmente carregada: -c ou clean\nCarregar a data de um dos backups: -l ou load");
    }
    else if (!strcmp(splitedinput[0],"output") || !strcmp(splitedinput[0],"-o")) {
        puts("consultar output");
        char chari;
        char linha[MAX];
        char *character;
        int task=0,beg=0,end=0,n,maxend;
        if (tam>1) {
            int i = atoi(splitedinput[1]),k=MAX;
            maxend=lseek(indextoSave,0,SEEK_END);
            lseek(indextoSave,0,SEEK_SET);
            while (1) { 
                linha[0]='\0';
                while ((n=read(indextoSave,&chari,1))) {
                    if (chari!='\n') {
                        strcat(linha,&chari);
                    }
                    else 
                        break;
                }
                for (int j=0;linha[j];j++) {
                    if ((linha[j]>57 || linha[j]<48) && linha[j]!=' ') {
                        for (int k=j;linha[k];k++)
                            linha[k]=linha[k+1];
                    }
                }
                task = strtol (linha,&character,10);
                beg = strtol (character,&character,10);
                end = strtol (character,NULL,10)+1;
                if (task==i || lseek(indextoSave,0,SEEK_CUR)==maxend)
                    break;
            }
            if (task!=i) {
                puts("non existent");
                return;
            }
            lseek(logtoSave,beg,SEEK_SET);
            while ((n=read(logtoSave,linha,1)) && end!=lseek(logtoSave,0,SEEK_CUR)) {
                write(1,linha,n);
            }
        }
        else r=1;
    }
    else if (!strcmp(splitedinput[0],"-b") || !strcmp(splitedinput[0],"backup")) {
        int p[2],i,fF,fT,n;
        char nome[MAX],spinner[MAX],linha[MAX];
        pipe(p);
        if (!(i=fork())) {
            dup2(p[1],1);
            close(p[1]);
            close(p[0]);
            execlp("date","date",NULL);
            _exit(0);
        }
        close(p[1]);
        waitpid(i,NULL,0);
        read(p[0],nome,MAX);
        strcpy(spinner,"backup/");
        strcat(spinner,nome);
        if (!(i=fork())) {
            execlp("mkdir","mkdir",spinner,NULL);
            _exit(0);
        }
        waitpid(i,NULL,0);
        for (i=0;i<4;i++) {
            spinner[0]='\0';
            strcpy(spinner,"backup/");
            strcat(spinner,nome);
            switch (i)
            {
            case 0:
                fF=open("files/data",O_RDONLY);
                strcat(spinner,"/data");
                break;
            case 1:
                fF=open("files/history",O_RDONLY);
                strcat(spinner,"/history");
                break;
            case 2:
                fF=open("files/index",O_RDONLY);
                strcat(spinner,"/index");
                break;
            default:
                fF=open("files/log",O_RDONLY);
                strcat(spinner,"/log");
                break;
            }
            fT=open(spinner,O_WRONLY | O_CREAT | O_TRUNC,0666);
            while ((n=read(fF,linha,MAX-1))) {
                write(fT,linha,n);
            }
            close(fF);
            close(fT);
        }
    }
    else if (!strcmp(splitedinput[0],"-c") || !strcmp(splitedinput[0],"clean")) {
        indextoSave=open("files/index",O_RDWR | O_TRUNC | O_APPEND);
        history=open("files/history",O_RDWR | O_TRUNC | O_APPEND);
        logtoSave=open("files/log",O_RDWR | O_TRUNC | O_APPEND);
        tempo_inatividade=-1;
        tempo_execucao=-1;
        ntarefa=1;
    }
    else r=1;
    if (r) {
        printf("INVALID INPUT\n");
    }
}

void main() {
    int n;
    char linha[256];
    ntarefa=1;
    tempo_inatividade=-1;
    tempo_execucao=-1;
    for (n=0;n<MAX;n++) {
        for (int j=0;j<5;j++)
            pid[n][j]=0;
        comand[n][0]='\0';
    }
    mkfifo("fifo",0666);
    indextoSave = open("files/index",O_RDWR | O_APPEND  | O_CREAT, 0666);
    logtoSave = open("files/log",O_RDWR | O_APPEND | O_CREAT, 0666);
    history=open("files/history",O_RDWR | O_APPEND | O_CREAT,0666);
    reloadData();
    signal(SIGCHLD,chld_died_handler);
    while (1) {
        fifo = open("fifo",O_RDONLY);
        while ((n=read(fifo,linha,MAX))) {
            doStuff(linha);
            saveData();
        }
        puts("client closed");
    }
}