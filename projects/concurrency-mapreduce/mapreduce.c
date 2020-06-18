#include "stdio.h"
#include "string.h"
#include "mapreduce.h"
#include "common.h"
#include "common_threads.h"


#define MAX_ENGLISH_WORDS 350000

typedef struct value_t{
    char* value;
    struct value_t* next;
}val;

typedef struct key_box{
    char* key;
    val* val_head;
}key_box;

typedef struct sort_arr_{
    char* key;
    int index;
}sort_arr;

typedef struct reducer_with_mem_idx_{
    Reducer r;
    int index;
}reducer_with_mem_idx_t;

key_box **glob_mem;
sort_arr **sorter;
int *sorter_size;
key_box *curr_reduce_key;

sem_t file_idx_mutex, *insert_mutex, print_mutex;
char **file_list;
int num_files, file_idx=0, num_partitions = 0;

Partitioner get_partition;

unsigned long hashstring(unsigned char *str)
{
    unsigned long hash = 5381;
    int c;

    while ((c = *str++))
        hash = ((hash << 5) + hash) + c; /* hash * 33 + c */

    return hash;
}

void* mapper_thread(void *arg){
    double stime,etime;
    stime = GetTime();
    Mapper map = (Mapper)arg;
    int next_file_idx;
    while(1){
        Sem_wait(&file_idx_mutex);
        next_file_idx = file_idx++;
        Sem_post(&file_idx_mutex);
        if(next_file_idx < num_files){
            map(file_list[next_file_idx+1]);
        }
        else{
            break;
        }
    }
    etime = GetTime();
    fprintf(stderr, "Time taken by %s for thread %ld is %f sec %d\n",
                 __FUNCTION__, gettid(), etime-stime, next_file_idx);
    return NULL;
}

char* get_next(char* key, int partition_number){
    if(strcmp(curr_reduce_key[partition_number].key,key) == 0){
        if(curr_reduce_key[partition_number].val_head == NULL){
            return NULL;
        }
        else{
            val* tmp = curr_reduce_key[partition_number].val_head;
            curr_reduce_key[partition_number].val_head = tmp->next;
            return tmp->value;
        }
    }
    assert(0);
    return NULL;
}

void* reducer_thread(void *arg){
    double stime,etime;
    stime = GetTime();
    reducer_with_mem_idx_t* tmp = (reducer_with_mem_idx_t*)arg;
    int queue_num = tmp->index;
    val *tmp1, *tmp2;
    Reducer reduce = tmp->r;
    for(int i = 0;i < sorter_size[queue_num]; i++){
        memcpy(&curr_reduce_key[queue_num], &glob_mem[queue_num][sorter[queue_num][i].index], sizeof(key_box));
        reduce(sorter[queue_num][i].key, get_next, queue_num);
        tmp1 = glob_mem[queue_num][sorter[queue_num][i].index].val_head;
        while(tmp1 != NULL){
            tmp2 = tmp1->next;
            free(tmp1);
            tmp1 = tmp2;
        }
    }
    etime = GetTime();
    fprintf(stderr, "Time taken by %s for thread %ld is %f sec\n", __FUNCTION__, gettid(), etime-stime);
    return NULL;
}

static int key_compare(const void *a, const void *b){
    char *str1, *str2;
    str1 = (*(sort_arr*)a).key;
    str2 = (*(sort_arr*)b).key;
    return strcmp(str1,str2);
}

void sort(sort_arr *arr, int n){

    qsort(arr, n, sizeof(sort_arr), key_compare);

    return;
}

void arrange(void){

    double stime,etime;
    stime = GetTime();
    //print(__FUNCTION__);
    for(int i = 0; i < num_partitions; i++){
        int idx = 0;
        sorter[i] = (sort_arr*)Malloc(sizeof(sort_arr)*sorter_size[i]);
        for(int j=0; j< MAX_ENGLISH_WORDS; j++){
            if(glob_mem[i][j].key != NULL){
                sorter[i][idx].key = glob_mem[i][j].key;
                sorter[i][idx++].index = j;
            }
        }
        //fprintf(stderr, "size of sorter %d sorter_size value %d\n", idx, sorter_size[i]);
        sort(sorter[i], sorter_size[i]);
    }
    etime = GetTime();
    fprintf(stderr, "Time taken by %s is %f sec\n", __FUNCTION__, etime-stime);


    return;
}


