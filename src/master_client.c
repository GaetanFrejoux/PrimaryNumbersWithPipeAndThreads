#if defined HAVE_CONFIG_H
#include "config.h"
#endif

#define _XOPEN_SOURCE

#include <stdlib.h>
#include <stdio.h>

#include "myassert.h"

#include "master_client.h"

/*** === FONCTIONS === ***/


// === MASTER ===

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


// === ORDRES === 

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
        printf("\nMaster : %d IS a prime number\n", request);
    }

    else{
        printf("\nMaster : %d IS NOT a prime number\n", request);
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

/*Compute local*/
void computeLocal(int N)
{
    int size = sqrt(N)-1;
    bool tab[size];
    pthread_t tabId[size];
    ThreadData data[size];

    printf("\n=== COMPUTE LOCAL =========================================\n\n");
    printf("My threads are working...\n");

    //Initialisation du tableau de booléens
    for (int i = 0; i < size; i++)
    {
        tab[i] = true;
    }

    //Pré-initialisation des threads
    preInitThread(N, size, tab, data);

    //Lancements des threads
    for (int i = 0; i < size; i++)
    {
        mycreate(&(tabId[i]), NULL, codeThread, &(data[i]));
    }
    
    //Attente de la fin des threads
    for (int i = 0; i < size; i++)
    {
        myjoin(tabId[i], NULL);
    }

    //TEST SI TOUS LES BOOLÉENS SONT == TRUE
    bool res = true;
    size--;

    while(size > -1)
    {
        if(!tab[size])
        {
            res = false;
            break;
        }
        size--;
    }

    printf("\nHum, that's for sure that ");

    if (res)
    {
        printf("%d IS a prime\n", N);
    }
    else
    {
        printf("%d IS NOT a prime\n", N);
    }
    
    printf("\n===========================================================\n");

}

/*Compute Prime du Master*/
void computePrimeMaster(int ans, int inputNumber, int r, int w, struct mS *m)
{
    myread(r, &inputNumber, sizeof(int));// récupère la valeur
    printf("\n\n=== COMPUTE %d ==================================\n", inputNumber);
    printf("\nOk, I'll search if %d is a prime\n", inputNumber);
    
    //4 CAS :
    
    if(inputNumber < m->highestPrime)
    {
        int tmp;
        mywrite((m->pipeMasterWorker), &inputNumber, sizeof(int)); //le nombre
        myread(m->pipeWorkerMaster, &ans, sizeof(int));
        myread(m->pipeWorkerMaster, &tmp, sizeof(int));
    }

    else if(inputNumber == m->highestPrime)
    {
        ans = 1; //est un nb premier
    }
    
    else if((inputNumber > m->highestPrime) && (inputNumber <= m->highestAskedNumber))
    {
        ans = 0; //n'est pas un nb premier
    }

    else
    {
        int inf = (m->highestAskedNumber);
        int sup = inputNumber;
        
        for(int i = inf ; i < sup ; i++)
        {
            mywrite((m->pipeMasterWorker), &i, sizeof(int));
            myread(m->pipeWorkerMaster, &ans, sizeof(int));
            myread(m->pipeWorkerMaster, &ans, sizeof(int));

            if( ans > (m->highestPrime) )
            {
                m->highestPrime = ans;
                printf("\n***** WORKER %d was created *****\n", m->highestPrime);
                m->howManyCalculatedPrime++;
            }
        }

        printf("\nEnough Workers were created\n");
        
        mywrite((m->pipeMasterWorker), &inputNumber,sizeof(int));
        myread(m->pipeWorkerMaster, &ans,sizeof(int));
        
        m->highestAskedNumber = inputNumber; //la valeur la plus élévée est maintenant égale à l'input
        
        int tmp;
        myread(m->pipeWorkerMaster, &tmp, sizeof(int)); // le plus grand nombre premier calculé.
        
        if( tmp > (m->highestPrime)){
            m->highestPrime = tmp;
            m->howManyCalculatedPrime++;
        }
    }
    printf("\n=== END COMPUTE %d ==============================\n", inputNumber);
    mywrite(w, &ans, sizeof(int)); //Envoi de la réponse au client
}

// === SÉMAPHORES === 

/*Création d'un sémaphore et initialisation*/
int semCreation(int key, int initVal)
{
    int semClient = semCreator(key);
    semSetVal(semClient,initVal);

    return semClient;
}


// === TUBES === 

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


// === THREADS ===

/*Create the thread*/
void mycreate(pthread_t *thread, const pthread_attr_t *attr, void *(*start_routine) (void *), void *arg)
{
	int test = pthread_create(thread, attr, start_routine, arg);
	myassert(test > -1, "ERROR : Thread is not created");
}

void myjoin(pthread_t thread, void **retval)
{
	int test = pthread_join(thread, retval);
	myassert(test > -1, "ERROR : myjoin()");
}

/*Initialisation des thread*/
void preInitThread(int N, int nbThread, bool *tab, ThreadData *data)
{
    for (int i = 0; i < nbThread; i++)
    {
        data[i].thValue = i+2;
        data[i].value = N;
        data[i].res = &tab[i];
    }
}

/*Code des threads du client*/
void *codeThread(void *var)
{
    ThreadData *d = (ThreadData*) var;

    if ((d->value) % (d->thValue) == 0)
    {
        *d->res = false;
    }

    return NULL;
}


//END
