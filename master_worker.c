#if defined HAVE_CONFIG_H
#include "config.h"
#endif

#include <stdlib.h>
#include <stdio.h>

#include "myassert.h"

#include "master_worker.h"

// fonctions éventuelles proposées dans le .h



void createFirstWorker(int pipeMasterWorker, int pipeWorkerMaster){
	
	//ordre des arguments : 
	//  - nomExec
	//	- primeNumber
	//	- tube masterWorker
	//	- tube workerMaster
	char * c1 = intToString(pipeMasterWorker);
	char * c2 = intToString(pipeWorkerMaster);
	char * arguments[5] = { 
		"worker",
		"2",
		c1,
		c2,
		NULL};
		
	int son = fork();
	
	//Fils créé ?
	if (son == -1) {
	   perror("createFirstWorker");
	   exit(EXIT_FAILURE);
	}
	
	if(son == 0){
		execv("worker",arguments);
	}
	free(c1);
	free(c2);
}

char * intToString(int val){
	char * buffer = malloc(sizeof(char)*12);
	sprintf(buffer,"%d",val);
	return buffer;
}
