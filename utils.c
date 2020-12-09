#include <stdlib.h>
#include <stdio.h>

#include "myassert.h"

#include "utils.h"

#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/ipc.h>
#include <sys/sem.h>
#include <pthread.h>
#include <fcntl.h>

/*** === FONCTIONS === ***/


// === TUBES === //

/*Open a file*/
int myopen(const char *pathname, int flags)
{
	int ret = open(pathname, flags);
	myassert(ret > -1, "ERROR : The file didn't open");

	return ret;
}

/*Read in a file*/
void myread(int fd, void *buf, size_t count)
{
	int test = read(fd, buf, count);
	myassert(test > -1, "ERROR : No reading in the file");
}

/*Write in a file*/
void mywrite(int fd, const void *buf, size_t count)
{
	int test = write(fd, buf, count);
	myassert(test > -1, "ERROR : No writing in the file");
}

void myclose(int fd)
{
	int test = close(fd);
	myassert(test > -1, "ERROR : The file didn't close");
}

/*Create a named pipe*/
int mymkfifo(const char *pathname, mode_t mode)
{
	int ret = mkfifo(pathname, mode);
	myassert(ret > -1, "ERREUR : Le tube nommé n'a pas été créé");
	
	return ret;
}


// === SÉMAPHORES === //

/*Get the key*/
key_t getKey(const char *pathname, int proj_id)
{
	int key = ftok(pathname, proj_id);
	myassert(key > -1, "ERROR : Issue with the creation of the key");
	
	return key;
}

/*Create the semaphore*/
int semCreator(key_t key)
{
	int semid = semget(key, 1, IPC_CREAT | 0641);
	myassert(semid > -1, "ERROR : Issue when trying to create the semaphore");
	
	return semid;
}

/*Get the semaphore*/
int semGet(key_t key)
{
	int semid = semget(key, 1, 0);
	myassert(semid > -1, "ERROR : Issue with semGet");
	
	return semid;
}

/*Control the semaphore*/
void semCtl(int semid, int semnum, int cmd)
{
	int test = semctl(semid, semnum, cmd);
	myassert(test > -1, "ERROR : semCtl()");
}

/*Set a value to the semaphore*/
void semSetVal(int semid, int val){
	int test = semctl(semid, 0, SETVAL, val);
	myassert(test > -1,"ERROR : Issue when trying to set the value of the semaphore");
}

/*Destruct the semaphore*/
void semDestruct(int semid)
{
	int test = semctl(semid, 0, IPC_RMID);
	myassert(test > -1, "ERROR : Issue with the destruction of the semaphore");
} 

/*Sem lock*/
void prendre(int semid)
{
	struct sembuf op = {0, -1, 0};
	
	int test = semop(semid, &op, 1);
	myassert(test > -1, "ERROR : Semaphore didn't take his place");
}

/*Sem unlock*/
void vendre(int semid)
{
	struct sembuf op = {0, 1, 0};
	
	int test = semop(semid, &op, 1);
	myassert(test > -1, "ERROR : Semaphore didn't let his place");
}

/*Sem wait*/
void attendre(int semid)
{
	struct sembuf op = {0, 0, 0};
	
	int test = semop(semid, &op, 1);
	myassert(test > -1, "ERROR : Semaphore did not wait");
}


// === THREADS === //

/*Create the thread*/
int mycreate(pthread_t *thread, const pthread_attr_t *attr, void *(*start_routine) (void *), void *arg)
{
	int ret = pthread_create(thread, attr, start_routine, arg);
	myassert(ret > -1, "ERROR : Thread is not created");
	
	return ret;
}

int myjoin(pthread_t thread, void **retval)
{
	int ret = pthread_join(thread, retval);
	myassert(ret > -1, "ERROR : myjoin()");
	
	return ret;
}

//END
