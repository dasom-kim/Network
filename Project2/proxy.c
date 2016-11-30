#include <stdio.h>
#include <unistd.h>
#include <pthread.h>
#include <stdlib.h>
#include <fcntl.h>
#include <time.h>
#include <netdb.h>
#include <string.h>
#include <arpa/inet.h>


#define MAX_REQUEST 2048
#define MAX_URL 2048
#define WEBPORT 80

struct info
{
   	int id;
   	int connfd;
    	char buffer[MAX_REQUEST];
    	char ip[20];
};

struct request
{
    	char request[MAX_REQUEST];
    	char method[16];
    	char path[MAX_URL];
   	char version[16];
   	char host[MAX_URL];
   	char page[MAX_URL];
};

int thread(struct info* Myinfo, int index, int sockfd); //return thread id
void *Connection(void *info);

FILE *fp_log; //share by threads
pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER; //initialize threads

int main(int argc, char** argv) {

    	int port = atoi(argv[1]);
    	int index_thread = 0;
	int sockfd;
	struct sockaddr_in serv_addr;
	
	sockfd = socket(PF_INET, SOCK_STREAM, 0);

	if(sockfd < 0){
		printf("Error open socket\n");
		exit(0);
	}

	memset(&serv_addr, 0, sizeof(serv_addr));

	serv_addr.sin_family = AF_INET;
	serv_addr.sin_addr.s_addr = INADDR_ANY;
	serv_addr.sin_port = htons(port);

	if(bind(sockfd, (struct sockaddr*)&serv_addr, sizeof(serv_addr)) == -1){
		printf("Error bind socket\n");
		exit(0);
	}

	//make threads (call "thread" function)

    	while (1)
    	{
		struct info* Myinfo;
		index_thread = thread(Myinfo, index_thread, sockfd);
   	}

    	close(sockfd);     
    	return 0;
}

int thread(struct info* Myinfo, int index, int sockfd)
{
	Myinfo = (struct info*) malloc(sizeof(struct info));

	index = index + 1; //add 1 to id

	int clnt_sock;
	socklen_t clnt_addr_size;
	struct sockaddr_in clnt_addr;
	char *str_temp;

	if(listen(sockfd, 5) == -1){
		printf("listen() error");
		exit(0);
	}

	clnt_addr_size = sizeof(clnt_addr);
	clnt_sock = accept(sockfd,(struct sockaddr*)&clnt_addr,&clnt_addr_size);

	if(clnt_sock == -1){
		printf("accept() error");
		exit(0);
	}

	Myinfo -> connfd = clnt_sock;
	
	getpeername(clnt_sock, (struct sockaddr *)&clnt_addr, &clnt_addr_size);
	str_temp = inet_ntoa(clnt_addr.sin_addr);
	strcpy(Myinfo -> ip, str_temp);

        pthread_t thread;

        Myinfo -> id = index;

	pthread_create(&thread, NULL, Connection, Myinfo); //call "Connection" function (create thread)
	pthread_detach(thread);

	return index;
}

