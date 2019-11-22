#include<stdio.h>
#include<sys/time.h>
#include<math.h>
#include<stdlib.h>

int main(){
    struct timeval start,end;
    int numPage,numTrial;

    printf("NumberOfPages - ");
    scanf("%d",&numPage);

    printf("number of trials - ");
    scanf("%d",&numTrial);

    int* arr = (int*)calloc(10000000,sizeof(int));
    int jump = 1<<10;

    gettimeofday(&start,NULL);

    for(int a = 0; a < numTrial; a++){
        for(int i=0; i < numPage * jump; i+=jump)
            arr[i]+=1;
    }

    gettimeofday(&end,NULL);

    printf("Average Cost Of Accessing a page - %lf nanoseconds", ((end.tv_sec-start.tv_sec)*pow(10,6)+
                                                                (end.tv_usec - start.tv_usec))/((numTrial*numPage)/1000.0));

    return 0;

}
