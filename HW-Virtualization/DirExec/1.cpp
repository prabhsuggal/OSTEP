#include<stdio.h>
#include<stdlib.h>
#include<sys/time.h>
#include<unistd.h>

int main(){
    int i=0,disc;
    
    struct timeval start,end;
    
    while(true){
        gettimeofday(&start,NULL);
        for(i=0;i<1000000;i++){
            read(disc,&i,1);
        }
        gettimeofday(&end,NULL);

        printf("%ld\n", ((end.tv_sec * 1000000 + end.tv_usec)
		   -  (start.tv_sec * 1000000 + start.tv_usec)));
    }

    return 0;

}