void *Connection(void *info)
{

    	struct info* Myinfo = (struct info*) info;

   	struct request* Myreq;
	Myreq = (struct request*) malloc(sizeof(struct request));

	struct sockaddr_in serv_addr;
    	struct hostent *server;

  	int bytesRead = 0;
    	char *temp;
	int sign = 0;
	char portnumber[MAX_URL];
	char tempvalue[MAX_URL];
   	char host[MAX_URL], page[MAX_URL];
    	int chunkRead;
    	size_t fd;
    	char data[512];
    	char reqBuffer[512];
    	int totalBytesWritten = 0;
    	int chunkWritten = 0;
    	time_t result = time(NULL);
	struct tm* brokentime = localtime(&result);	    
    	int serv_sock;
	
    	memset(Myinfo -> buffer, 0, MAX_REQUEST);
    	bytesRead = read(Myinfo -> connfd, Myinfo -> buffer, sizeof(Myinfo -> buffer));
	strncpy(Myreq -> request, Myinfo -> buffer, MAX_REQUEST - 1);
	strncpy(Myreq -> method, strtok_r(Myinfo -> buffer, " ", &temp), 15);
	strncpy(Myreq -> path, strtok_r(NULL, " ", &temp), MAX_URL-1);
	strncpy(Myreq -> version, strtok_r(NULL, "\r\n", &temp), 15);
	sscanf(Myreq -> path, "http://%99[^/]%99[^\n]", host, page);
	strncpy(Myreq -> page, page, MAX_URL - 1);
	strncpy(Myreq -> host, host, MAX_URL - 1);

	if (bytesRead == 0) { //if it has no data, exit!
		exit(0);
	}

	if (strcmp(Myreq -> page, "") == 0) { //protect from segmentation fault..

		if (strcmp(host, "") != 0) {
			char *temphost;
			strcpy(tempvalue, host);
 			temphost = strtok(host, ":");

			if (strcmp(temphost, tempvalue) == 0) {
				exit(0);
			} else { //web port is not 80
				char *temphost;
				sign = 1;
				sscanf(Myreq -> path, "http://%99[^:]%99[^\n]", host, portnumber);	
			}
		} else {
			exit(0);
		}
	}

	printf("%s\n", Myinfo -> ip);
	printf("%d of thread created\n", Myinfo -> id);
    	printf("Read %d bytes!\n", bytesRead);
	printf("request : %s\n ", Myreq -> request);
	printf("method : %s\n ", Myreq -> method);
	printf("path : %s\n ", Myreq -> path);
	printf("version : %s\n ", Myreq -> version);
	
	if (sign == 0) {
		printf("page : %s\n ", Myreq -> page);
	}

	printf("host : %s\n", Myreq -> host);

	memset(reqBuffer, 0, 512);
	serv_sock = socket(AF_INET, SOCK_STREAM, 0);
	server = gethostbyname(host);

	memset((char *)&serv_addr, 0, sizeof(serv_addr));
	serv_addr.sin_family = AF_INET;
	memmove((char *)&serv_addr.sin_addr.s_addr,(char *)server->h_addr,server->h_length);

	if (sign == 0) {
		serv_addr.sin_port = htons(WEBPORT);
	} else {
		serv_addr.sin_port = htons(atoi(portnumber));
	}

	if (connect(serv_sock,(struct sockaddr *) &serv_addr,sizeof(serv_addr)) < 0){
		printf("Error Connecting\n");
		exit(0);
	}

	strcat(reqBuffer, Myreq -> method);
	strcat(reqBuffer," ");
	strcat(reqBuffer, Myreq -> page);
	strcat(reqBuffer," ");
	strcat(reqBuffer, Myreq -> version);
	strcat(reqBuffer,"\r\n");
	strcat(reqBuffer, "host: ");
	strcat(reqBuffer, host);
	strcat(reqBuffer, "\r\n");
	strcat(reqBuffer, "\r\n");
	printf("Request %s :\n%s\n %s\n", host, reqBuffer, asctime(brokentime));
	chunkRead = write(serv_sock, reqBuffer, strlen(reqBuffer));

	while ((chunkRead = read(serv_sock, data, sizeof(data)))!= (size_t)NULL)
	{
		chunkWritten = write(Myinfo -> connfd, data, chunkRead);
		totalBytesWritten += chunkWritten;
	}

    	pthread_mutex_lock(&mutex); //lock

    	fp_log = fopen("proxy.log", "a"); //write this data at "proxy.log"
    	fprintf(fp_log, "%s %s %s %d\n", asctime(brokentime), Myinfo -> ip, Myreq -> path, totalBytesWritten);
    	fclose(fp_log);

    	pthread_mutex_unlock(&mutex); //unlock

    	close(serv_sock);
    	close(Myinfo -> connfd);
    	pthread_exit(NULL);
}
