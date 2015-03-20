/******************************************************************
* this file contains the macro DEBUG to print debug messages      *
* and we can turn it on/off in here                               *
*******************************************************************/

#include "stdio.h"

// debug on/off
#if 1
#define _ENABLE_DEBUG_
#endif

#ifdef _ENABLE_DEBUG_
#  define DEBUG(args ...) \
        do { \
            fprintf(stderr,"DEBUG:"); \
            fprintf(stderr, args); \
            fprintf(stderr,"\n"); \
        } while (0)
#else
#  define DEBUG(args ...) do { } while (0)
#endif













