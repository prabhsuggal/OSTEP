#include<stdio.h>
#include<string.h>
#include<stdlib.h>
#include<sys/sysinfo.h>
#include<sys/mman.h>
#include<math.h>
#include<errno.h>
#include<stdbool.h>

#include "common.h"
#include "common_threads.h"

void *Malloc(size_t size) {
    void *rc = malloc(size);
    assert(rc != NULL);
    return rc;
}

typedef struct zip_t{
    char *res;
    int size;
}zip;

zip *gzip, zip_res = { NULL, 0};

char *fileptr;
int file_piece, idx;
struct stat sbuf;
bool map_done=false;
pthread_cond_t mapped;
pthread_mutex_t mutex;

sem_t increment,tlock;

void concat(zip *zip_res, zip cat){
    if(zip_res->size == 0){
        memcpy(zip_res->res, cat.res, cat.size);
        zip_res->size = cat.size; 
    }
    else{
        int tmp1, tmp2, tmp;
        char str1, str2;
        memcpy(&tmp1, zip_res->res+zip_res->size-5, 4);
        memcpy(&tmp2, cat.res, 4);
        str1 = zip_res->res[zip_res->size-1];
        str2 = cat.res[4];
        if(str1 == str2){
            tmp = tmp1+tmp2;
            memcpy(zip_res->res + zip_res->size-5, &tmp, 4);
            zip_res->res[zip_res->size-1] = str1;
            memcpy(zip_res->res + zip_res->size, cat.res+5, cat.size-5);
            zip_res->size += (cat.size - 5);
        }
        else{
            memcpy(zip_res->res + zip_res->size, cat.res, cat.size);
            zip_res->size += cat.size;
        }
    }
}

zip wzip(char *fptr, int size){
    //printf("String sent to thread : %ld is %.*s\n", gettid(), size, fptr);
    zip *buf = (zip*)Malloc(sizeof(zip));
    buf->res = (char*)Malloc(sizeof(char)*size*5);
    char *tmp;
    int count=0,it=0;
    char str;
    str = *fptr;
    count++;

    for(int i=1; i <size;i++){
        if(str == *(fptr + i)){
            count++;
        }
        else{
            memcpy(buf->res+it*5, &count, sizeof(int));
            buf->res[it*5+sizeof(int)] = str;
            it++;
            str = *(fptr + i);
            count=1;
        }
    }
    if(count != 0){
        memcpy(buf->res+it*5, &count, sizeof(int));
        buf->res[it*5+sizeof(int)] = str;
        it++;
    }
    buf->size = 5*it;
    tmp = buf->res;
    buf->res = realloc(buf->res, 5*it);
    if(buf->res == NULL){
        printf("%s: realloc failed\n", __FUNCTION__);
        buf->res = tmp;
    }
    return *buf;
}

void* single_wzip(void *arg)
{
    char *file_ptr;
    int size, loc_idx;

    Mutex_lock(&mutex);
    while(!map_done)
        Cond_wait(&mapped, &mutex);
    Mutex_unlock(&mutex);

    while(1){
        Sem_wait(&increment);
        if(idx*file_piece < sbuf.st_size){
            file_ptr = fileptr + idx*file_piece;
            if(sbuf.st_size < (idx+1)*file_piece){
                size = sbuf.st_size - idx*file_piece;
            }
            else{
                size = file_piece;
            }
            loc_idx = idx++;
        }
        else{
            Sem_post(&increment);
            break;
        }  
        Sem_post(&increment);

        //printf("String sent to thread : %ld with idx %d is \n %.*s\n", gettid(), loc_idx, size, file_ptr);

        gzip[loc_idx] = wzip(file_ptr, size);


        /* int tmp=0;
        Sem_wait(&tlock);
        printf("zip for idx %d thread %ld\n",loc_idx, gettid());
        for(int y=0; y < gzip[loc_idx].size; y+=5){
            memcpy(&tmp, gzip[loc_idx].res + y, 4);
            printf("%d%c",tmp,gzip[loc_idx].res[y+4]);
        }
        printf("\n");
        Sem_post(&tlock); */

    }
    return NULL;
}

int main(int argc, char* argv[])
{   
    int opt, num_threads, tot_parts;
    int mul_t = 1, cores = get_nprocs_conf();
    int fd, pieces = 2, num_files=0, zip_res_size = 0;

    Sem_init(&increment, 1);
    Sem_init(&tlock,1);
    Mutex_init(&mutex);
    Cond_init(&mapped);

    while((opt = getopt(argc, argv, "m:p:")) != -1){
        switch (opt)
        {
        case 'm':
            mul_t = atoi(optarg);
            break;
        case 'p':
            pieces = atoi(optarg);
            break;
        default :
            fprintf(stderr, "Usage: ./pzip [-m multiple] [-p avg_file_parts_per_thread] file1 [file2]...\n");
            exit(1);
        }
    }
    if(optind == argc){
        fprintf(stderr, "Usage: ./pzip [-m multiple] [-p avg_file_parts_per_thread] file1 [file2]...\n");
        exit(1);
    }
    
    num_files = argc - optind;
    num_threads = cores*mul_t;
    tot_parts = num_threads*pieces;
	pthread_t *thread_t = (pthread_t*)Malloc(num_threads*sizeof(pthread_t));
    gzip = (zip*)Malloc(sizeof(zip)*tot_parts);
    fprintf(stderr, "will be running %d threads in %d cores CPU\n", num_threads, cores);

    for(int i=0; i < num_threads; i++){
        Pthread_create(&thread_t[i], NULL, single_wzip, NULL);
    }
    double stime=0,etime=0;
    stime = GetTime();
    for(;optind<argc;optind++){
        fd = Open(argv[optind], O_RDONLY);
        Fstat(fd, &sbuf);
        file_piece = sbuf.st_size/tot_parts + (sbuf.st_size%tot_parts != 0) ;
        fileptr = mmap(NULL, sbuf.st_size, PROT_READ, MAP_FILE | MAP_PRIVATE | MAP_POPULATE, fd, 0);
        if(fileptr == MAP_FAILED){
            fprintf(stderr, "%s: mmap failed for %s and error: %s\n", __FUNCTION__, argv[optind], strerror(errno))
            ;
            exit(1);
        }
        Mutex_lock(&mutex);
        Pthread_cond_broadcast(&mapped);
        map_done = true;
        Mutex_unlock(&mutex);
    }

    for(int i=0;i < num_threads; i++){
        Pthread_join(thread_t[i], NULL);
    }

    for(int i = 0; i < num_files; i++){
        munmap(fileptr, sbuf.st_size);
    }

    for(int i = 0;i < tot_parts; i++){
        zip_res_size+=gzip[i].size;
    }
    zip_res.res = (char*)Malloc(sizeof(char)*zip_res_size);

    for(int i=0;i < tot_parts; i++){
        concat(&zip_res, gzip[i]);
        /* for(int y=0; y < gzip[i].size; y+=5){
            printf("%d%c",*(int*)(gzip[i].res + y),gzip[i].res[y+4]);
        }
        printf("\n"); */
    }

    for(int i=0; i<zip_res.size; i+=5){
        printf("%d%c",*(int*)(zip_res.res + i),zip_res.res[i+4]);
    }    

    etime = GetTime();

    fprintf(stderr, " Time taken by pzip is %f sec\n", etime-stime);

    return 0;
}