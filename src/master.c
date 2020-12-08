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
	
	//Sem
	int idSemMasterClient;
	int idSemMasterWorker;

	//Pipe
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
    int valeur;
    //while(1){
		
		prendre((m->idSemMasterClient));
		read(tcm,&valeur,sizeof(int));
		if(valeur == ORDER_STOP)
		{
			//TODO
		}
		
		if(valeur == ORDER_COMPUTE_PRIME)
		{
			//TODO
			int v;
			read(tcm,&v,sizeof(int));
			if(v>(m->highestAskedNumber)){
				m->highestAskedNumber = v;
			}
			write((m->pipeMasterWorker),&v,sizeof(int));
			
			//Temporaire vend pour le worker
			struct sembuf up ={0,1,0};
			struct sembuf down ={0,-1,0};

			semop((m->idSemMasterWorker), &up, 1);
			int ans;
			sleep(1);
			semop((m->idSemMasterWorker), &down, 1);

			read(m->pipeWorkerMaster,&ans,sizeof(int));
			write(tmc,&ans,sizeof(int));
		}
		
		if(valeur == ORDER_HOW_MANY_PRIME)
		{
			write(tmc,&(m->howManyCalculatedPrime), sizeof(int));
			//TODO
			//redonner l'accès au client

		}
		
		if(valeur == ORDER_HIGHEST_PRIME)
		{
			
			int highestPrime = m->highestPrime;

			write(tmc,&highestPrime, sizeof(int));
			//TODO
			//redonner l'accès au client
		}
		vendre(m->idSemMasterClient);

		//break;
	//}
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
    int semMasterClient = semCreator(keymc);
    semSetVal(semMasterClient,0); // Pour que le master puisse attendre le l'input du client
    
    int  k =ftok("master_worker.h", 2);
    int semMasterWorker = semget(k,1,IPC_CREAT | 0641);
	semctl(semMasterWorker, 0, SETVAL, 0);// Pour que le worker puisse attendrel'input du master
    
    
    // - création des tubes nommés
    mymkfifo("tubeClientMaster",0600); //tube client vers master
    mymkfifo("tubeMasterClient",0600); //tube master vers client
	
	masterStats ms = malloc(sizeof(struct mS));
	ms->highestPrime = 2;
	ms->highestAskedNumber = 1; 
	ms->howManyCalculatedPrime = 1; //ou 1 ?
	ms->idSemMasterClient = semMasterClient;
	ms->idSemMasterWorker = semMasterWorker;
	
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
	
	ms->pipeMasterWorker =pipeMasterWorker[1];
	ms->pipeWorkerMaster =pipeWorkerMaster[0];
	
    // - boucle infinie
    loop(ms);
    
    
    // destruction des tubes nommés, des sémaphores, ...
    sleep(1);
	unlink("tubeClientMaster");
	unlink("tubeMasterClient");
	
	semDestruct(semMasterClient);
	semDestruct(semMasterWorker);
	close(pipeMasterWorker[1]); //partie ecriture
	close(pipeWorkerMaster[0]); //partie lecture
	free(ms);
    return EXIT_SUCCESS;
}

// N'hésitez pas à faire des fonctions annexes ; si les fonctions main
// et loop pouvaient être "courtes", ce serait bien
