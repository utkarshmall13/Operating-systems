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
#include <errno.h>

char* filename;
int read_so_far=0;					// Global var to store data read so far. Useful when signal is received.

void sig_handler(int signo){
	if (signo == SIGINT)
		fprintf(stderr,"Received SIGINT; downloaded %d bytes of %s so far.\n",read_so_far,filename);			// print data read so far and exit.
	exit(1);
}


void download(char* filename,char* ip,int port,int display,int display1){
	
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
	
	char buff[BUFSIZ];
	//printf(filename);
	n = write(sockfd,filename,strlen(filename));
	if (n < 0) fprintf(stderr, "ERROR writing to socket\n");
    
    char fname[256];
	FILE * received_file;
	if(display1){
		fname[0]='f';fname[1]='i';fname[2]='l';fname[3]='e';fname[4]='s';fname[5]='/';
		int i=0;
		while(filename[i]!='\0'){	
			fname[i+6]=filename[i];
			i++;
		}
		fname[i+6]='\0';
		received_file = fopen(fname, "w");
	}
	bzero(buff,BUFSIZ);

    int len;

	while (((len = read(sockfd, buff,sizeof(buff)-1)) > 0)){
		if(display1)	fwrite(buff, sizeof(char), len, received_file);
		read_so_far+=len;										// Update data read so far.
		if(display!=0) printf("%s",buff);
		bzero(buff,BUFSIZ);
	}

	if(display1)	fclose(received_file);
	close(sockfd);

}

int main(int argc, char **argv)
{
	if(argc!=5){
		fprintf(stderr,"ERROR: use \"./get-one-file <filename> <server-address> <port-number> <display/nodisplay>\"\n");
		return 1;
	}
	
	if (signal(SIGINT, sig_handler) == SIG_ERR)	printf("\ncan't catch SIGINT\n");
	
	
	filename=argv[1];
    char* ip;
    ip=argv[2];
    int port=atoi(argv[3]);
    int display=0;
    if(strcmp(argv[4],"display")==0) display=1;
    int display1=0;
    //printf("%d\n",display1);
	download(filename,ip,port,display,display1);
	//fprintf(stderr,"Download finished for file : %s\n",filename);
	return 0;
}


