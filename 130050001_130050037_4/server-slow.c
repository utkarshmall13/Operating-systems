#include <unistd.h>
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
		void *p = file_buffer;
		while (bytes_read > 0) {
			int bytes_written = write(newsockfd, file_buffer, bytes_read);
			sleep(1);																		/// SLEEP 1 sec after every data packet sent.
			if (bytes_written <= 0) {
				fprintf(stderr,"Error in writing to the socket\n");
			}
			bytes_read -= bytes_written;
			p += bytes_written;
		}
	}
	fclose(f);
}

int main(int argc, char *argv[])
{
    struct sockaddr_in myaddr ,clientaddr;

	if (argc < 2) {
		fprintf(stderr,"ERROR, no port provided\n");
		return 1;
	}

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
    int len=sizeof(myaddr);
    if(bind(sockid,( struct sockaddr*)&myaddr,len)==-1){
		fprintf(stderr,"Error in binding to socket\n");
		return 1;
    }
    if(listen(sockid,1000)==-1){
		fprintf(stderr,"Error in listening over socket\n");
		return 1;
    }

    int pid,new;
    static int counter=0;

	int flags = fcntl(sockid, F_GETFL, 0);
	fcntl(sockid, F_SETFL, flags | O_NONBLOCK);
    while(1){
        new = accept(sockid, (struct sockaddr *)&clientaddr, &len);
		if(new < 0){
			//Reaping zombie processes using non blocking wait call
			while ( (pid = waitpid((pid_t)(-1), 0, WNOHANG)) > 0) {}
			continue;
	    }
		//forking main process
        if ((pid = fork()) == -1){
			fprintf(stderr,"Error in forking parent process\n");
			//Reaping zombie processes using non blocking wait call
        	close(new);
            continue;
        }
        else if(pid > 0){
            close(new);
			//Reaping zombie processes using non blocking wait call
			while ( (pid = waitpid((pid_t)(-1), 0, WNOHANG)) > 0) {}
            continue;
        }
        else if(pid == 0){
			//In child process
            char buf[100];
            int n = read(new,buf,255);
			if(n<0)fprintf(stderr,"Error in reading filename over socket\n");
            buf[n]='\0';
            int i;
			printf("Request for \"%s\"\n",buf);
			send_file(new,buf);
            close(new);
            break;
        }
	}
    close(sockid);
    return 0;
}
