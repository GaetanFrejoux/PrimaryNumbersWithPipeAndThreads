#ifndef CONFIG_H
#define CONFIG_H

// uncomment to use verbose mode
//#define VERBOSE

#ifdef VERBOSE
    #define TRACE(x) fprintf(stderr, (x));
    #define TRACE2(x,p1) fprintf(stderr, (x), (p1));
#else
    #define TRACE(x)
    #define TRACE2(x,p1)
#endif



#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/ipc.h>
#include <sys/sem.h>
#include <pthread.h>
#include <fcntl.h>

#endif
