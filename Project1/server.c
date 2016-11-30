#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <stdlib.h>
#include <strings.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <fcntl.h>

void error(char *msg)
{
    perror(msg);
    exit(1);
}

int main(int argc, char *argv[])
{
     int sockfd, newsockfd;
     int portno;
     socklen_t clilen;

     char *body, *buf, *header_format, *header, *token;
     char contenttype[20], temp[10];
     int fsize, m;
     FILE *f;
  
     char buffer[1024];
     
     struct sockaddr_in serv_addr, cli_addr;
     
     int n;
     if (argc < 2) {
             fprintf(stderr,"ERROR, no port provided\n");
             exit(1);
     }
     
     sockfd = socket(AF_INET, SOCK_STREAM, 0);
     if (sockfd < 0) 
             error("ERROR opening socket");
     
     bzero((char *) &serv_addr, sizeof(serv_addr));
     portno = atoi(argv[1]);
     serv_addr.sin_family = AF_INET;
     serv_addr.sin_addr.s_addr = INADDR_ANY;
     serv_addr.sin_port = htons(portno);
     
     if (bind(sockfd, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) < 0)
     	     error("ERROR on binding");

     listen(sockfd,5);
     
     clilen = sizeof(cli_addr);

     while(1){

	     newsockfd = accept(sockfd, (struct sockaddr *) &cli_addr, &clilen);
	     if (newsockfd < 0) 
		  error("ERROR on accept");

	     bzero(buffer, 1024);
	     n = read(newsockfd, buffer, 1024); //buffer에 저장합니다.
	     
	     if (n < 0) 
		  error("ERROR reading from socket");
             
             printf("%s\n",buffer); //읽어들인 내용을 출력합니다.

	     token = (char *)malloc(30); //요청받은 파일명만을 추출하기 위한 보조 수단에 메모리를 할당합니다.
	     bzero(token, sizeof(token)); //token을 비워줍니다.
	     sscanf(buffer, "%s %s", temp, token); //buffer에서 token으로 읽어들입니다.
	     token += 1; //이때 '/'가 포함되므로 한 칸 뒤로 미뤄줍니다.
	     f = fopen(token, "r"); //순수 파일명 (ex) pg1513.txt)만을 fopen의 첫번째 변수로 넣어주고, 읽기 전용으로 파일을 엽니다.
	     fseek(f, 0, SEEK_END); //커서를 파일의 맨 끝에 위치시킵니다.
	     fsize = ftell(f); //커서가 있는 위치를 이용하여 파일의 사이즈를 알아냅니다.
	     fseek(f, 0, SEEK_SET); //파일의 시작 위치까지

	     buf = (char*)malloc(fsize+1); //메모리를 할당합니다.
	     header_format = "HTTP/1.1 200 OK\r\n" //헤더를 설정합니다.
	     "\r\n";

	     header = (char *)malloc(strlen(header_format) - 2 + strlen(contenttype) + 1); //메모리를 할당합니다.
	     sprintf(header, header_format, NULL); //헤더에 헤더 포맷을 입력합니다.

	     body = (char*)malloc(fsize+strlen(header)); //메모리를 할당합니다.
	     fsize = fread(buf, 1, fsize, f); //fsize의 사이즈를 가진 f를 buf에 읽어들입니다.
	     bzero(body, fsize+strlen(header)); //fsize와 헤더의 크기를 합친만큼의 사이즈를 가진 body를 비워줍니다.
	     strcat(body, header); //헤더를 body에 넣어줍니다.
	     memcpy(body+strlen(header), buf, fsize); //나머지 내용을 입력합니다.
	     m = write(newsockfd, body, fsize + strlen(header)); //서버의 내용을 입력합니다.

	     if (n < 0) 
		  error("ERROR writing to socket");

	     free(buf); //메모리를 해제합니다.
	     free(body); //메모리를 해제합니다.
	     fclose(f); //파일 open을 종료합니다.
	     close(newsockfd); //소켓을 종료합니다.

    }

    close(sockfd); //소켓을 종료합니다.
    return 0; 
}