#ifndef CLIENT_CRIBLE
#define CLIENT_CRIBLE

// On peut mettre ici des éléments propres au couple master/client :
//    - des constantes pour rendre plus lisible les comunications
//    - des fonctions communes (création tubes, écriture dans un tube,
//      manipulation de sémaphores, ...)

/*** === INCLUDE === ***/

#include "utils.h"


/*** === CONSTANTES === ***/

// ordres possibles pour le master
#define ORDER_NONE                0
#define ORDER_STOP               -1
#define ORDER_COMPUTE_PRIME       1
#define ORDER_HOW_MANY_PRIME      2
#define ORDER_HIGHEST_PRIME       3
#define ORDER_COMPUTE_PRIME_LOCAL 4   // ne concerne pas le master

/*Identifiant pour le deuxième paramètre de ftok*/
#define CLIENTS_ID 1
#define MASTER_CLIENT_ID 5



/*** === STRUCTURES === ***/

//Structure pour le master
struct mS
{
	//Stats
	int highestPrime;
	int highestAskedNumber;
	int howManyCalculatedPrime;

	//Pipes
	int pipeMasterWorker;
	int pipeWorkerMaster;

	//Semaphore
	int sem;
};

//Structure pour les thread
typedef struct
{
    int thValue;
    int value;
    bool * res;
}ThreadData;


/*** === FONCTIONS === ***/


// === MASTER ===

/*Création et initialisation de la structure*/
struct mS *initMasterStats(int highest, int highestAsked, int howmany, int w, int r,int s);


// === ORDRES ===


//CLIENT

/*Demande au Master si un nombre est premier ou non*/
void computePrimeClient(int semId, int w, int r, int order, int request);

/*Gère les requêtes à un seul ordre côté Client*/
void oneOrderRequestClient(int semId, int w, int r, int order, char *answer);

/*Compute local*/
void computeLocal(int N);


//MASTER

/*Compute Prime du Master*/
void computePrimeMaster(int ans, int inputNumber, int r, int w, struct mS *ms);


// === SÉMAPHORES ===

/*Création d'un sémaphore et initialisation*/
int semCreation(int key, int initVal);


// === TUBES ===

/*Création des tubes nommés*/
void linkMasterClient(char *tubeCM, char *tubeMC);

/*Fermeture des pipes*/
void closePipe(int r, int w);


// === THREADS CLIENT ===

void mycreate(pthread_t *thread, const pthread_attr_t *attr, void *(*start_routine) (void *), void *arg);
void myjoin(pthread_t thread, void **retval);
void myexit();

/*Initialisation des thread*/
void preInitThread(int N, int nbThread, bool *tab, ThreadData *data);

/*Code des threads du client*/
void *codeThread(void *var);


#endif
