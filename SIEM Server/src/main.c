#include <stdio.h>
#include <stdlib.h>
#include <sys/inotify.h>
#include <mysql/mysql.h>
#include <time.h>
#include "harvester.h"
#include "parser.h"
#include <pthread.h>
#include <regex.h>
#include "heap.h"
#include <unistd.h>
#include <sys/types.h> 
#include <sys/socket.h>
#include <netinet/in.h>

#define EVENT_SIZE  	(sizeof (struct inotify_event))
#define EVENT_BUF_LEN   (1024*(EVENT_SIZE + 16))

pthread_mutex_t mutLogQueue, mutEmptyLogQueue, mutPriorityQueue, mutFullPriorityQueue, mutEmptyPriorityQueue;

typedef struct Log{
	char *path;
	char *type;
	char *host;
	char *log;
	time_t indexTime;
	time_t timestamp;
}Log;

typedef struct Pipeline{
	LogDATA *logdata;
	PQueue *logLines;
	MDATA *metadata;
}Pipeline;

char *getNextLine(char **string){
	int i=0;
	char *ch;
	for(ch = *string; *ch != '\n' && *ch != '\0'; ch++);
	i = ch - *string;
	if(!i) return NULL;
	char *a = (char *)malloc(i + 1);
	strncpy(a, *string, i);
	a[i] = '\0';
	*string = ch;
	return a;
}
void removeChar(char *str, char garbage) {
    char *src, *dst;
    for (src = dst = str; *src != '\0'; src++) {
        *dst = *src;
        if (*dst != garbage) dst++;
    }
    *dst = '\0';
}
int timestampCompare(const void *d1, const void *d2) {
    return ((LogDATA *)d1)->readTime - ((LogDATA *)d2)->readTime;
}
void * holderThread(Pipeline *p){
	PQueue *pq = p->logLines;
	MYSQL conexao;
	Log * logBuffer;
	char * query;
	int result;
	mysql_init(&conexao);
	const char * prequery = "INSERT INTO siem.events (time, host, type, path, log) VALUES (%ld, \"%s\", \"%s\", \"%s\", \"%s\");";
	if(mysql_real_connect(&conexao, "localhost", "siemadmin", "siemadmin", "siem", "3306", NULL, 0) ){
		printf("Conectado!\n");
	}else{
		printf("Falha da conexao\n");
		printf("Erro %d : %s\n", mysql_errno(&conexao), mysql_error(&conexao));
		return;
	}

	while(1){
	//	//printf("Holder: PQ: %d\n", pqueueSize(p->logLines));
		if(pqueue_isEmpty(pq)){
			//printf("Empty\n");
	//		printf("Lock\n");
			pthread_mutex_lock(&mutEmptyPriorityQueue);
		}else {
			printf("Lock\n");
			pthread_mutex_lock(&mutPriorityQueue);
			logBuffer = (Log*) pqueue_mindequeue(pq);
			printf("Unlock\n");
			pthread_mutex_unlock(&mutPriorityQueue);
			printf("Unlock\n");
			pthread_mutex_unlock(&mutFullPriorityQueue);
			printf("Unlock\n");
			pthread_mutex_unlock(&mutLogQueue);
				if(logBuffer->host != NULL) removeChar(logBuffer->host, '\"');
				if(logBuffer->type != NULL) removeChar(logBuffer->type, '\"');
				if(logBuffer->path != NULL) removeChar(logBuffer->path, '\"');
				if(logBuffer->log != NULL) removeChar(logBuffer->log, '\"');
			if(logBuffer != NULL && 0){
				if(prequery!=NULL) sprintf(query, prequery, logBuffer->timestamp, logBuffer->host, logBuffer->type, logBuffer->path, logBuffer->log);
				if(query!=NULL) result = mysql_query(&conexao, query);
			}
	//		//if (!result) printf("Registros inseridos %d\n", mysql_affected_rows(&conexao));
	//		if(result) printf("Erro na inserção %d : %s\n", mysql_errno(&conexao), mysql_error(&conexao));
	//		//printf("Holder: PQ: %d\n", pqueueSize(p->logLines));
//	//		free(logBuffer);
		}		
	}
}

