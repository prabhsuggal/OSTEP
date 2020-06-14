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
#include "rw-lock.h"

#define MAX_PARTS 1024

void *Malloc(size_t size) {
    void *rc = malloc(size);
    assert(rc != NULL);
    return rc;
}

//struct for zipping parts of the file
typedef struct zip_t{
    char *res;
    int size;
}zip;

//global constructs
zip *gzip; 

//storing all the file pointers for given files
char **fileptr;
/**
 * piece_size = size of each part of file which is sent in each thread as block(configurable)
 * idx = storing loc until which file has been sent for processing
 * file_idx = which file is currently being processed
 * num_files = nuum of files sent for zipping
 * tot_parts = total parts created while doing pzip(should be less than MAX_PARTS)
 * prev_file_end_idx = idx where the prev file's last part was processed
 * sbuf = struct array to store size of all files involved
 */
int piece_size = 100, idx, file_idx = 0, num_files, tot_parts=0, prev_file_end_idx = 0;
struct stat* sbuf;

/**
 * mutex/semaphores/rwlock used
 * mapped/map_done/map_t : used to ensure that worker threads start working on a file only once mmap is done
 * increment : done to ensure that idx is tampered by only one thread at a time
 * tlock : was used to print some debug logs cleanly.
 * rwlock_t : for updating file_idx, once given file parts are done
 */
bool* map_done;
pthread_cond_t* mapped;
pthread_mutex_t* map_t;

sem_t increment, tlock;
rwlock_t file_idx_lock;

/**
 * @brief used for concating zip_res and cat
 *          Result is stored in zip_res only. also updates size attribute for zip_res
 * 
 * @param zip_res 
 * @param cat 
 */
void concat(zip *zip_res, zip cat){
    // when zip_res is empty
    if(zip_res->size == 0){
        memcpy(zip_res->res, cat.res, cat.size);
        zip_res->size = cat.size; 
    }
    // zip_res - non-empty
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

/**
 * @brief simple function for zipping. this is single threaded function runnning
 *          in parallel threads.
 * 
 * @param fptr 
 * @param size 
 * @return zip 
 */
zip wzip(char *fptr, int size){
    zip *buf = (zip*)Malloc(sizeof(zip));
    buf->res = (char*)Malloc(sizeof(char)*size*5);
    char *tmp;
    int count=0,it=0;
    char str;
    str = *fptr;
    count++;

    //zipping a file part pointed by fptr
    for(int i=1; i < size;i++){
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

/**
 * @brief worker threads taking file parts and and sending them to 
 *          wzip function.
 *          all the mutli threading complexities are handled here.
 * 
 * @param arg 
 * @return void* 
 */
void* single_wzip(void *arg)
{
    char *file_ptr;
    int size, loc_idx, loc_file_idx;

NEW_FILE:
    rwlock_acquire_readlock(&file_idx_lock);
    Mutex_lock(map_t + file_idx);
    while(!map_done[file_idx])
        Cond_wait(mapped + file_idx, map_t + file_idx);
    Mutex_unlock(map_t + file_idx);
    rwlock_release_readlock(&file_idx_lock);

    while(1){
        Sem_wait(&increment);
        if(loc_file_idx != file_idx){
            if(file_idx < num_files){
                loc_file_idx = file_idx;
                Sem_post(&increment);
                goto NEW_FILE;
            }
            Sem_post(&increment);
            break;
        }
        if((idx-prev_file_end_idx)*piece_size < sbuf[file_idx].st_size){
            file_ptr = fileptr[file_idx] + (idx-prev_file_end_idx)*piece_size;
            if(sbuf[file_idx].st_size < ((idx-prev_file_end_idx)+1)*piece_size){
                size = sbuf[file_idx].st_size - (idx-prev_file_end_idx)*piece_size;
            }
            else{
                size = piece_size;
            }
            loc_idx = idx++;
        }
        else{
            rwlock_acquire_writelock(&file_idx_lock);
            file_idx++;
            prev_file_end_idx = idx;
            rwlock_release_writelock(&file_idx_lock);
            Sem_post(&increment);
            continue;
        }
        tot_parts++;
        Sem_post(&increment);

        // printf("String sent to thread : %ld with idx %d is \n %.*s\n", gettid(), loc_idx, size, file_ptr);

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
    zip zip_res = { NULL, 0};
    int opt, num_threads;
    int mul_t = 1, cores = get_nprocs_conf();
    int* fd, zip_res_size = 0;

    Sem_init(&increment, 1);
    Sem_init(&tlock,1);
    rwlock_init(&file_idx_lock);

    while((opt = getopt(argc, argv, "m:p:")) != -1){
        switch (opt)
        {
        case 'm':
            mul_t = atoi(optarg);
            break;
        case 'p':
            piece_size = atoi(optarg);
            break;
        default :
            fprintf(stderr, "Usage: ./pzip [-m multiple] [-p avg_file_block_size_per_thread] file1 [file2]...\n");
            exit(1);
        }
    }
    if(optind == argc){
        fprintf(stderr, "Usage: ./pzip [-m multiple] [-p avg_file_block_size_per_thread] file1 [file2]...\n");
        exit(1);
    }
    
    num_files = argc - optind;
    num_threads = cores*mul_t;

    /**
     * @brief initialising variables in basis of num_threads, num_files
     * 
     */
	pthread_t *thread_t = (pthread_t*)Malloc(num_threads*sizeof(pthread_t));
    gzip = (zip*)Malloc(sizeof(zip)*MAX_PARTS);
    fd = (int*)Malloc(sizeof(int)*num_files);
    sbuf = (struct stat*)Malloc(sizeof(struct stat)*num_files);
    fileptr = (char**)Malloc(sizeof(char*)*num_files);
    fprintf(stderr, "will be running %d threads in %d cores CPU\n", num_threads, cores);

    /**
     * mutex/cond init
     */
    map_done = (bool*)Malloc(sizeof(bool)*num_files);
    mapped = (pthread_cond_t*)Malloc(sizeof(pthread_cond_t)*num_files);
    map_t = (pthread_mutex_t*)Malloc(sizeof(pthread_mutex_t)*num_files);
    for(int i=0; i<num_files; i++){
        Cond_init(mapped+i);
        Mutex_init(map_t+i);
        map_done[i] = false;
    }

    for(int i=0; i < num_threads; i++){
        Pthread_create(&thread_t[i], NULL, single_wzip, NULL);
    }
    double stime=0,etime=0;
    stime = GetTime();
    for(int i=0;optind<argc;optind++, i++){
        fd[i] = Open(argv[optind], O_RDONLY);
        Fstat(fd[i], &sbuf[i]);
        
        fileptr[i] = mmap(NULL, sbuf[i].st_size, PROT_READ, MAP_FILE | MAP_PRIVATE | MAP_POPULATE, fd[i], 0);
        if(fileptr == MAP_FAILED){
            fprintf(stderr, "%s: mmap failed for %s and error: %s\n", __FUNCTION__, argv[optind], strerror(errno));
            exit(1);
        }
        Mutex_lock(map_t+i);
        Pthread_cond_broadcast(mapped+i);
        map_done[i] = true;
        Mutex_unlock(map_t+i);
    }

    for(int i=0;i < num_threads; i++){
        Pthread_join(thread_t[i], NULL);
    }

    for(int i = 0; i < num_files; i++){
        munmap(fileptr[i], sbuf[i].st_size);
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