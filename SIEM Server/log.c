#include <stdio.h>
#include <stdlib.h>
#include <sys/inotify.h>
#include <mysql/mysql.h>

#define EVENT_SIZE  ( sizeof (struct inotify_event) )
#define EVENT_BUF_LEN     ( 1024 * ( EVENT_SIZE + 16 ) )
int exists(const char *fname){
    FILE *file;
    if (file = fopen(fname, "r"))    {
        fclose(file);
        return 1;
    }
    return 0;
}
int main(){
	FILE *fp;
	char * line = NULL;
	size_t len = 0;
	if(exists("files.conf")){
		fp = fopen("files.conf", "r");
	}else{
		printf("Especifique os arquivos de coleta em files.conf\n");
		return 0;
	}

	while( getline(&line, &len, fp) != -1 ){
		printf("%s", line);
	}

	fclose(fp);
	if(line) free(line);
}