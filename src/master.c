#if defined HAVE_CONFIG_H
#include "config.h"
#endif

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>

#include "myassert.h"

#include "master_client.h"
#include "master_worker.h"


#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>

/************************************************************************
 * Données persistantes d'un master
 ************************************************************************/

// on peut ici définir une structure stockant tout ce dont le master
// a besoin
typedef struct{
	int highestPrime;
	int highestAskedNumber;
	int howManyCalculatedPrime;
} *masterStats;


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
	int tcm = open("tubeClientMaster",O_RDONLY); //ouverture en mode lecture
    int tmc = open("tubeMasterClient",O_WRONLY); //ouverture en mode écriture
    int valeur;
    while(true){
		if(read(tcm,&valeur,sizeof(int))==1)
		{
			if(valeur == ORDER_STOP)
			{
				//TODO
			}
			
			if(valeur == ORDER_COMPUTE_PRIME)
			{
				//TODO
			}
			
			if(valeur == ORDER_HOW_MANY_PRIME)
			{
				//TODO
			}
			
			if(valeur == ORDER_HIGHEST_PRIME)
			{
				//TODO
			}
		}
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
}


/************************************************************************
 * Fonction principale
 ************************************************************************/

int main(int argc, char * argv[])
{
    if (argc != 1)
        usage(argv[0], NULL);

    // - création des sémaphores
    
    
    // - création des tubes nommés
    mkfifo("tubeClientMaster",0600); //tube client vers master
    mkfifo("tubeMasterClient",0600); //tube master vers client
    
    
    // - création du premier worker
    int son = fork();
    if(son == 0){
		execv("worker",NULL);
	}
	
	
	masterStats ms;
	ms->highestPrime = 2;
	ms->highestAskedNumber = 1; 
	ms->howManyCalculatedPrime = 0; //ou 1 ?
    // boucle infinie
    
    loop(ms);

    // destruction des tubes nommés, des sémaphores, ...

    return EXIT_SUCCESS;
}

// N'hésitez pas à faire des fonctions annexes ; si les fonctions main
// et loop pouvaient être "courtes", ce serait bien
