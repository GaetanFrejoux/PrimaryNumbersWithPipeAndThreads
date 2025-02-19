#if defined HAVE_CONFIG_H
#include "config.h"
#endif

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>

#include "myassert.h"

#include "master_client.h"
#include "master_worker.h"

#define DEFAULT_ORDER -10;


/************************************************************************
 * Données persistantes d'un master
 ************************************************************************/
// on peut ici définir une structure stockant tout ce dont le master
// a besoin

typedef struct mS* masterStats;


/************************************************************************
 * Usage et analyse des arguments passés en ligne de commande
 ************************************************************************/

static void usage(const char *exeName, const char *message)
{
    fprintf(stderr, "usage : %s\n", exeName);
    if (message != NULL)
        fprintf(stderr, "message : %s\n", message);
    exit(EXIT_FAILURE);
}


/************************************************************************
 * boucle principale de communication avec le client
 ************************************************************************/
void loop(masterStats m)
{
	//OUVERTURE DES TUBES AVEC LE CLIENT
	printf("\n=== BEGIN ======================================\n");
    
    
    int order = DEFAULT_ORDER; // Stocke l'ordre du client
    int inputNumber = 0; // Stocke la valeur demandé par le client
    int ans = 0; // Stocke la réponse des workers

    while(true){ //Boucle infinie
		printf("\nI'm waiting for a Customer...\n");
		int tcm = myopen("tubeClientMaster",O_RDONLY); //ouverture en mode lecture
		int tmc = myopen("tubeMasterClient",O_WRONLY); //ouverture en mode écriture
		myread(tcm, &order, sizeof(int)); // récupère l'ordre
		printf("\nI received a Customer, Hi !\n");		
		if (order == ORDER_STOP) {
			printf("\n\n=== STOP =======================================\n");
			printf("\nWell, I'll stop my workers\n");
			mywrite((m->pipeMasterWorker), &order, sizeof(int)); // envoie 0 au worker pour qu'il s'arrete;
			wait(NULL); // Attend la fin du Worker 2 et donc de tous les Workers
			printf("\nI finished, Bye !\n");
			printf("\n=== END PROGRAM ================================\n\n");
			mywrite(tmc, &order, sizeof(int));//test renvoie -1
			break; //Sortie de la boucle while
		}
		
		else if (order == ORDER_COMPUTE_PRIME) {
			computePrimeMaster(ans, inputNumber, tcm, tmc, m); //Définie dans le .h
		}
		
		else if (order == ORDER_HOW_MANY_PRIME) {
			mywrite(tmc, &(m->howManyCalculatedPrime), sizeof(int)); // écrit la valeur dans le tube
			printf("\n\n=== HOWMANY =====================================\n");
			printf("\n%d Workers have been created\n", m->howManyCalculatedPrime);
			printf("\n=== END HOWMANY =================================\n");
		}
		
		else if (order == ORDER_HIGHEST_PRIME) {
			mywrite(tmc,&(m->highestPrime), sizeof(int)); // écrit la valeur dans le tube
			printf("\n\n=== HIGHEST ====================================\n");
			printf("\nThe highest prime I discovered is %d\n", m->highestPrime);
			printf("\n=== END HIGHEST ================================\n");
		}
		order = DEFAULT_ORDER; //On redonne à order une valeur qui ne correspond à rien
		closePipe(tmc, tcm);
		prendre(m->sem); // Attends que le client se termine.
	}

	//FERMETURE DES TUBES NOMMÉS
}


/************************************************************************
 * Fonction principale
 ************************************************************************/

int main(int argc, char * argv[])
{
    if (argc != 1)
        usage(argv[0], NULL);

    //CRÉATION DES SÉMAPHORES
	//pour les clients entre eux
	int keym = getKey("master_client.h", CLIENTS_ID);
	int semClient = semCreation(keym, 1);

	//Pour attendre la fin de chaque client
	int keymc = getKey("master_client.h", MASTER_CLIENT_ID);
	int semClientMaster = semCreation(keymc, 0);

    //CRÉATION DES TUBES NOMMÉS
	linkMasterClient("tubeClientMaster", "tubeMasterClient");
	int pipeMasterWorker[2]; // master vers worker
	mypipe(pipeMasterWorker);
	
	int pipeWorkerMaster[2]; // worker vers master
	mypipe(pipeWorkerMaster);
	
	//CRÉATION DU PREMIER WORKER
	createFirstWorker(pipeMasterWorker[0],pipeWorkerMaster[1]);
	
	//FERMETURE DES PIPES (LECTURE DU MASTER ET ÉCRITURE DU WORKER)
	closePipe(pipeMasterWorker[0], pipeWorkerMaster[1]);
	
	//Création de la structure stockant les données nécessaires pour le master
	
	masterStats ms = initMasterStats(2, 2, 1, pipeMasterWorker[1], pipeWorkerMaster[0],semClientMaster);
	
    //BOUCLE INFINIE
    loop(ms);

    prendre(ms->sem); // Attends que le client se termine.
	
	//ON FERME ET ON DÉTRUIT TOUT
    unlink("tubeClientMaster");
	unlink("tubeMasterClient");

	semDestruct(semClient);
	semDestruct(semClientMaster);
	closePipe(pipeWorkerMaster[0], pipeMasterWorker[1]);
	free(ms);
	
    return EXIT_SUCCESS;
}
