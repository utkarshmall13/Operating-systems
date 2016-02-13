#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <pthread.h>
#include <sys/time.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <signal.h>
#include <unistd.h>


void download(char* filename,char* ip,int port,int display){
	
	int sockfd, portno=port, n;
	struct sockaddr_in serv_addr;
	struct hostent *server;
	char buffer1[256];
	char* buffer=buffer1;

	//create socket, get sockfd handle
	sockfd = socket(AF_INET, SOCK_STREAM, 0);
	if (sockfd < 0) fprintf(stderr,"ERROR opening socket\n");
	//fill in server address in sockaddr_in datastructure
	server = gethostbyname(ip);
	if (server == NULL) {
		fprintf(stderr,"ERROR, no such host\n");
		exit(0);
	}
	bzero((char *) &serv_addr, sizeof(serv_addr));
	serv_addr.sin_family = AF_INET;
	bcopy((char *)server->h_addr, (char *)&serv_addr.sin_addr.s_addr,server->h_length);
	serv_addr.sin_port = htons(portno);

	//connect to server
	if(connect(sockfd,(struct sockaddr *)&serv_addr,sizeof(serv_addr)) < 0) fprintf(stderr,"ERROR connecting to server\n");

	//set file name
	bzero(buffer,256);
	buffer=filename;
	//send filename to server
	n = write(sockfd,buffer,strlen(buffer));
	if (n < 0) fprintf(stderr, "ERROR writing to socket\n");

	char buff[BUFSIZ];
	bzero(buff,BUFSIZ);

    int len;

	while (((len = read(sockfd, buff,sizeof(buff)-1)) > 0)){
		//fwrite(buff, sizeof(char), len, received_file);
		if(display!=0) printf("%s",buff);
		bzero(buff,BUFSIZ);
	}

	//fclose(received_file);
	close(sockfd);

}

int main(int argc, char **argv)
{
	if(argc!=5){
		fprintf(stderr,"ERROR: use \"./get-one-file <filename> <server-address> <port-number> <display/nodisplay>\"\n");
		return 1;
	}

	char* filename;
	filename=argv[1];
    char* ip;
    ip=argv[2];
    int port=atoi(argv[3]);
    int display=0;
    if(strcmp(argv[4],"display")==0) display=1;
	download(filename,ip,port,display);
}

