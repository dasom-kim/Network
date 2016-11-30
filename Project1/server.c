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
	     n = read(newsockfd, buffer, 1024); //buffer�� �����մϴ�.
	     
	     if (n < 0) 
		  error("ERROR reading from socket");
             
             printf("%s\n",buffer); //�о���� ������ ����մϴ�.

	     token = (char *)malloc(30); //��û���� ���ϸ��� �����ϱ� ���� ���� ���ܿ� �޸𸮸� �Ҵ��մϴ�.
	     bzero(token, sizeof(token)); //token�� ����ݴϴ�.
	     sscanf(buffer, "%s %s", temp, token); //buffer���� token���� �о���Դϴ�.
	     token += 1; //�̶� '/'�� ���ԵǹǷ� �� ĭ �ڷ� �̷��ݴϴ�.
	     f = fopen(token, "r"); //���� ���ϸ� (ex) pg1513.txt)���� fopen�� ù��° ������ �־��ְ�, �б� �������� ������ ���ϴ�.
	     fseek(f, 0, SEEK_END); //Ŀ���� ������ �� ���� ��ġ��ŵ�ϴ�.
	     fsize = ftell(f); //Ŀ���� �ִ� ��ġ�� �̿��Ͽ� ������ ����� �˾Ƴ��ϴ�.
	     fseek(f, 0, SEEK_SET); //������ ���� ��ġ����

	     buf = (char*)malloc(fsize+1); //�޸𸮸� �Ҵ��մϴ�.
	     header_format = "HTTP/1.1 200 OK\r\n" //����� �����մϴ�.
	     "\r\n";

	     header = (char *)malloc(strlen(header_format) - 2 + strlen(contenttype) + 1); //�޸𸮸� �Ҵ��մϴ�.
	     sprintf(header, header_format, NULL); //����� ��� ������ �Է��մϴ�.

	     body = (char*)malloc(fsize+strlen(header)); //�޸𸮸� �Ҵ��մϴ�.
	     fsize = fread(buf, 1, fsize, f); //fsize�� ����� ���� f�� buf�� �о���Դϴ�.
	     bzero(body, fsize+strlen(header)); //fsize�� ����� ũ�⸦ ��ģ��ŭ�� ����� ���� body�� ����ݴϴ�.
	     strcat(body, header); //����� body�� �־��ݴϴ�.
	     memcpy(body+strlen(header), buf, fsize); //������ ������ �Է��մϴ�.
	     m = write(newsockfd, body, fsize + strlen(header)); //������ ������ �Է��մϴ�.

	     if (n < 0) 
		  error("ERROR writing to socket");

	     free(buf); //�޸𸮸� �����մϴ�.
	     free(body); //�޸𸮸� �����մϴ�.
	     fclose(f); //���� open�� �����մϴ�.
	     close(newsockfd); //������ �����մϴ�.

    }

    close(sockfd); //������ �����մϴ�.
    return 0; 
}