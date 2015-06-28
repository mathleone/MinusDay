#ifndef HARVESTER_H_INCLUDED
#define HARVESTER_H_INCLUDED
	
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/inotify.h>
#include "queue.h"
#include "parser.h"

typedef struct MDATA{
	int id;
	char *path;
	char *sourcetype;
	char *host;
	int offset;
	FILE *fp;
}MDATA;

//int isDir(const char *);
int exists(const char *);
MDATA *populateMDATAQueue();
//void dumpEvents(MDATA *, pthread_mutex_t *, LogDATA *, pthread_mutex_t *);
char *strdup(const char *);
void displayInotifyEvent(struct inotify_event *);

#endif