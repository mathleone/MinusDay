#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h> 
#include <sys/socket.h>
#include <netinet/in.h>

#define portno 9991
const char *ACK = "Muito prazer, kivia. Voce eh linda!";

int main(int argc, char *argv[]){
    int sockfd, newsockfd;
    socklen_t clilen;
    char buffer[256];
    struct sockaddr_in serv_addr, cli_addr;
    int n;
    
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    
    if (sockfd < 0) printf("ERROR opening socket");
    
    bzero((char *) &serv_addr, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = INADDR_ANY;
    serv_addr.sin_port = htons(portno);
    
    if (bind(sockfd, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) < 0) printf("ERROR on binding");
    
    listen(sockfd,5);
    clilen = sizeof(cli_addr);
    
    while(1){
        newsockfd = accept(sockfd,(struct sockaddr *) &cli_addr, &clilen);
        if (newsockfd < 0) printf("ERROR on accept");
        bzero(buffer,256);
        n = read(newsockfd,buffer,255);
        if (n < 0) printf("ERROR reading from socket");
        printf("Here is the message: %s\n",buffer);

        n = write(newsockfd,ACK,strlen(ACK));
        if (n < 0) printf("ERROR writing to socket");
    
        close(newsockfd);
    }

    close(sockfd);

    return 0; 
}