#if defined HAVE_CONFIG_H
#include "config.h"
#endif

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>

#include "myassert.h"

#include "master_client.h"
#include "master_worker.h"


/************************************************************************
 * Données persistantes d'un master
 ************************************************************************/
// on peut ici définir une structure stockant tout ce dont le master
// a besoin
struct mS{
	
	//Stats
	int highestPrime;
	int highestAskedNumber;
	int howManyCalculatedPrime;

	//Pipes
	int pipeMasterWorker;
	int pipeWorkerMaster;
};

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
	
	int tcm = myopen("tubeClientMaster",O_RDONLY); //ouverture en mode lecture
    int tmc = myopen("tubeMasterClient",O_WRONLY); //ouverture en mode écriture
    
    
    int order=-10; // Stocke l'ordre du client
    int inputNumber; // Stocke la valeur demandé par le client
    int ans; // Stocke la réponse des workers

    while(true){ //Boucle infinie
		
		read(tcm,&order,sizeof(int)); // récupère l'ordre
		
		if(order == ORDER_STOP)
		{
			write((m->pipeMasterWorker),&order,sizeof(int)); // envoie 0 au worker pour qu'il s'arrete;
			wait(NULL);
			break; //Sortie de la boucle while
		}
		
		if(order == ORDER_COMPUTE_PRIME)
		{
			
			read(tcm,&inputNumber,sizeof(int));// récupère la valeur
			
			//4 cas :
			
			if(inputNumber < m->highestPrime){
				
				write((m->pipeMasterWorker),&inputNumber,sizeof(int));//le nombre
				write((m->pipeMasterWorker),&inputNumber,sizeof(int));//le nombre
				read(m->pipeWorkerMaster,&ans,sizeof(int));
				

			}
			else if(inputNumber == m->highestPrime ){
				
				ans = 1; //est un nb premier

			}
			
			else if((inputNumber > m->highestPrime) && (inputNumber <= m->highestAskedNumber )){
				
				ans = 0; //n'est un nb premier

			}
			else{
				int inf = (m->highestAskedNumber)+1;
				int sup = inputNumber;
				
				for(int i = inf ; i < sup ; i++){
					
					write((m->pipeMasterWorker),&i,sizeof(int));
					read(m->pipeWorkerMaster,&ans,sizeof(int));
					read(m->pipeWorkerMaster,&ans,sizeof(int));
					if( ans > (m->highestPrime) ){
						m->highestPrime =ans;
						m->howManyCalculatedPrime++;
					}
				}
				
				write((m->pipeMasterWorker),&inputNumber,sizeof(int));
				read(m->pipeWorkerMaster,&ans,sizeof(int));
				
				m->highestAskedNumber = inputNumber; //la valeur la plus élévé est maintenant égal à l'input
				
				int tmp;
				read(m->pipeWorkerMaster,&tmp,sizeof(int)); // le plus grand nombre premier calculé.
				
				if( tmp > (m->highestPrime)){
					m->highestPrime = tmp;
					m->howManyCalculatedPrime++;
				}
			}
			
			write(tmc,&ans,sizeof(int)); // envoie la réponse au client 
			
		}
		
		
		
		if(order == ORDER_HOW_MANY_PRIME)
		{
			
			write(tmc,&(m->howManyCalculatedPrime), sizeof(int)); // écrit la valeur dans le tube

		}
		
		if(order == ORDER_HIGHEST_PRIME)
		{
		
			write(tmc,&(m->highestPrime), sizeof(int)); // écrit la valeur dans le tube
		}
		
		order = -10; // on redonne à order une valeur qui ne correspond à rien
	
	}
	// ferme les tubes physiques
	close(tcm); 
	close(tmc);
}
	
	
	
    // boucle infinie :
    // - ouverture des tubes (cf. rq client.c)
    // - attente d'un ordre du client (via le tube nommé)
    // - si ORDER_STOP
    //       . envoyer ordre de fin au premier worker et attendre sa fin
    //       . envoyer un accusé de réception au client
    // - si ORDER_COMPUTE_PRIME
    //       . récupérer le nombre N à tester provenant du client
    //       . construire le pipeline jusqu'au nombre N-1 (si non encore fait) :
    //             il faut connaître le plus nombre (M) déjà enovoyé aux workers
    //             on leur envoie tous les nombres entre M+1 et N-1
    //             note : chaque envoie déclenche une réponse des workers
    //       . envoyer N dans le pipeline
    //       . récupérer la réponse
    //       . la transmettre au client
    // - si ORDER_HOW_MANY_PRIME
    //       . transmettre la réponse au client
    // - si ORDER_HIGHEST_PRIME
    //       . transmettre la réponse au client
    // - fermer les tubes nommés
    // - attendre ordre du client avant de continuer (sémaphore : précédence)
    // - revenir en début de boucle
    //
    // il est important d'ouvrir et fermer les tubes nommés à chaque itération
    // voyez-vous pourquoi ?


/************************************************************************
 * Fonction principale
 ************************************************************************/

int main(int argc, char * argv[])
{
    if (argc != 1)
        usage(argv[0], NULL);

    // - création des sémaphores
    
	int keymc = getKey("master_client.h", PROJ_ID);
    int semClient = semCreator(keymc);
    semSetVal(semClient,1);
    
    
    // - création des tubes nommés
    
    mymkfifo("tubeClientMaster",0600); //tube client vers master
    mymkfifo("tubeMasterClient",0600); //tube master vers client
	
	
	// - création des tubes anonymes
	
	int pipeMasterWorker[2]; // master vers worker
	pipe(pipeMasterWorker);
	
	int pipeWorkerMaster[2]; // worker vers master
	pipe(pipeWorkerMaster);
	
	
	// - création du premier worker
	
	createFirstWorker(pipeMasterWorker[0],pipeWorkerMaster[1]);
	
	
	// - fermeture des pipes 
	
	close(pipeMasterWorker[0]); //partie lecture
	close(pipeWorkerMaster[1]); //partie ecriture
	
	
	// - création de la structure stockant les données nécessaires pour le master
	
	masterStats ms = malloc(sizeof(struct mS));
	
	ms->highestPrime = 2;
	ms->highestAskedNumber = 2; 
	ms->howManyCalculatedPrime = 1;
	ms->pipeMasterWorker =pipeMasterWorker[1];
	ms->pipeWorkerMaster =pipeWorkerMaster[0];
	
	
    // - boucle infinie
    loop(ms);
    
    
    // destruction des tubes nommés, des sémaphores, ...
	unlink("tubeClientMaster");
	unlink("tubeMasterClient");
	
	semDestruct(semClient);
	
	close(pipeMasterWorker[1]); //partie ecriture
	close(pipeWorkerMaster[0]); //partie lecture
	
	free(ms);
	
    return EXIT_SUCCESS;
}

// N'hésitez pas à faire des fonctions annexes ; si les fonctions main
// et loop pouvaient être "courtes", ce serait bien
