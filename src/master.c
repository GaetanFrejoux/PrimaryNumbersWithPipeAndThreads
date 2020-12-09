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
// struct mS{
	
// 	//Stats
// 	int highestPrime;
// 	int highestAskedNumber;
// 	int howManyCalculatedPrime;

// 	//Pipes
// 	int pipeMasterWorker;
// 	int pipeWorkerMaster;
// };

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
    int inputNumber; // Stocke la valeur demandé par le client
    int ans; // Stocke la réponse des workers

    while(true){ //Boucle infinie
		
		myread(tcm, &order, sizeof(int)); // récupère l'ordre
		
		if(order == ORDER_STOP)
		{
			printf("\n\n=== STOP =======================================\n");
			printf("\nWell, I'll stop my workers\n");
			mywrite((m->pipeMasterWorker), &order, sizeof(int)); // envoie 0 au worker pour qu'il s'arrete;
			wait(NULL);
			printf("\nI finished, Bye !\n");
			printf("\n=== END PROGRAM ================================\n");
			mywrite(tmc, &order, sizeof(int));//test renvoie -1
			break; //Sortie de la boucle while
		}
		
		if(order == ORDER_COMPUTE_PRIME)
		{
			myread(tcm, &inputNumber, sizeof(int));// récupère la valeur
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
			mywrite(tmc, &ans, sizeof(int)); //Envoi de la réponse au client 
		}
		
		if(order == ORDER_HOW_MANY_PRIME)
		{
			mywrite(tmc, &(m->howManyCalculatedPrime), sizeof(int)); // écrit la valeur dans le tube
			printf("\n\n=== HOWMANY =====================================\n");
			printf("\n%d Workers have been created\n", m->howManyCalculatedPrime);
			printf("\n=== END HOWMANY =================================\n");
		}
		
		if(order == ORDER_HIGHEST_PRIME)
		{
			mywrite(tmc,&(m->highestPrime), sizeof(int)); // écrit la valeur dans le tube
			printf("\n\n=== HIGHEST ====================================\n");
			printf("\nThe highest prime I discovered is %d\n", m->highestPrime);
			printf("\n=== END HIGHEST ================================\n");
		}
		
		order = DEFAULT_ORDER; // on redonne à order une valeur qui ne correspond à rien
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
	
	// malloc(sizeof(struct mS));
	
	// ms->highestPrime = 2;
	// ms->highestAskedNumber = 2; 
	// ms->howManyCalculatedPrime = 1;
	// ms->pipeMasterWorker = pipeMasterWorker[1];
	// ms->pipeWorkerMaster = pipeWorkerMaster[0];
	
	
    //BOUCLE INFINIE
    loop(ms);
    
	//ON FERME ET ON DÉTRUIT TOUT
    unlink("tubeClientMaster");
	unlink("tubeMasterClient");
	semDestruct(semClient);
	// myclose(pipeMasterWorker[1]); //partie ecriture
	// myclose(pipeWorkerMaster[0]); //partie lecture
	closePipe(pipeWorkerMaster[0], pipeMasterWorker[1]);
	free(ms);
	
    return EXIT_SUCCESS;
}

// N'hésitez pas à faire des fonctions annexes ; si les fonctions main
// et loop pouvaient être "courtes", ce serait bien
