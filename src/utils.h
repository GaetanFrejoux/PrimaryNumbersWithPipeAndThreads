#ifndef _UTILS_H_
#define _UTILS_H_


/*** === INCLUDE === ***/
 
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/ipc.h>
#include <sys/sem.h>
#include <sys/wait.h>
#include <pthread.h>
#include <fcntl.h>


/*** === FONCTIONS === ***/


//TUBES

int myopen(const char *pathname, int flags);
void myread(int fd, void *buf, size_t count);
void mywrite(int fd, const void *buf, size_t count);
void myclose(int fd);

int mymkfifo(const char *pathname, mode_t mode);


//SÉMAPHORES

key_t getKey(const char *pathname, int proj_id);

int semCreator(key_t key);
int semGet(key_t key);

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
