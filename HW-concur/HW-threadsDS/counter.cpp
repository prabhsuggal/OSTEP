#include<stdio.h>
#include"counter.h"

counter_t c;

void* multInc(void *arg){
    
    counter_t *c = (counter_t*)arg;
    for(int i=0;i<1e6;i++){
        increment(c);
    }

    return NULL;
}

int main(){

    int threads;
    printf("No. Of Threads required:");
    scanf("%d",&threads);
   
    init(&c);

    pthread_t* p = (pthread_t*)calloc(sizeof(pthread_t),threads);
    double start,end;

    start = Time_GetSeconds();
    for(int i=0;i<threads;i++){
        pthread_create(&p[i],NULL,multInc, &c);
    }

    for(int i=0;i<threads;i++){
        pthread_join(p[i],NULL);
    }
    
    end = Time_GetSeconds();
    printf("value of counter after %lf updates is %d in %lf seconds\n",threads*1e6,get(&c), end-start);

    return 0;
}