void * parserThread(Pipeline *p){
	LogDATA *pBuffer;
	PQueue *priorityQueue;
	Log *logline;
	char *cPointer, *nextLine, *datetime;
	int timestamp;
	regex_t TimeRegEx, DateRegEx;
	regmatch_t matches[5];
	int start, end;
	char *extTime, *extDate;
	struct tm tm;
	time_t t;
	datetime = malloc(sizeof(char));
	datetime[0] = '\0';

	priorityQueue = p->logLines;
	
	int result = regcomp(&TimeRegEx, "([0-9]{2})\:([0-9]{2})\:([0-9]{2})(,[0-9]{3})?", REG_EXTENDED);
	if(result) return;
	
	result = regcomp(&DateRegEx, "([0-9]{2}|[0-9]{4})\/([0-9]{2})\/([0-9]{2}|[0-9]{4})|([0-9]{2}|[0-9]{4})-([0-9]{2})-([0-9]{2}|[0-9]{4})", REG_EXTENDED);
	if(result) printf("erro"); //return;

	while(1){
		//printf("Parser: PQ: %d\n", pqueueSize(priorityQueue));
		//printf("Parser: Q: %d\n", sizeofQueue(p->logdata));
		while(pqueue_isFull(priorityQueue)) sleep(1);
		if(!isEmpty(p->logdata)){
			printf("Lock\n");
			pthread_mutex_lock(&mutLogQueue);
			pBuffer = (LogDATA *) removeQueue(p->logdata);
			//printf("Parser: mutLogQueue: Unlock\n");
			printf("Unlock\n");
			pthread_mutex_unlock(&mutLogQueue);
			//printf("Parser: mutLogQueue: Unlocked\n");
			//printf("Timestamp: %ld\n", pBuffer->readTime);
			for(cPointer = pBuffer->log; *cPointer != '\0'; cPointer++){
				//printf("############################\n");
				nextLine = getNextLine(&cPointer);
				if(nextLine != NULL){
					//printf("3\n");
					result = regexec(&TimeRegEx, nextLine, 5, matches, 0);
					//printf("2\n");
					if(!result){
						start = matches[1].rm_so;
						end = matches[1].rm_eo;
						extTime = malloc(end - start + 1);
						strncpy(extTime, nextLine + start, end - start);
						extTime[end - start] = '\0';
						//printf("Extracted Time: %s", extTime);
						strcat(datetime, extTime);
						strcat(datetime, ":");
						free(extTime);
						
						start = matches[2].rm_so;
						end = matches[2].rm_eo;
						extTime = malloc(end - start + 1);
						strncpy(extTime, nextLine + start, end - start);
						extTime[end - start] = '\0';
						//printf(":%s:", extTime);
						strcat(datetime, extTime);
						strcat(datetime, ":");
						free(extTime);
						
						start = matches[3].rm_so;
						end = matches[3].rm_eo;
						extTime = malloc(end - start + 1);
						strncpy(extTime, nextLine + start, end - start);
						extTime[end - start] = '\0';
						//printf("%s\n", extTime);
						strcat(datetime, extTime);
						strcat(datetime, " ");
						free(extTime);
					}
					result = regexec(&DateRegEx, nextLine, 5, matches, 0);
					if(!result){
						start = matches[1].rm_so;
						end = matches[1].rm_eo;
						extDate = malloc(end - start + 1);
						strncpy(extDate, nextLine + start, end - start);
						extDate[end - start] = '\0';
						//printf("Extracted Date: %s/", extDate);
						strcat(datetime, extDate);
						strcat(datetime, "/");
						free(extDate);
						
						start = matches[2].rm_so;
						end = matches[2].rm_eo;
						extDate = malloc(end - start + 1);
						strncpy(extDate, nextLine + start, end - start);
						extDate[end - start] = '\0';
						//printf("%s/", extDate);
						strcat(datetime, extDate);
						strcat(datetime, "/");
						free(extDate);
						
						start = matches[3].rm_so;
						end = matches[3].rm_eo;
						extDate = malloc(end - start + 1);
						strncpy(extDate, nextLine + start, end - start);
						extDate[end - start] = '\0';
						//printf("%s\n", extDate);
						strcat(datetime, extDate);
						free(extDate);
					}
					//printf("%ld\n", pBuffer->readTime);
					t = pBuffer->readTime;
					if(datetime != NULL) 
						if(strptime(datetime, "%H:%M:%S %Y/%m/%d", &tm) != NULL){
							t = mktime(&tm);
						}
					//printf("%s - %d - %ld\n", datetime, strlen(datetime), t);
					datetime[0] = '\0';
					logline = malloc(sizeof(Log));
					logline->host = pBuffer->host;
					logline->type = pBuffer->sourcetype;
					logline->path = pBuffer->path;
					logline->log = nextLine;
					logline->timestamp = t;
					logline->indexTime = pBuffer->readTime;
					while(pqueue_isFull(priorityQueue)){
						printf("Lock\n");
						pthread_mutex_lock(&mutFullPriorityQueue);
						printf("Lock\n");
						pthread_mutex_lock(&mutLogQueue);
					}
					printf("Lock\n");
					pthread_mutex_lock(&mutPriorityQueue);
					pqueue_minenqueue(priorityQueue, logline);
					printf("Unlock\n");
					pthread_mutex_unlock(&mutPriorityQueue);
					printf("Unlock\n");
					pthread_mutex_unlock(&mutEmptyPriorityQueue);
					//printf("%s\n", logline->log);
				}
			}
		}else{
			printf("Lock\n");
			pthread_mutex_lock(&mutEmptyLogQueue);
		}
	}
	regfree(&TimeRegEx);
	regfree(&DateRegEx);
}

