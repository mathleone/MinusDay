#include "harvester.h"

int exists(const char *fname){
    FILE *file;
    if (file = fopen(fname, "r")){
        fclose(file);
        return 1;
    }
    return 0;
}

//int isDir(const char *fname){
//	DIR* dir = opendir("mydir");
//	if (dir){
//	    closedir(dir);
//	    return 1;
//	}
//	else if (ENOENT == errno) return 0;
//	else return 0;
//}

/*int useQueue(){
	Data *y = (Data *)initQueue();
	Data x;
	x.x = 'h';
	insertQueue(y, x);
	x.x = 'e';
	insertQueue(y, x);
	x.x = 'l';
	insertQueue(y, x);
	x.x = 'l';
	insertQueue(y, x);
	x.x = 'o';
	insertQueue(y, x);
	while(!isEmpty(y))
		printf("%c", removeQueue(y));
	printf("\n");
	return 1;
}
*/

//void displayInotifyEvent(struct inotify_event *i){
//    printf("wd =%2d; ", i->wd);
//    if (i->cookie > 0)
//        printf("cookie =%4d; ", i->cookie);
//    printf("mask = ");
//    if (i->mask & IN_ACCESS)        printf("IN_ACCESS ");
//    if (i->mask & IN_ATTRIB)        printf("IN_ATTRIB ");
//    if (i->mask & IN_CLOSE_NOWRITE) printf("IN_CLOSE_NOWRITE ");
//    if (i->mask & IN_CLOSE_WRITE)   printf("IN_CLOSE_WRITE ");
//    if (i->mask & IN_CREATE)        printf("IN_CREATE ");
//    if (i->mask & IN_DELETE)        printf("IN_DELETE ");
//    if (i->mask & IN_DELETE_SELF)   printf("IN_DELETE_SELF ");
//    if (i->mask & IN_IGNORED)       printf("IN_IGNORED ");
//    if (i->mask & IN_ISDIR)         printf("IN_ISDIR ");
//    if (i->mask & IN_MODIFY)        printf("IN_MODIFY ");
//    if (i->mask & IN_MOVE_SELF)     printf("IN_MOVE_SELF ");
//    if (i->mask & IN_MOVED_FROM)    printf("IN_MOVED_FROM ");
//    if (i->mask & IN_MOVED_TO)      printf("IN_MOVED_TO ");
//    if (i->mask & IN_OPEN)          printf("IN_OPEN ");
//    if (i->mask & IN_Q_OVERFLOW)    printf("IN_Q_OVERFLOW ");
//    if (i->mask & IN_UNMOUNT)       printf("IN_UNMOUNT ");
//    printf(" --- \n");
//    if (i->len > 0) printf("name = %s\n", i->name);
//}

char *strdup(const char *s){
	size_t len = 1+strlen(s);
	char *p = malloc(len);
	return p ? memcpy(p, s, len) : NULL;
}

MDATA *populateMDATAQueue(){
	FILE *fp;
	char * line = NULL, sourcetype[128], path[128];
	size_t len = 0;
	int length, offset;

	MDATA *MDATAQueue = (MDATA *)initQueue();
	MDATA *MDATANode = malloc(sizeof(MDATA));

	if(exists("files.conf")){
		fp = fopen("files.conf", "r");
	}else{
		printf("Especifique os arquivos de coleta em files.conf\n");
		return 0;
	}

	//while( (length = getline(&line, &len, fp)) != -1 ){
	int i=0;
	while(EOF != fscanf(fp, "%d %s %s", &offset, &sourcetype, &path) ){
		//if(length>1){
			//printf("%s\n", sourcetype);
			//line[strlen(line)-1] = '\0';
			MDATANode = malloc(sizeof(MDATA));
			char hostname[1024];
			hostname[1023] = '\0';
			gethostname(hostname, 1023);
			MDATANode->host = hostname;
			MDATANode->path = strdup(path);
			//printf("%s\n", MDATANode->path);
			MDATANode->sourcetype = strdup(sourcetype);
			MDATANode->offset = offset;
			//MDATANode->fp = fopen(MDATANode->path, "r");

			insertQueue(MDATAQueue, MDATANode);
			MDATANode = NULL;
		//}
	}

	fclose(fp);
	if(line) free(line);
	return MDATAQueue;
}

void dumpEvents(MDATA *m, LogDATA *l, pthread_mutex_t *mut){
	MDATA *mBuffer;
	LogDATA *lBuffer;
	int i;
	long offset;
	for(i=0; i<sizeofQueue(m); i++){
		mBuffer = (MDATA *) removeQueue(m);
		if(exists(mBuffer->path)) {

			mBuffer->fp = fopen(mBuffer->path, "r");
			
			fseek(mBuffer->fp, 0, SEEK_END);
			offset = ftell(mBuffer->fp) - mBuffer->offset;
			if(offset){
				fseek(mBuffer->fp, -offset, SEEK_END);
				char * buffer = malloc (sizeof(char)*offset);
				fread(buffer, 1, offset, mBuffer->fp);
				mBuffer->offset = ftell(mBuffer->fp);
				if(buffer != NULL){
					//printf("Host: %s - ", mBuffer->host);
					//printf("Path: %s - ", mBuffer->path);
					//printf("Type: %s - ", mBuffer->sourcetype);
					//printf("Offset: %d\n", mBuffer->offset);
					//printf("%d\n", offset);
					//printf("%s", buffer);
					lBuffer = malloc(sizeof(LogDATA));

					lBuffer->host = mBuffer->host;

					lBuffer->path = mBuffer->path;

					lBuffer->sourcetype = mBuffer->sourcetype;

					lBuffer->readTime = time(NULL);
					//lBuffer->log = malloc(strlen(buffer)+1);
					//strcpy(lBuffer->log, buffer);

					lBuffer->log = buffer;

					insertQueue(&(*l), lBuffer);
				} 	
			}

			fclose(mBuffer->fp);
		}
		insertQueue(m, mBuffer);
	}
}

void closeFiles(MDATA *q){
	MDATA *mBuffer;
	int i;
	for(i=0; i<sizeofQueue(q); i++){
		mBuffer = (MDATA *) removeQueue(q);
		fclose(mBuffer->fp);
	}
}

