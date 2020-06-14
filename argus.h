#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <signal.h>

void saveData();

void reloadData();

int getPIDOfTask(int task);

int getIndexOfPID(int pidd);

void addProcess(int p,int pai);

void saveToLogs(int * p,int t,int died);

void chld_died_handler();

void terminate(int s);

int addToPIDList(int pidd);

void sendsig();

void doStuff(char * linha);