#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h> 
#include <pthread.h>
#include <sys/time.h>
#include <unistd.h>
#include <iostream>
using namespace std;

int N=10,K=10000,count=0,locked=0;

float time_diff(struct timeval t0, struct timeval t1){
	
    return (t1.tv_sec - t0.tv_sec) * 1000.0f + (t1.tv_usec - t0.tv_usec) / 1000.0f;
}
	
void *myThread(void *vargp){
	
	for(int i=0;i<K;i++){
		while(locked);
		locked=1;
		count++;
		locked=0;
	}
}
  
int main(int argc, char **argv){
	
    pthread_t tid[N];
    
    struct timeval t0;
	struct timeval t1;
	gettimeofday(&t0, 0);
    for(int i=0;i<N;i++){
		pthread_create(&tid[i], NULL, myThread, (void *)0);
    }
    
    for(int i=0;i<N;i++){
		pthread_join(tid[i], NULL);
    }
    
    gettimeofday(&t1, 0);
	float elapsed=time_diff(t0,t1);
	cout<<"Count = "<<count<<endl<<"Time = "<<elapsed<<endl;
	return 0;
}
