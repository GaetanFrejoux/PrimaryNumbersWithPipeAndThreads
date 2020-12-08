#if defined HAVE_CONFIG_H
#include "config.h"
#endif

#define _XOPEN_SOURCE

#include <stdlib.h>
#include <stdio.h>

#include "myassert.h"

#include "master_client.h"

/*** === FONCTIONS === ***/


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