void * listenerThread(Pipeline *p){
	PQueue *priorityQueue = p->logLines;
	char *ACK = "ack!";
	int sockfd, newsockfd;
	socklen_t clilen;
	char buffer[2048];
	struct sockaddr_in serv_addr, cli_addr;
	int n;
	sockfd = socket(AF_INET, SOCK_STREAM, 0);
	
	if (sockfd < 0) printf("ERROR opening socket");
	
	bzero((char *) &serv_addr, sizeof(serv_addr));
	serv_addr.sin_family = AF_INET;
	serv_addr.sin_addr.s_addr = INADDR_ANY;
	serv_addr.sin_port = htons(9991);
	
	if (bind(sockfd, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) < 0) printf("ERROR on binding");
		
	while(1){
		//printf("Holder: PQ: %d\n", pqueueSize(p->logLines));
		//printf("1\n");
		listen(sockfd,5);
		clilen = sizeof(cli_addr);
	    newsockfd = accept(sockfd,(struct sockaddr *) &cli_addr, &clilen);
	    if (newsockfd < 0) printf("ERROR on accept");
	    bzero(buffer,2048);
	    n = read(newsockfd,buffer,2047);
	    if (n < 0) printf("ERROR reading from socket");
		printf("%s\n", buffer);
		while(pqueue_isFull(priorityQueue)){
			printf("Lock\n");
			pthread_mutex_lock(&mutFullPriorityQueue);
			printf("Lock\n");
			pthread_mutex_lock(&mutLogQueue);
		}
		printf("Lock\n");
		pthread_mutex_lock(&mutPriorityQueue);
		pqueue_minenqueue(priorityQueue, buffer);
		printf("Unlock\n");
		pthread_mutex_unlock(&mutPriorityQueue);
		printf("Unlock\n");
		pthread_mutex_unlock(&mutEmptyPriorityQueue);

	    n = write(newsockfd,ACK,strlen(ACK));
	    if (n < 0) printf("ERROR writing to socket");
	
	    close(newsockfd);
	}
	
	close(sockfd);
	
	return 0;
}

int main(){
	time_t seconds;
	Pipeline *pipe = malloc(sizeof(Pipeline));
	pipe->logdata = initQueue();
	
	pipe->metadata = populateMDATAQueue();
	MDATA *mBuffer;

	pipe->logLines = pqueue_new(timestampCompare, (size_t) 1024);

	char buffer[EVENT_BUF_LEN] __attribute__ ((aligned(8))), *p;
	struct inotify_event *event;
	int fd = inotify_init(), wd, length, i;

	pthread_t parser_id;
	int err = pthread_create(&parser_id, NULL, &parserThread, pipe);
	if (err != 0) printf("\ncan't create thread :[%s]", strerror(err));

	pthread_t holder_id;
	err = pthread_create(&holder_id, NULL, &holderThread, pipe);
	if (err != 0) printf("\ncan't create thread :[%s]", strerror(err));

	pthread_t listener_id;
	err = pthread_create(&listener_id, NULL, &listenerThread, pipe);
	if (err != 0) printf("\ncan't create thread :[%s]", strerror(err));
	
	for(i=0; i<sizeofQueue(pipe->metadata); i++){
		mBuffer = (MDATA *) removeQueue(pipe->metadata);
		//printf("%d\n", mBuffer->offset);
		wd = inotify_add_watch(fd, mBuffer->path, IN_MODIFY);
		insertQueue(pipe->metadata, mBuffer);
	}
	//printf("%ld\n", time(NULL));
	printf("Lock\n");
	pthread_mutex_lock(&mutLogQueue);
	dumpEvents(pipe->metadata, pipe->logdata);
	printf("Unlock\n");
	pthread_mutex_unlock(&mutLogQueue);
	//printf("%ld\n", time(NULL));
	//A fila não está mais vazia
	printf("Unlock\n");
	pthread_mutex_unlock(&mutEmptyLogQueue);
	while(1){
		//printf("Main: PQ: %d\n", pqueueSize(pipe->logLines));
		//printf("Main: Q: %d\n", sizeofQueue(pipe->logdata));
		length = read(fd, buffer, EVENT_BUF_LEN);
		//printf("%ld - File changed! %ld bytes read from event\n", time(NULL), (long)length);
		for (p = buffer; p < buffer + length; ) {
            event = (struct inotify_event *) p;
            //displayInotifyEvent(event);
            //printf("1\n");
            printf("Unlock\n");
            pthread_mutex_unlock(&mutLogQueue);
            printf("Lock\n");
            pthread_mutex_lock(&mutLogQueue);
        	dumpEvents(pipe->metadata, pipe->logdata);
        	printf("Unlock\n");
        	pthread_mutex_unlock(&mutLogQueue);
        	//printf("2\n");
        	//A fila não está mais vazia
        	printf("Unlock\n");
        	pthread_mutex_unlock(&mutEmptyLogQueue);
            p += sizeof(struct inotify_event) + event->len;
        }
	}

	closeFiles(pipe->metadata);
	freeQueue(pipe->metadata);
}