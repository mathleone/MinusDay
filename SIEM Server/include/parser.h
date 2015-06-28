#ifndef PARSER_H_INCLUDED
#define PARSER_H_INCLUDED

#include "queue.h"
#include <time.h>

typedef struct LogDATA{
	char *path;
	char *sourcetype;
	char *host;
	char *log;
	time_t readTime;
}LogDATA;



#endif