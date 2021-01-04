#ifndef _INCLUDES_H_
#define _INCLUDES_H_

#include <assert.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <time.h>
#include <vector>

#ifdef WIN32
#include <windows.h>
#else
#include <dirent.h>
#include <pthread.h>
#include <sys/time.h>
#include <values.h>
#endif

#define MIN(a, b) ((a) < (b) ? (a) : (b))
#define MAX(a, b) ((a) > (b) ? (a) : (b))
#define ARRAY_SIZE(AA) (sizeof(AA)/sizeof(AA[0]))

#endif
