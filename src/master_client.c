#if defined HAVE_CONFIG_H
#include "config.h"
#endif

#define _XOPEN_SOURCE

#include <stdlib.h>
#include <stdio.h>

#include "myassert.h"

#include "master_client.h"

/*** === FONCTIONS === ***/

//MASTER

/*Création et initialisation de la structure*/
struct mS *initMasterStats(int highest, int highestAsked, int howmany, int w, int r)
{
    struct mS *m = malloc(sizeof(struct mS));
	
	m->highestPrime = highest;
	m->highestAskedNumber = highestAsked; 
	m->howManyCalculatedPrime = howmany;
	m->pipeMasterWorker = w;
	m->pipeWorkerMaster = r;

    return m;
}

//ORDRES

/*Demande au Master si un nombre est premier ou non*/
void computePrimeClient(int semId, int w, int r, int order, int request)
{
    int res = 0;

    prendre(semId);//On réserve notre place auprès du Master

    printf("\n===========================================================\n\n");
    printf("Hi, is %d a prime number ?\n", request);

    mywrite(w, &order, sizeof(int));//On écrit notre ordre dans le tube
	mywrite(w, &request, sizeof(int));//Et la requête qui suit
		
	myread(r, &res, sizeof(int));//Réponse du Master

    if(res == 1){
        printf("\nMaster : %d is a prime number\n", request);
    }

    else{
        printf("\nMaster : %d is not a prime number\n", request);
    }
    printf("\n===========================================================\n");

    vendre(semId);//On rend la place
}

/*Gère les requêtes à un seul ordre côté Client*/
void oneOrderRequestClient(int semId, int w, int r, int order, char *answer)
{
    int res = 0;

    prendre(semId);//On réserve la place auprès du Master

    printf("\n===========================================================\n\n");

    mywrite(w, &order,sizeof(int));//On donne l'ordre au Master
	myread(r, &res,sizeof(int));//On récupère la réponse du Master
	
    printf(answer, res);
    printf("\n===========================================================\n");

    if(order != -1)//En gros, si le Master n'est pas stoppé
    {
        vendre(semId);//On passe le relais au client suivant
    }
}


//SÉMAPHORES

/*Création d'un sémaphore et initialisation*/
int semCreation(int key, int initVal)
{
    int semClient = semCreator(key);
    semSetVal(semClient,initVal);

    return semClient;
}

//TUBES

/*Création des tubes nommés*/
void linkMasterClient(char *tubeCM, char *tubeMC)
{
    mymkfifo(tubeCM,0600); //tube client vers master
    mymkfifo(tubeMC,0600); //tube master vers client
}

/*Fermeture des pipes*/
void closePipe(int r, int w)
{
    myclose(r); //partie lecture
	myclose(w); //partie ecriture
}

//END
