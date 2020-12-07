#if defined HAVE_CONFIG_H
#include "config.h"
#endif

#define _XOPEN_SOURCE

#include <stdlib.h>
#include <stdio.h>

#include "myassert.h"

#include "master_client.h"

// fonctions éventuelles proposées dans le .h

//SEMBUF




key_t getKey(){
	key_t k = ftok("master_client.c",PROJ_ID); 
	myassert(k!=-1,"ftok function didn't succed to create a key");
	return k;
}

int semCreator(){
	key_t k = getKey();
	int sem = semget(k,1, IPC_CREAT | 0641);
	myassert(sem!=-1, "Error when trying to create the semaphore for master and client");
	return sem;
}
int semGet(){
	key_t k = getKey();
	int sem = semget(k,1, 0);
	myassert(sem!=-1, "Error when trying to get the semaphore for master and client");
	return sem;
}

void semSetVal(int id,int val){
	int t = semctl(id,0,SETVAL,val);
	myassert(t!=-1,"Error when trying to set the value of the semaphore");
}

void semOperation(int id , struct sembuf op , int time){
	int t = semop(id,&op,time);
	myassert(t!=-1,"Error when making an operation");
} 