void MR_Run(int argc, char *argv[], 
	    Mapper map, int num_mappers, 
	    Reducer reduce, int num_reducers, 
	    Partitioner partition)
{
    get_partition = partition;
    num_partitions = num_reducers;

    curr_reduce_key = (key_box*)Malloc(num_reducers*sizeof(key_box));
    sorter_size  = (int*)Calloc(num_partitions, sizeof(int));
    sorter = (sort_arr**)Malloc(sizeof(sort_arr*)*num_partitions);

    pthread_t *mapper_threads, *reducer_threads;
    mapper_threads = (pthread_t*)Malloc(sizeof(pthread_t)*num_mappers);
    reducer_threads = (pthread_t*)Malloc(sizeof(pthread_t)*num_reducers);
    file_list = argv;
    num_files = argc-1;

    Sem_init(&file_idx_mutex, 1);
    Sem_init(&print_mutex, 1);
    insert_mutex = (sem_t*)Malloc(sizeof(sem_t)*num_partitions);
    for(int i=0; i<num_partitions; i++){
        Sem_init(&insert_mutex[i], 1);
    }

    glob_mem = (key_box**)Calloc(num_reducers, sizeof(key_box*));
    for(int i=0;i < num_reducers; i++){
        glob_mem[i] = (key_box*)Calloc(MAX_ENGLISH_WORDS, sizeof(key_box));
    }

    for(int i=0; i<num_mappers; i++){
        Pthread_create(&mapper_threads[i], NULL, mapper_thread, map);
    }

    for(int i = 0;  i< num_mappers; i++){
        Pthread_join(mapper_threads[i], NULL);
    }

    arrange();
    
    reducer_with_mem_idx_t args[num_reducers];
    for(int i=0; i<num_reducers; i++){
        args[i].r = reduce;
        args[i].index = i;
        Pthread_create(&reducer_threads[i], NULL, reducer_thread, &args[i]);
    }

    for(int i = 0;  i< num_reducers; i++){
        Pthread_join(reducer_threads[i], NULL);
    }

    for(int i=0; i < num_partitions; i++){
        for(int j = 0; j < MAX_ENGLISH_WORDS; j++){
            free(glob_mem[i][j].key);
        }
        free(glob_mem[i]);
        free(sorter[i]);
    }
    free(curr_reduce_key);
    free(mapper_threads);
    free(reducer_threads);
    free(sorter);
    free(sorter_size);
    free(glob_mem);
}

void MR_Emit(char *key, char *value){
    int queue = get_partition(key, num_partitions);
    unsigned long idx = hashstring((unsigned char*)key)%MAX_ENGLISH_WORDS;
    Sem_wait(&insert_mutex[queue]);
    while(glob_mem[queue][idx].val_head != NULL){
        if(strcmp(key, glob_mem[queue][idx].key) == 0){
            val* val_node = (val*)Malloc(sizeof(val));
            val_node->value = value;
            val_node->next = glob_mem[queue][idx].val_head;
            glob_mem[queue][idx].val_head = val_node;

            Sem_post(&insert_mutex[queue]);
            return;
        }
        idx = (idx+1)%MAX_ENGLISH_WORDS;
    }
    glob_mem[queue][idx].key = (char*)Malloc(sizeof(char)*(strlen(key)+1));
    strcpy(glob_mem[queue][idx].key, key);
    val* val_node = (val*)Malloc(sizeof(val));
    val_node->value = value;
    val_node->next = NULL;
    glob_mem[queue][idx].val_head = val_node;
    sorter_size[queue]++;
    Sem_post(&insert_mutex[queue]);

    return;
}

void print(const char* calling_function){
    Sem_wait(&print_mutex);
    printf("***********STARTING**PRINT**OF**GLOBAL**MEM******\n");
    printf("---------IN--FUNCTION--%s----\n",calling_function);
    for(int i=0;i < num_partitions; i++){
        printf("In Partitition %d\n", i);
        for(int j=0; j < MAX_ENGLISH_WORDS; j++){
            if(glob_mem[i][j].key != NULL){
                printf("Key : \"%s\" values", glob_mem[i][j].key);
                val* node = glob_mem[i][j].val_head;
                while(node != NULL){
                    printf("-> %s", node->value);
                    node = node->next;
                }
                printf("-> NULL\n");
            }
        }   
    }
    printf("***********FINISHED**PRINT**OF**GLOBAL**MEM******\n");
    Sem_post(&print_mutex);
}

