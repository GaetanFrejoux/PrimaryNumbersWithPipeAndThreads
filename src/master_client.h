#ifndef CLIENT_CRIBLE
#define CLIENT_CRIBLE

// On peut mettre ici des éléments propres au couple master/client :
//    - des constantes pour rendre plus lisible les comunications
//    - des fonctions communes (création tubes, écriture dans un tube,
//      manipulation de sémaphores, ...)


/*** === CONSTANTES === ***/

// ordres possibles pour le master
#define ORDER_NONE                0
#define ORDER_STOP               -1
#define ORDER_COMPUTE_PRIME       1
#define ORDER_HOW_MANY_PRIME      2
#define ORDER_HIGHEST_PRIME       3
#define ORDER_COMPUTE_PRIME_LOCAL 4   // ne concerne pas le master

// bref n'hésitez à mettre nombre de fonctions avec des noms explicites
// pour masquer l'implémentation


/*Identifiant pour le deuxième paramètre de ftok*/
#define PROJ_ID 1


/*** === FONCTIONS === ***/


//TUBES

int myopen(const char *pathname, int flags, mode_t mode);
ssize_t myread(int fd, void *buf, size_t count);
ssize_t mywrite(int fd, const void *buf, size_t count);

int mymkfifo(const char *pathname, mode_t mode);


//SÉMAPHORES

key_t getKey(const char *pathname, int proj_id);

int semCreator(key_t key);
int semGet(key_t key, int nsems, int semflg);

void semCtl(int semid, int semnum, int cmd);
void semSetVal(int semid, int val);
void semDestruct(int semid);

void prendre(int semid);
void vendre(int semid);
void attendre(int semid);


//THREADS

int mycreate(pthread_t *thread, const pthread_attr_t *attr, void *(*start_routine) (void *), void *arg);
int myjoin(pthread_t thread, void **retval);

#endif
