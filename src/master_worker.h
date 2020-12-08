#ifndef MASTER_WORKER_H
#define MASTER_WORKER_H

// On peut mettre ici des éléments propres au couple master/worker :
//    - des constantes pour rendre plus lisible les comunications
//    - des fonctions communes (écriture dans un tube, ...)
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/sem.h>
struct wS{
	int primeNumber; // sa valeur premiere
	int prevWorker; // le tube vers le  worker precedant (read)
	int master; // le tube vers le master (write)
	int sem; //semaphore pour que le worker attendent le master
};

typedef struct wS* workerStats;
 
void createFirstWorker(int pipeMasterWorker, int pipeWorkerMaster);
char * intToString(int val);
#endif
