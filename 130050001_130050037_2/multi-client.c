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

//Global variables to save stats of server
int requests=0;
float tot_resp_time=0;

//Argument frame for the thread
struct argum{
	int portno;
	char* ip;
	int random;
	float wait;
	int duration;
};

//calculate time difference between two time values in seconds(floating point)
float time_diff(struct timeval t0, struct timeval t1)
{
    return (t1.tv_sec - t0.tv_sec) * 1000.0f + (t1.tv_usec - t0.tv_usec) / 1000.0f;
}

//Multi client thread function
void *clientThread(void *vargp)
{
	struct argum *ar=(struct argum *)vargp;

    //t0 is time value before getting into loop
	struct timeval t0;
    //t1 is for stroing total time for which loop has run
	struct timeval t1;
	gettimeofday(&t0, 0);
	while(1){
        //if elapsed_time>total_time: break
		gettimeofday(&t1, 0);
		float elapsed=time_diff(t0,t1);
		if(elapsed>ar->duration*1000) break;

		int sockfd, portno=ar->portno, n;
		struct sockaddr_in serv_addr;
		struct hostent *server;
		char buffer[256];


		//create socket, get sockfd handle
		sockfd = socket(AF_INET, SOCK_STREAM, 0);
		if (sockfd < 0) fprintf(stderr,"ERROR opening socket\n");
		//fill in server address in sockaddr_in datastructure
		server = gethostbyname(ar->ip);
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
		if(ar->random==1){
			snprintf(buffer,sizeof(buffer),"get files/foo%d.txt",rand()%1000);
		}
		else strcpy(buffer,"get files/foo1.txt");

		//send filename to server
		n = write(sockfd,buffer,strlen(buffer));
		if (n < 0) fprintf(stderr, "ERROR writing to socket\n");

        //(un)comment 84-91 and 104 and 114 line to start/stop creating file at client
		//Note: for this client executable must be in different directory
        //filename for writing/creating
		//char filename[256];
		//bzero(filename,256);
		//strcpy(filename,buffer);
		//filename[strlen(buffer)]='\0';
		//strcpy(filename,filename+4);

        ////open the file
		//FILE * received_file = fopen(filename, "w");

		char buff[BUFSIZ];
		bzero(buff,BUFSIZ);

        int len;

        //time in handling each Request t2-t3
		struct timeval t2;
		struct timeval t3;

		gettimeofday(&t2, 0);
		while (((len = read(sockfd, buff,sizeof(buff)-1)) > 0)){
			//fwrite(buff, sizeof(char), len, received_file);
			bzero(buff,BUFSIZ);
		}
		gettimeofday(&t3, 0);

        //stats modification
		float elapsed1=time_diff(t2,t3);
		requests++;
		tot_resp_time+=elapsed1;

		//fclose(received_file);
		close(sockfd);
		sleep(ar->wait);
	}

}

int main(int argc, char **argv)
{
	if(argc!=7){
		fprintf(stderr,"ERROR: use \"./multi-client <server-address> <port-number> <users> <time> <wait> <random/fixed>\"\n");
		return 1;
	}

	struct argum ar;
    ar.ip=malloc(256);
    ar.ip=argv[1];
    ar.portno=atoi(argv[2]);
    ar.duration=atoi(argv[4]);
    ar.wait=atoi(argv[5]);

	if(strcmp(argv[6],"random")==0) ar.random=1;
	else if(strcmp(argv[6],"fixed")==0) ar.random=0;
	else {
		fprintf(stderr,"Error file access is either \"random\" or \"fixed\"\n");
		return 1;
	}

    int i,total_users;
    total_users=atoi(argv[3]);
    //make n threads for n users
    pthread_t tid[total_users];
    for(i=0;i<total_users;i++){
		pthread_create(&tid[i], NULL, clientThread, (void *)&ar);
    }
    for(i=0;i<total_users;i++){
        pthread_join(tid[i],NULL);
    }
    printf("Done!\nthroughput = %f req/s\naverage response time = %f sec\n",((float)requests)/atoi(argv[4]),tot_resp_time/requests/1000);
    exit(0);
}
