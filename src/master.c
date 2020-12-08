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
static struct sembuf up = {0,1,0};
static struct sembuf down ={0,-1,0};
static struct sembuf nul ={0,0,0};

// on peut ici définir une structure stockant tout ce dont le master
// a besoin
struct mS{
	int highestPrime;
	int highestAskedNumber;
	int howManyCalculatedPrime;
	int idSemMasterClient;
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
	int tcm = myopen("tubeClientMaster",0666); //ouverture en mode lecture
    int tmc = myopen("tubeMasterClient",0666); //ouverture en mode écriture
    int valeur;
    while(1){
			semOperation((m->idSemMasterClient),down,1);
			read(tcm,&valeur,sizeof(int));
			if(valeur == ORDER_STOP)
			{
				//TODO
			}
			
			if(valeur == ORDER_COMPUTE_PRIME)
			{
				//TODO
				int v;
				if(read(tcm,&v,sizeof(int))==1){
					if(v>(m->highestAskedNumber)){
						m->highestAskedNumber = v;
					}
				}
				else {
					//ERREUR
				}
			}
			
			if(valeur == ORDER_HOW_MANY_PRIME)
			{
				write(tmc,&(m->howManyCalculatedPrime), sizeof(int));
				//TODO
				//redonner l'accès au client
				semOperation((m->idSemMasterClient),up,1);
				sleep(1);

			}
			
			if(valeur == ORDER_HIGHEST_PRIME)
			{
				
				int highestPrime = m->highestPrime;

				write(tmc,&highestPrime, sizeof(int));
				//TODO
				//redonner l'accès au client
			}
			break;
		}
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
	int key = getKey("master_client.h", PROJ_ID);
    int semMasterClient = semCreator(key);
    semSetVal(semMasterClient,0); // Pour que le master puisse attendre le l'input du client
    // - création des tubes nommés
    mymkfifo("tubeClientMaster",0600); //tube client vers master
    mymkfifo("tubeMasterClient",0600); //tube master vers client
    
    
    // - création du premier worker
    /*int son = fork();
    if(son == 0){
		execv("worker",NULL);
	}
	*/
	
	masterStats ms = malloc(sizeof(struct mS));
	ms->highestPrime = 2;
	ms->highestAskedNumber = 1; 
	ms->howManyCalculatedPrime = 0; //ou 1 ?
	ms->idSemMasterClient = semMasterClient;
	
	
    // boucle infinie
    loop(ms);

    // destruction des tubes nommés, des sémaphores, ...
	unlink("tubeClientMaster");
	unlink("tubeMasterClient");
	semDestruct(semMasterClient);
    return EXIT_SUCCESS;
}

// N'hésitez pas à faire des fonctions annexes ; si les fonctions main
// et loop pouvaient être "courtes", ce serait bien
