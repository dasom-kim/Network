#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h> 

int connectTo(char* host, int portno) {
    int sockfd;
    struct sockaddr_in serv_addr;
    struct hostent *server;    
    
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    server = gethostbyname(host);
    
    if (server == NULL) {
        printf("No such host\n");
        exit(0);
    }    
    
    bzero((char *) &serv_addr, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    bcopy((char *)server->h_addr, 
         (char *)&serv_addr.sin_addr.s_addr,
         server->h_length);
    serv_addr.sin_port = htons(portno);
    if (connect(sockfd,(struct sockaddr *) &serv_addr,sizeof(serv_addr)) < 0)  {
        printf("Error Connecting\n");
        exit(0);
    }
        
    return sockfd;
}

int makeListener(int portno) {
     int sockfd;
     struct sockaddr_in serv_addr;
     
     sockfd = socket(AF_INET, SOCK_STREAM, 0);
     if (sockfd < 0) {
        printf("ERROR opening socket\n");
        exit(0);
     }
     bzero((char *) &serv_addr, sizeof(serv_addr));
     serv_addr.sin_family = AF_INET;
     serv_addr.sin_addr.s_addr = INADDR_ANY;
     serv_addr.sin_port = htons(portno);
     if (bind(sockfd, (struct sockaddr *) &serv_addr,
              sizeof(serv_addr)) < 0) {
              printf("ERROR on binding\n");
              exit(0);
     }
              
     return sockfd;
}

int listenFor(int sockfd) {
    int clilen, newsockfd;
    struct sockaddr_in cli_addr;
    
	listen(sockfd,5);
	
     clilen = sizeof(cli_addr);
     
     newsockfd = accept(sockfd, 
                 (struct sockaddr *) &cli_addr, 
                 &clilen);
     if (newsockfd < 0) 
          printf("ERROR on accept\n");	
          
     return newsockfd;
}
