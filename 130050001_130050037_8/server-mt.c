#include <signal.h>
#include <stdio.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <string.h>
#include <sys/types.h>
#include <stdlib.h>
#include <netinet/in.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/sendfile.h>
#include <sys/wait.h>
#include <errno.h>
#include <queue>
#include <pthread.h>
#include <unistd.h>
#include <iostream>
using namespace std;

void sig_handler(int signo){
	if (signo == SIGPIPE);
		//cout<<"Connection dropped due to timeout"<<endl;
}

queue<int> requests;
int limit;
pthread_mutex_t queue_mutex=PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t queue_has_space=PTHREAD_COND_INITIALIZER;
pthread_cond_t queue_has_data=PTHREAD_COND_INITIALIZER;

void send_file(int newsockfd, char buffer[]){
	int n;
	char filename[strlen(buffer)];
	// get filename from arguments
    strncpy(filename,buffer,strlen(buffer));
    filename[strlen(buffer)]='\0';

    struct stat file_stat;
	int sent_bytes = 0;
    int offset;
    int remain_data;

	//Open file to read
	FILE* f=fopen(filename,"r");
	//Error in opening file
	if (f == NULL) {
        fprintf(stderr,"fopen failed, errno = %d\n", errno);
        return;
    }
	//Read the file and write over socket iteratively
	char file_buffer[BUFSIZ];
	while (1){
		// Read data into file_buffer. bytes_read is the total number of bytes read, as it may be less than BUFSIZ
		int bytes_read = fread(file_buffer, sizeof(char),BUFSIZ, f);
		//If file is totally read
		if (bytes_read == 0)break;
		if (bytes_read < 0) {
			fprintf(stderr,"Error in reading file from disk\n");
		}
		// We need a loop for the write, because not all of the data may be written in one call of socket write
		char *p = file_buffer;
		while (bytes_read > 0) {
			int bytes_written = write(newsockfd, file_buffer, bytes_read);
			if (bytes_written <= 0) {
				fprintf(stderr,"Error in writing to the socket\n");
				break;
			}
			bytes_read -= bytes_written;
			p += bytes_written;
		}
	}
	fclose(f);
}


void *myThread(void *vargp){
	while(1){
		
		pthread_mutex_lock(&queue_mutex);									/// Queue is locked
		                                                                    
			while(requests.size()==0)                                       /// Go to sleep if queue empty
				pthread_cond_wait(&queue_has_data,&queue_mutex);            
			                                                                
			int new1=requests.front();                                      /// pop the request for processing
			requests.pop();                                                 
			
			pthread_cond_signal(&queue_has_space);                          //  wakeup main thread notifying that queue has space
			                                                                
		pthread_mutex_unlock(&queue_mutex);                                 /// release the lock
			
		
		char buf[256]="get a";
		int n = read(new1,buf,255);
		if(n<0)	fprintf(stderr,"Error in reading filename over socket\n");
		buf[n]='\0';
		
		//removing "get " from front to get file name;
		for(int i=0;i<n-3;i++){
			buf[i]=buf[i+4];
		}
		printf("Request for \"%s\"\n",buf);
		send_file(new1,buf);
		close(new1);
	}
	int x=1;
	pthread_exit(&x);
}


int main(int argc, char *argv[])
{
	signal(SIGPIPE, sig_handler);
    struct sockaddr_in myaddr ,clientaddr;
	
	if (argc < 4) {
		fprintf(stderr,"ERROR, no port provided\n");
		return 1;
	}
	
	int N=atoi(argv[2]);
	limit=atoi(argv[3]);
	bool unbounded=false;
	if(limit==0) unbounded=true;
	
	
    int sockid,newsockid;
    sockid=socket(AF_INET,SOCK_STREAM,0);
    memset(&myaddr,'0',sizeof(myaddr));
    myaddr.sin_family=AF_INET;
    myaddr.sin_port=htons(atoi(argv[1]));
    myaddr.sin_addr.s_addr=INADDR_ANY;

    if(sockid==-1){
        fprintf(stderr,"Error in creating socket\n");
		return 1;
    }
    unsigned int len=sizeof(myaddr);
    if(bind(sockid,( struct sockaddr*)&myaddr,len)==-1){
		fprintf(stderr,"Error in binding to socket\n");
		return 1;
    }
    if(listen(sockid,10)==-1){
		fprintf(stderr,"Error in listening over socket\n");
		return 1;
    }

    int pid,new1,successful=0;
    static int counter=0;
    
    pthread_t tid[N];
	for(int i=0;i<N;i++){
		int created=pthread_create(&tid[i], NULL, myThread, (void *)0);
		if(created!=0){
			cout<<"Failed to create thread no. "<<i+1;
			if(created==11) cout<<" due to system limitations"<<endl;
			else cout<<endl;
		}
		successful++;
    }
	
    
	int flags = fcntl(sockid, F_GETFL, 0);
	fcntl(sockid, F_SETFL, flags | O_NONBLOCK);
    while(1){
        pthread_mutex_lock(&queue_mutex);										/// Queue is locked
        
			while(requests.size()>=limit && !unbounded)							/// Go to sleep if queue is filled upto limit
				pthread_cond_wait(&queue_has_space,&queue_mutex);
				
			new1 = accept(sockid, (struct sockaddr *)&clientaddr, &len);		/// Accept otherwise
			if(new1>0){
				requests.push(new1);											/// push 
				pthread_cond_signal(&queue_has_data);							// wakeup worker threads notifying that queue has data
			}
		pthread_mutex_unlock(&queue_mutex);										/// release the lock
		
	}
    
	for(int i=0;i<successful;i++){
		pthread_join(tid[i], NULL);
	}
    close(sockid);
    return 0;
}

