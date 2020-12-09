#if defined HAVE_CONFIG_H
#include "config.h"
#endif

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>

#include "myassert.h"

#include "master_worker.h"

/************************************************************************
 * Données persistantes d'un worker
 ************************************************************************/

// on peut ici définir une structure stockant tout ce dont le worker
// a besoin : le nombre premier dont il a la charge, ...


/************************************************************************
 * Usage et analyse des arguments passés en ligne de commande
 ************************************************************************/

static void usage(const char *exeName, const char *message)
{
    fprintf(stderr, "usage : %s <n> <fdIn> <fdToMaster>\n", exeName);
    fprintf(stderr, "   <n> : nombre premier géré par le worker\n");
    fprintf(stderr, "   <fdIn> : canal d'entrée pour tester un nombre\n");
    fprintf(stderr, "   <fdToMaster> : canal de sortie pour indiquer si un nombre est premier ou non\n");
    if (message != NULL)
        fprintf(stderr, "message : %s\n", message);
    exit(EXIT_FAILURE);
}

static void parseArgs(int argc, char * argv[] , workerStats ws)
{
    if (argc != 4)
        usage(argv[0], "Nombre d'arguments incorrect");

    // remplir la structure
    ws->primeNumber = strtol(argv[1],NULL,10);
    ws->prevWorker = strtol(argv[2],NULL,10);
    ws->master = strtol(argv[3],NULL,10);
}

/************************************************************************
 * Boucle principale de traitement
 ************************************************************************/

void loop(workerStats ws)
{
	int valeur;
	int ans;
	int son = 0; // pour que le premier worker puisse créer le second
	while(1){
		read(ws->prevWorker,&valeur,sizeof(int));
		if(valeur ==-1 ){
			//TODO : 
			// envoie le signal au autres worker
			wait(NULL);
			break;
		}
		if(valeur ==(ws->primeNumber)){
			ans = 1; // est prime
			write(ws->master,&ans,sizeof(int));
			write(ws->master,&(ws->primeNumber),sizeof(int));


		}
		else if(valeur%(ws->primeNumber)==0){
			ans = 0; //faux
			write(ws->master,&ans,sizeof(int));
			write(ws->master,&(ws->primeNumber),sizeof(int));

		}
		else{
			//TODO Creer le fils s'il n'est pas créé
			
			if(son == 0){ // Si il n'a pas déjà créé un fils
				
				int fd[2]; 
				pipe(fd); // creation du pipe entre le worker et son fils 
				
				son = fork(); //créer le fils
				
				if(son == 0){ // si c'est le fils
					ans = 1;
					ws->primeNumber = valeur; // il prend la valeur actelle;
					close(fd[1]);
					ws->prevWorker = fd[0];
					write(ws->master,&ans,sizeof(int));
					write(ws->master,&(ws->primeNumber),sizeof(int));				
				}
				else{
					close(fd[0]);
					ws->nextWorker = fd[1];
				}
			}
			else{
				//TODO transmetre le nombre au fils avec le tube
				write(ws->nextWorker,&valeur,sizeof(int));
			}
		}
		
	}


	
	
    // boucle infinie :
    //    attendre l'arrivée d'un nombre à tester
    //    si ordre d'arrêt
    //       si il y a un worker suivant, transmettre l'ordre et attendre sa fin
    //       sortir de la boucle
    //    sinon c'est un nombre à tester, 4 possibilités :
    //           - le nombre est premier
    //           - le nombre n'est pas premier
    //           - s'il y a un worker suivant lui transmettre le nombre
    //           - s'il n'y a pas de worker suivant, le créer
}

/************************************************************************
 * Programme principal
 ************************************************************************/

int main(int argc, char * argv[])
{
	workerStats ws = malloc( sizeof( struct wS ) );

    parseArgs(argc, argv ,ws);
    
    // Si on est créé c'est qu'on est un nombre premier
    
    // Envoyer au master un message positif pour dire
    // que le nombre testé est bien premier

    loop(ws);

    // libérer les ressources : fermeture des files descriptors par exemple
    close(ws->master);
	close(ws->prevWorker);
	free(ws);
    return EXIT_SUCCESS;
}
