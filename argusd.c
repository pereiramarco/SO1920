#include "argus.h"

#define MAX 256
#define SEND 512
#define split " "

//pid[MAX][MAX] matriz que contem no indice 0 o pid do pai, no indice 1 o numero da tarefa no 2 o pid do numero de filhos no 3 numero de comandos nessa tarefa e nos restantes o pid dos filhos que se encontram a executar
int tempo_inatividade,tempo_execucao,pid[MAX][MAX],ntarefa,indextoSave,logtoSave,history,fifo,general,output;
char comand[MAX][MAX];

/**
 * Salva informação para o servidor ler caso vá abaixo
 */
void saveData() {
    char s[MAX];
    general=open("files/data",O_WRONLY | O_CREAT | O_TRUNC,0666);
    sprintf(s,"%d %d %d\n",tempo_execucao,tempo_inatividade,ntarefa);
    write(general,s,strlen(s));
    close(general);
}

/**
 * Carrega a informação escrita nos ficheiros
 */
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
    close(general);
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
    pid[i][4+pid[i][2]++]=p;
}

/**
 * Salvaguarda a data de uma função para o logs e index
 * @param p pipe descriptor 
 * @param t task of the pipe
 */
void saveToLogs(int * p,int t,int died) {
    char string[MAX],indexC[MAX];
    int n,startsAt,total=0,fileTemp,c=getPIDOfTask(t);
    indexC[0]='\0';
    sprintf(indexC,"%d",c);
    sprintf(string,"%d\n",died);
    fileTemp=open(indexC,O_WRONLY | O_CREAT,0666);
    write(fileTemp,string,2);
    if (!died) {
        close(p[1]);
        while((n=read(p[0],string,MAX))) {
            write(fileTemp,string,n);
        }
    }
    close(fileTemp);
}


/**
 * Função que é ativada quando uma tarefa morre, desta forma pode guardar no histórico a forma como morreu e salvar no ficheiro log e index os dados da mesma
 */
void chld_died_handler() {
    int e,pai = wait(&e),n,beg=0,i=0;
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
            i++;
        }
        if ((i==0) || (i==1 && n == 2)) {
            write(logtoSave,"No output\n",10);
        }
        c[0]='\0';
        sprintf(c,"%d %d %d\n",pid[paiID][1],beg,lseek(logtoSave,0,SEEK_END));
        write(indextoSave,c,strlen(c));
        close(tempFile);
    }
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
        case (3):
            sprintf(c,"#%d, max inatividade: %s\n",task,comand[task]);
        break;
        default :
        break;
        
    }
    write(history,c,strlen(c));
    for (int j=0;j<MAX;j++) {
        pid[paiID][j]=0;
    }
    comand[paiID][0]='\0';
}

/**
 * Função que termina tarefa com um código diferente para poder depois reconhecer a forma como morreu
 * @param s sinal que ativou esta função
 */
