#include <sys/mman.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdlib.h>
#include <stdio.h>
#include <sys/time.h>
#include <string.h>

int BLOCK_SIZE=512;

int min(int a,int b){
    if(a<b)return a;
    return b;
}

//calculate time difference between two time values in seconds(floating point)
float time_diff(struct timeval t0, struct timeval t1){
    return (t1.tv_sec - t0.tv_sec) * 1000.0f + (t1.tv_usec - t0.tv_usec) / 1000.0f;
}

float mmapped_readwrite(int filecount,int rwflag){
    int i,fddata[filecount],file_size[filecount];
    struct stat sb;
    char* filepointer[filecount];
    struct timeval ti,tf;

    char filename[255];
    for(i=0;i<filecount;i++){
        snprintf(filename,sizeof(filename),"files/foo%d.txt",i+1);
        fddata[i] = open(filename, O_RDWR);
        if (fddata[i] == -1) {
            exit(1);
        }
        if (fstat(fddata[i], &sb) == -1) exit(1);
        file_size[i]=sb.st_size;
    }

    for(i=0;i<filecount;i++){
        filepointer[i] = mmap(NULL, file_size[i], PROT_READ | PROT_WRITE, MAP_SHARED, fddata[i], 0);
        if (filepointer[i] == MAP_FAILED){
            exit(1);
        }
    }

    //read mmapped file
    gettimeofday(&ti, 0);
    char buffer[BLOCK_SIZE];
    int read_bytes=0;

    for(i=0;i<filecount;i++){
        /*int offsetl=0;
        int remaining_data=file_size[i];
        while(remaining_data>0){
            int sizereadwrite=min(sizeof(buffer),remaining_data);
            strncpy(buffer,filepointer[i]+offsetl,sizereadwrite);
            if(rwflag!=0)strncpy(filepointer[i]+offsetl,buffer,sizereadwrite);
            //printf("%p\n",filepointer[i]);
            remaining_data-=sizereadwrite;
            offsetl+=sizereadwrite;
            read_bytes+=sizereadwrite;
        }*/
        int offset=0;
        char bit;
        while(offset<file_size[i]){
            bit=filepointer[i][offset];
            if(rwflag!=0)filepointer[i][offset]=bit;
            offset++;
            read_bytes++;
        }
    }

    for(i=0;i<filecount;i++){
        if(rwflag!=0){
            if (msync(filepointer[i], file_size[i], MS_SYNC)){
                perror("Could not sync the file to disk");
            }
        }
        if(munmap(filepointer[i], file_size[i]) == -1) {
            perror("Error un-mmapping the file");
        }
        close(fddata[i]);
    }
    gettimeofday(&tf, 0);
    float MBs=(read_bytes*1.0)/1000000;
    float timesec=time_diff(ti,tf)/1000;

    return MBs/timesec;
}

float disk_readwrite(int filecount,int rwflag){
    int i,fddatain[filecount],fddataout[filecount],file_size[filecount];
    struct stat sb;
    char* readpointer[filecount];
    struct timeval ti,tf;

    char filename[255];
    for(i=1;i<=filecount;i++){
        snprintf(filename,sizeof(filename),"files/foo%d.txt",i);
        fddatain[i] = open(filename, O_RDONLY);
        fddataout[i] = open(filename, O_WRONLY);
        if (fddatain[i] == -1) exit(1);
        if (fddataout[i] == -1) exit(1);
        if (fstat(fddatain[i], &sb) == -1) exit(1);
        file_size[i]=sb.st_size;
    }

    char buffer[BLOCK_SIZE];
    int read_bytes=0;
    int write_bytes=0;
    gettimeofday(&ti, 0);
    for(i=1;i<=filecount;i++){
        int offset=0;
        while(offset<file_size[i]){
            int readonce;
            readonce=read(fddatain[i],buffer,sizeof(buffer));
            read_bytes+=readonce;
            if(rwflag!=0)write_bytes+=write(fddataout[i],buffer,readonce);
            offset+=BLOCK_SIZE;
        }
    }
    gettimeofday(&tf, 0);

    for(i=1;i<=filecount;i++){
        close(fddatain[i]);
        close(fddataout[i]);
    }

    float MBs=(read_bytes*1.0)/1000000;
    float timesec=time_diff(ti,tf)/1000;

    return MBs/timesec;
}

int main(int argc, char *argv[]){
    if(argc!=5){
        printf("correct format: ./experimentB <read/write> <BLOCK_SIZE> <mmap/disk> <filecount>\n");
        exit(1);
    }
    if(!(strcmp(argv[1],"read")==0||strcmp(argv[1],"write")==0)){
        printf("correct format: ./experimentB <read/write> <BLOCK_SIZE> <mmap/disk> <filecount>\n");
        printf("use 'read' or 'write' only\n");
        exit(1);
    }
    if(atoi(argv[2])<=0||atoi(argv[2])%512!=0){
        printf("correct format: ./experimentB <read/write> <BLOCK_SIZE> <mmap/disk> <filecount>\n");
        printf("BLOCK_SIZE must be multiple of 512\n");
        exit(1);
    }
    if(!(strcmp(argv[3],"mmap")==0||strcmp(argv[3],"disk")==0)){
        printf("correct format: ./experimentB <read/write> <BLOCK_SIZE> <mmap/disk> <filecount>\n");
        printf("use 'mmap' or 'disk' only\n");
        exit(1);
    }
    if(atoi(argv[4])<=0||atoi(argv[4])>25){
        printf("correct format: ./experimentB <read/write> <BLOCK_SIZE> <mmap/disk> <filecount>\n");
        printf("number of files between 1 and 25\n");
        exit(1);
    }
    BLOCK_SIZE=atoi(argv[2]);
    //BLOCK_SIZE=2;
    int filecount=atoi(argv[4]);
    if(strcmp(argv[1],"read")==0&&strcmp(argv[3],"mmap")==0){
        printf("average throughput for mmap read: %f MB/s\n",mmapped_readwrite(filecount,0));
    }
    else if(strcmp(argv[1],"write")==0&&strcmp(argv[3],"mmap")==0){
        printf("average throughput for mmap write: %f MB/s\n",mmapped_readwrite(filecount,1));
    }
    else if(strcmp(argv[1],"read")==0&&strcmp(argv[3],"disk")==0){
        printf("average throughput for disk read: %f MB/s\n",disk_readwrite(filecount,0));
    }
    else if(strcmp(argv[1],"write")==0&&strcmp(argv[3],"disk")==0){
        printf("average throughput for disk write: %f MB/s\n",disk_readwrite(filecount,1));
    }

}
