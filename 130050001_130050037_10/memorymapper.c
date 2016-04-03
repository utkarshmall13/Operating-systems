#include <sys/mman.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdlib.h>
#include <stdio.h>

void wait_for_enter(){
    while(getchar()!='\n');
}

int main(int argc, char *argv[]){
    int fddata,file_size;
    struct stat sb;
    char* src;

    fddata = open("files/foo1.txt", O_RDONLY);
    if (fddata == -1) exit(1);
    if (fstat(fddata, &sb) == -1) exit(1);
    file_size=sb.st_size;
    if (file_size == 0) exit(1);
    //part (1)
    printf("file not memory mapped\n");
    wait_for_enter();

    src = mmap(NULL, sb.st_size, PROT_READ, MAP_PRIVATE, fddata, 0);
    if (src == MAP_FAILED)  exit(1);
    //part (2)
    printf("file memory mapped\n");
    wait_for_enter();

    printf("%c\n",src[0]);
    //part (3)
    printf("read first bit\n");
    wait_for_enter();
    int curr_offset=10000;
    while(curr_offset<file_size){
        printf("%c",src[curr_offset]);
        curr_offset+=10000;
    }
    printf("\n");
    //part (3)
    printf("read each 10000th bit\n");
    wait_for_enter();

    if(munmap(src, file_size) == -1) {
	perror("Error un-mmapping the file");
    }
    exit(0);
}