void terminate(int s) {
    int j;
    int k=getpid();
    int p=getIndexOfPID(k);
    int i,died;
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
    if (p!=-1) {
        for (j=4;j<3+pid[p][2];j++)
            kill(pid[p][j],SIGSTOP);
        saveToLogs(NULL,pid[p][1],died);
        for (j=4;j<4+pid[p][2];j++) {
            if (pid[p][j]) {
                kill(pid[p][j],SIGKILL);
                pid[p][j]=0;
            }
        }
        for (i=0;i<4;i++) {
            pid[p][i]=0;
        }
        kill(k,SIGKILL);
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
    char send[SEND];
    send[0]='\0';
    int i,tam,r=0,t,j=0,m;
    token=strtok(linha,"\n");
    token=strtok(token,split);
    for (i=0;i<MAX && token;i++) {
        splitedinput[i]=token;
        token=strtok(NULL,split);
    }
    tam=i;
    if (!strcmp(splitedinput[0],"tempo-inactividade")) {
        if (tam==2 && (t=atoi(splitedinput[1]))) {
            tempo_inatividade=t;
            sprintf(send,"Novo tempo de inatividade: %d\n",tempo_inatividade);
            write(output,send,strlen(send));
        }
        else r=1;
    }
    else if (!strcmp(splitedinput[0],"tempo-execucao")) {
        if (tam==2 && (t=atoi(splitedinput[1]))) {
            tempo_execucao=t;
            sprintf(send,"Novo tempo de execução: %d\n",tempo_execucao);
            write(output,send,strlen(send));
        }
        else r=1;
    }
    else if (!strcmp(splitedinput[0],"executar")) {
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
                        j--;
                    }
                    else if (i>0 && !strcmp(splitedinput[i-1],"<")) {
                        fromFile=splitedinput[i];
                        j--;
                    }
                    else if (strcmp(splitedinput[i],"<") && strcmp(splitedinput[i],">")) 
                        comando[c][j]=splitedinput[i];
                    else j--;
                }
                comando[c][j]=NULL;
            }
            if (!(k=fork())) {
                addToPIDList(getpid());
                pid[getIndexOfPID(getpid())][3]=c;
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
                        write(output,"Erro ao abrir o ficheiro destino\n",33);
                        return;
                    }
                }
                close(output);  // closing output 
                if (toFile) {
                    tF=open(toFile,O_WRONLY | O_CREAT | O_TRUNC,0666);
                }
                pipe(p);
                int papa;
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
                    write(1,"Erro ao correr comando\n",23);
                    exit(0);
                }
                papa=getpid();
                addProcess(i,papa);
                close(p[1]);
                //verificar tempo inatividade para a primeira tarefa
                pipe(pi);
                if (!(i=fork())) {
                    dup2(pi[1],1);
                    close(pi[0]);
                    close(pi[1]);
                    if (tempo_inatividade>0) {
                        signal(SIGALRM,sendsig);
                        alarm(tempo_inatividade);
                    }
                    while((n=read(p[0],caracter,MAX))) {
                        write(1,caracter,n);
                        if (tempo_inatividade>0) alarm(tempo_inatividade);
                    }
                    exit(0);
                }
                addProcess(i,papa); 
                // lançar a execução de comandos encadeados por |
                for (i=1;i<c-1;i++) {
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
                        write(1,"Erro ao correr comando\n",23);
                        exit(0);
                    }
                    addProcess(k,getpid());
                    close(p[1]);
                    // verificar inatividade
                    pipe(pi);
                    if (!(m=fork())) {
                        dup2(pi[1],1);
                        close(pi[0]);
                        close(pi[1]);
                        if (tempo_inatividade>0) {
                            signal(SIGALRM,sendsig);
                            alarm(tempo_inatividade);
                        }
                        while((n=read(p[0],caracter,MAX))) {
                            write(1,caracter,n);
                            if (tempo_inatividade>0) alarm(tempo_inatividade);
                        }
                        exit(0);
                    }
                    addProcess(m,getpid());
                }
                if (c-1>0) {
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
                        write(1,"Erro ao correr comando\n",23);
                        exit(0);
                    }
                    addProcess(k,getpid());
                    close(p[1]);
                    // verificar inatividade
                    pipe(pi);
                    if (!(m=fork())) {
                        dup2(pi[1],1);
                        close(pi[0]);
                        close(pi[1]);
                        if (tempo_inatividade>0) {
                            signal(SIGALRM,sendsig);
                            alarm(tempo_inatividade);
                        }
                        while((n=read(p[0],caracter,MAX))) {
                            write(1,caracter,n);
                            if (tempo_inatividade>0) alarm(tempo_inatividade);
                        }
                        exit(0);
                    }
                    addProcess(m,getpid());
                }
                saveToLogs(pi,pid[getIndexOfPID(getpid())][1],0);
                exit(0);
            }
            i=addToPIDList(k);
            pid[getIndexOfPID(k)][3]=c;
            for (int j=1;j<=tam-1;j++) {
                strcat(comand[i],splitedinput[j]);
                strcat(comand[i]," ");
            }
            sprintf(linha,"Iniciou a tarefa #%d de comando %s\n",i,comand[i]); 
            write(output,linha,strlen(linha));
        }
        else r=1;
    }
    else if (!strcmp(splitedinput[0],"listar")) {
        char c[MAX];
        int j=0;
        for (i=0;i<MAX;i++) {
            if (pid[i][0]>0) {
                if (i==0) write(output,"######TASKS######\n",18);
                c[0]='\0';
                sprintf(c,"Tarefa %d em execução!\n",pid[i][1]);
                write(output,c,strlen(c));
                j++;
            }
        }
        if (!j) write(output,"Nada a apresentar\n",18);
    }
    else if (!strcmp(splitedinput[0],"terminar")) {
        if (tam>1) {
            int p=getPIDOfTask(atoi(splitedinput[1]));
            if (p>0) {
                kill(p,SIGUSR1);
                write(output,"Tarefa terminada\n",17);
            }
            else write(output,"Tarefa não existe ou já terminou\n",35);
        }
        else r=1;
    }
    else if (!strcmp(splitedinput[0],"historico")) {
        char c[MAX];
        int i=0;
        int n;
        lseek(history,0,SEEK_SET);
        while((n=read(history,c,MAX-2))) {
            if (i==0) write(output,"#####HISTORY#####\n",18);
            write(output,c,n);
            i++;
        }
        if (!i) write(output,"Nada a apresentar\n",18);
    }
    else if (!strcmp(splitedinput[0],"ajuda")) {
        write(output,"Executar task: -e ou executar 'comando1 | comando2 | ...'\nMudar tempo inatividade: -i ou tempo-inatividade n(segundos)\nMudar tempo execução: -m ou tempo-execucao n(segundos)\nListar tarefas: -l ou listar\nTerminar tarefa: -t ou terminar n(numero da tarefa)\nVer histórico: -r ou historico\nVer o output de uma tarefa: -o ou output n(numero da tarefa)\nGuardar backup da info atual: -b ou backup\nApagar data atualmente carregada: -c ou clean\nCarregar a data de um dos backups: -f ou fill 'nome do backup'\nSair: quit\n",513);
    }
    else if (!strcmp(splitedinput[0],"output")) {
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
                write(output,"Tarefa não existente\n",22);
                return;
            }
            send[0]='\0';
            linha[0]='\0';
            lseek(logtoSave,beg,SEEK_SET);
            int rea=0;
            while (rea!=end-beg-1) {
                if (MAX>end-beg-1-rea)
                    n=read(logtoSave,send,end-beg-rea-1);
                else 
                    n=read(logtoSave,send,MAX);
                rea+=n;
                write(output,send,n);
            }
        }
        else r=1;
    }
    else if (!strcmp(splitedinput[0],"backup")) {
        int p[2],i,fF,fT,n;
        char nome[MAX],spinner[MAX],linha[MAX];
        signal(SIGCHLD,SIG_IGN);
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
        nome[strlen(nome)-1]='\0';
        strcpy(spinner,"backup/");
        strcat(spinner,nome);
        if (!(i=fork())) {
            execlp("mkdir","mkdir",spinner,NULL);
            _exit(0);
        }
        waitpid(i,NULL,0);
        signal(SIGCHLD,chld_died_handler);
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
                fF=open("files/log.idx",O_RDONLY);
                strcat(spinner,"/log.idx");
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
        write(output,"Backup saved\n",13);
    }
    else if (!strcmp(splitedinput[0],"clean")) {
        indextoSave=open("files/log.idx",O_RDWR | O_TRUNC | O_APPEND);
        history=open("files/history",O_RDWR | O_TRUNC | O_APPEND);
        logtoSave=open("files/log",O_RDWR | O_TRUNC | O_APPEND);
        tempo_inatividade=-1;
        tempo_execucao=-1;
        ntarefa=1;
        for (i=0;i<MAX;i++) {
            if (pid[i][0]!=0) {
                kill(pid[i][0],SIGUSR1);
            }
            comand[i][0]='\0';
        }
        write(output,"All clean now\n",14);
    }
    else if (!strcmp(splitedinput[0],"-f") || !strcmp(splitedinput[0],"fill")) {
        int out,in,i,n;
        char nome[MAX], spinner[MAX];
        nome[0]='\0';
        if (tam > 1) {
            for (i=0;splitedinput[1][i];i++) {
                splitedinput[1][i]=splitedinput[1][i+1];
            }
            strcat(nome,"backup//");
            for (i=1;i<tam;i++) {
                strcat(nome,splitedinput[i]);
                strcat(nome," ");
            }
            nome[strlen(nome)-2]='\0';
            for (int i=0;i<4;i++) {
                spinner[0]='\0';
                strcat(spinner,nome);
                switch (i) {
                    case 0:
                        strcat(spinner,"/data");
                        in=open(spinner,O_RDONLY);
                        out=open("files/data",O_WRONLY | O_CREAT | O_TRUNC,0666);
                    break;
                    case 1:
                        strcat(spinner,"/history");
                        in=open(spinner,O_RDONLY);
                        out=open("files/history",O_WRONLY | O_CREAT | O_TRUNC,0666);
                    break;
                    case 2:
                        strcat(spinner,"/log.idx");
                        in=open(spinner,O_RDONLY);
                        out=open("files/log.idx",O_WRONLY | O_CREAT | O_TRUNC,0666);
                    break;
                    case 3:
                        strcat(spinner,"/log");
                        in=open(spinner,O_RDONLY);
                        out=open("files/log",O_WRONLY | O_CREAT | O_TRUNC,0666);
                    break;
                }
                if (in==-1)  {
                    r=1;
                    break;
                }
                else {
                    while ((n=read(in,spinner,MAX-1))) {
                        write(out,spinner,n);
                    }
                    close(in);
                    close(out);
                }
            }
            write(output,"Os dados de um backup foram carregados para o servidor\n",55);
        }
        else r=1;
    }
    else r=1;
    if (r) {
        write(output,"INVALID INPUT\n",14);
    }
}

