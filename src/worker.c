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

typedef struct wS* workerStats;

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
 * Fonction pour le fils
 ************************************************************************/

void pizza(int valeur, int fd[], workerStats ws)
{
	int tmp = 1;
	ws->primeNumber = valeur; //Prend la valeur actelle;
	myclose(fd[1]);
	ws->prevWorker = fd[0];
	mywrite(ws->master, &tmp, sizeof(int));
	mywrite(ws->master, &(ws->primeNumber), sizeof(int));
}

/************************************************************************
 * Boucle principale de traitement
 ************************************************************************/

void loop(workerStats ws)
{
	int valeur;
	int ans;
	int son = 0; //Pour que le premier Worker puisse créer le second

	while(1)
	{
		myread(ws->prevWorker, &valeur, sizeof(int));

		if(valeur == -1 )
		{
			if(son != 0)
			{
				mywrite(ws->nextWorker, &valeur, sizeof(int));
				wait(NULL);
			}
			break;
		}
		if(valeur == (ws->primeNumber))
		{
			ans = 1; //Vrai : premier
			mywrite(ws->master,&ans,sizeof(int));
			mywrite(ws->master,&(ws->primeNumber),sizeof(int));
		}

		else if(valeur%(ws->primeNumber) == 0)
		{
			ans = 0; //Faux : non premier
			mywrite(ws->master,&ans,sizeof(int));
			mywrite(ws->master, &(ws->primeNumber), sizeof(int));
		}

		else{
			
			if(son == 0) //Si il n'a pas déjà créé un fils
			{				
				int fd[2];
				pipe(fd); //Création du pipe entre le worker et son fils
				
				son = fork(); //Création d'un fils
				
				if(son == 0) //Si fils
				{
					pizza(valeur, fd, ws);
				}

				else
				{
					myclose(fd[0]);
					ws->nextWorker = fd[1];
				}
			}

			else
			{
				mywrite(ws->nextWorker, &valeur, sizeof(int));
			}
		}
	}
}

/************************************************************************
 * Programme principal
 ************************************************************************/

int main(int argc, char * argv[])
{
	workerStats ws = malloc( sizeof( struct wS ) );

    parseArgs(argc, argv ,ws);
    
    //Si on est créé c'est qu'on est un nombre premier
    
    /*Envoyer au Master un message positif pour dire
	que le nombre testé est bien premier*/

    loop(ws);

    //Libérer les ressources : fermeture des files descriptors par exemple
    myclose(ws->master);
	myclose(ws->prevWorker);
	free(ws);

    return EXIT_SUCCESS;
}
