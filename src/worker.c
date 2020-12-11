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
    ws->primeNumber = (int)strtol(argv[1],NULL,10); 
    ws->prevWorker = (int)strtol(argv[2],NULL,10);
	ws->nextWorker = 0;
    ws->master = (int)strtol(argv[3],NULL,10);
}

/************************************************************************
 * Fonction pour le fils
 ************************************************************************/

void sonProcess(int valeur, int fd[], workerStats ws)
{
	int tmp = 1;
	ws->primeNumber = valeur; //Prend la valeur actuelle;
	myclose(fd[1]); //Ferme la partie écriture
	ws->prevWorker = fd[0]; // Partie lecture
	mywrite(ws->master, &tmp, sizeof(int)); // renvoie true
	mywrite(ws->master, &(ws->primeNumber), sizeof(int)); // renvoie sa valeur
}

/************************************************************************
 * Boucle principale de traitement
 ************************************************************************/

void loop(workerStats ws)
{
	int valeur;
	int ans;
	int son = 0; //Pour que le premier Worker puisse créer le second

	while(true)
	{
		myread(ws->prevWorker, &valeur, sizeof(int));

		if (valeur == -1 )
		{
			if (son != 0) // S'il a un fils
			{
				mywrite(ws->nextWorker, &valeur, sizeof(int)); // Envoie le signal d'arrêt
				wait(NULL); // Attend que son fils se termine
			}
			break; // Quitte le while.
		}

		if (valeur == (ws->primeNumber))
		{
			ans = 1; //Vrai : premier car égal à un worker
			mywrite(ws->master,&ans,sizeof(int));
			mywrite(ws->master,&(ws->primeNumber),sizeof(int));
		}

		else if (valeur%(ws->primeNumber) == 0)
		{
			ans = 0; //Faux : non premier car divisible par le worker.
			mywrite(ws->master,&ans,sizeof(int));
			mywrite(ws->master, &(ws->primeNumber), sizeof(int));
		}

		else
		{
			//Si il n'a pas déjà créé un fils
			if (son == 0)
			{				
				int fd[2];
				mypipe(fd); //Création du pipe entre le worker et son fils
				
				son = fork(); //Création d'un fils

				//Si fils
				if (son == 0) {
					sonProcess(valeur, fd, ws);
				}

				else {
					myclose(fd[0]);
					ws->nextWorker = fd[1];
				}
			}

			else {
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
	workerStats ws = malloc( sizeof( struct wS ) ); // Alloc de l'espace pour la structure du Worker

    parseArgs(argc, argv ,ws);

    loop(ws); //  Boucle

    //Libérer les ressources : fermeture des files descriptors par exemple

    myclose(ws->master); // Fermeture du tube ver le master
	myclose(ws->prevWorker); // Fermeture du tube vers le précèdent

	if(ws->nextWorker!=0){ // Si ce n'est pas le dernier worker, on ferme le tube vers le suivant.
		myclose(ws->nextWorker);
	}

	free(ws); // Libère l'espace allouer pour la structure

    return EXIT_SUCCESS;
}