/**
 * Função responsável por inicializar o servidor e as suas pastas
 */
void initServer() {
    int n;
    char folder[2][10];
    strcpy(folder[0],"files");
    strcpy(folder[1],"backup");
    ntarefa=1;
    tempo_inatividade=-1;
    tempo_execucao=-1;
    for (n=0;n<MAX;n++) {
        for (int j=0;j<MAX;j++)
            pid[n][j]=0;
        comand[n][0]='\0';
    }
    for (n=0;n<2;n++) {
        if (!fork()) {
        execlp("mkdir","mkdir","-p",folder[n],NULL);
        _exit(0);
        }
        wait(NULL);
    }
    indextoSave = open("files/log.idx",O_RDWR | O_APPEND | O_CREAT, 0666);
    logtoSave = open("files/log",O_RDWR | O_APPEND | O_CREAT, 0666);
    history=open("files/history",O_RDWR | O_APPEND | O_CREAT,0666);
    reloadData();
    signal(SIGCHLD,chld_died_handler);
}

void main() {
    char linha[256];
    initServer();
    while (1) {
        fifo = open("userin",O_RDONLY,0666);
        while (read(fifo,linha,MAX)) {
            output = open("userout",O_WRONLY,0666);
            doStuff(linha);
            saveData();
            close(output);
        }
        close(fifo);
    }
}