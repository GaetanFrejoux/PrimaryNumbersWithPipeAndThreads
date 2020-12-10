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
	printf("\nI'm waiting for a Customer...\n");
	int tcm = myopen("tubeClientMaster",O_RDONLY); //ouverture en mode lecture
	printf("\nI received a Customer, Hi !\n");
    int tmc = myopen("tubeMasterClient",O_WRONLY); //ouverture en mode écriture
    
    int order = DEFAULT_ORDER; // Stocke l'ordre du client
    int inputNumber = 0; // Stocke la valeur demandé par le client
    int ans = 0; // Stocke la réponse des workers

    while(true){ //Boucle infinie
		
		myread(tcm, &order, sizeof(int)); // récupère l'ordre
		
		if (order == ORDER_STOP) {
			printf("\n\n=== STOP =======================================\n");
			printf("\nWell, I'll stop my workers\n");
			mywrite((m->pipeMasterWorker), &order, sizeof(int)); // envoie 0 au worker pour qu'il s'arrete;
			wait(NULL);
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
	}

	//FERMETURE DES TUBES NOMMÉS
	closePipe(tmc, tcm);
}


/************************************************************************
 * Fonction principale
 ************************************************************************/

int main(int argc, char * argv[])
{
    if (argc != 1)
        usage(argv[0], NULL);

    //CRÉATION DES SÉMAPHORES
	int keymc = getKey("master_client.h", PROJ_ID);
	int semClient = semCreation(keymc, 1);
    
    //CRÉATION DES TUBES NOMMÉS
	linkMasterClient("tubeClientMaster", "tubeMasterClient");
	
	//CRÉATION DES TUBES ANONYMES
	int pipeMasterWorker[2]; // master vers worker
	pipe(pipeMasterWorker);
	
	int pipeWorkerMaster[2]; // worker vers master
	pipe(pipeWorkerMaster);
	
	//CRÉATION DU PREMIER WORKER
	createFirstWorker(pipeMasterWorker[0],pipeWorkerMaster[1]);
	
	//FERMETURE DES PIPES (LECTURE DU MASTER ET ÉCRITURE DU WORKER)
	closePipe(pipeMasterWorker[0], pipeWorkerMaster[1]);
	
	//Création de la structure stockant les données nécessaires pour le master
	
	masterStats ms = initMasterStats(2, 2, 1, pipeMasterWorker[1], pipeWorkerMaster[0]);
	
    //BOUCLE INFINIE
    loop(ms);
    
	//ON FERME ET ON DÉTRUIT TOUT
    unlink("tubeClientMaster");
	unlink("tubeMasterClient");
	semDestruct(semClient);
	closePipe(pipeWorkerMaster[0], pipeMasterWorker[1]);
	free(ms);
	
    return EXIT_SUCCESS;
}
