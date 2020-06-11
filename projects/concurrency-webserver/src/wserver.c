#include <stdio.h>
#include <limits.h>
#include <stdbool.h>
#include <errno.h>
#include "request.h"
#include "io_helper.h"
#include "mythreads.h"
#include "common.h"
#include "common_threads.h"

#define MAXBUF (8192)
#define REQUEST_FILE(fd_t) { \
    struct stat sbuf; \
    char buf[MAXBUF], method[MAXBUF], uri[MAXBUF], version[MAXBUF]; \
    char filename[MAXBUF], cgiargs[MAXBUF]; \
    readline_or_die(fd_t, buf, MAXBUF); \
    sscanf(buf, "%s %s %s", method, uri, version); \
    request_parse_uri(uri, filename, cgiargs); \
	assert(stat(filename, &sbuf) == 0); \
	buf_i.fd = fd_t; \
	buf_i.size = sbuf.st_size; \
	memcpy(buf_i.buf, buf, sizeof(*buf)*MAXBUF); \
}


typedef struct buffer_t{
	int fd;
	int size;
	char* buf;
}buffer;


char default_root[] = ".";
buffer *fix_buffer;
int buffers, fill = 0, use = 0;
sem_t empty, full, mutex;
bool visual=false;

void print_queue(int start, bool show_size){
	for(int i=0;i<buffers; i++){
		printf("%d			%d\n",start, (show_size ? fix_buffer[start].size : 0));
		start++;
	}
}

#define PRINT_QUEUE(a, ...) print_queue(a,(true, ##__VA_ARGS__))


void (*put)(buffer*);
buffer (*get)(void);


void put_fifo(buffer* buf_i){
	fix_buffer[fill].fd = buf_i->fd;
	fix_buffer[fill].buf = NULL;
	fill = (fill + 1)%buffers;
}

buffer get_fifo(void){
	buffer tmp = fix_buffer[use];
	if(visual)PRINT_QUEUE(use, false);
	if(visual)printf("picked load with index %d\n", use);
	use = (use + 1)%buffers;
	return tmp;
}

void put_sff(buffer* buf_i){
	while(fix_buffer[fill].size != INT_MAX)
		fill = (fill+1)%buffers;
	fix_buffer[fill].fd = buf_i->fd;
	fix_buffer[fill].size = buf_i->size;
	fix_buffer[fill].buf = (char*)Malloc(MAXBUF*sizeof(char));
	memcpy(fix_buffer[fill].buf, buf_i->buf, sizeof(*(buf_i->buf))*MAXBUF);
}

buffer get_sff(void){
	int min_size_idx, min_size = INT_MAX;
	if(visual)PRINT_QUEUE(0);
	for(int i=0; i < buffers; i++){
		if(min_size > fix_buffer[use].size){
			min_size = fix_buffer[use].size;
			min_size_idx = use;
		}
		use = (use+1)%buffers;
	}
	fix_buffer[min_size_idx].size = INT_MAX;
	if(visual)printf("picked load with size %d index %d\n", min_size, min_size_idx);
	return fix_buffer[min_size_idx];
}


void *worker_thread(void *arg){
	if(visual)printf("starting slave thread pid %ld\n", gettid());
	while(1){
		Sem_wait(&full);
		Sem_wait(&mutex);
		buffer tmp_fd = get();
		Sem_post(&mutex);
		Sem_post(&empty);
		request_handle(tmp_fd.fd, tmp_fd.buf);
		close_or_die(tmp_fd.fd);
	}
	return NULL;
}
//
// ./wserver [-d <basedir>] [-p <portnum>] 
// 
int main(int argc, char *argv[]) {
    int c;

    char *root_dir = default_root;
	bool fifo = true;
    int port = 10000;
	int threads = 1;
	buffers = 1;
    
    while ((c = getopt(argc, argv, "d:p:t:b:s:v:")) != -1)
	switch (c) {
	case 'd':
	    root_dir = optarg;
	    break;
	case 'p':
	    port = atoi(optarg);
	    break;
    case 't':
		threads = atoi(optarg);
		if(threads < 1)
			threads = 1;
		break;
	case 'b':
		buffers = atoi(optarg);
		if(buffers < 0){
			buffers = 1;
		}
		break;
	case 's':
		if(strcmp(optarg, "FIFO") == 0){
			fifo = true;
		}
		else if(strcmp(optarg, "SFF") == 0){
			fifo = false;
		}
		else{
			fprintf(stderr, "%s is not an option for scheduling algorithm\n", optarg);
			exit(1);
		}
		break;
	case 'v':
		visual = true;
		break;
	default:
	    fprintf(stderr, "usage: wserver [-d basedir] [-p port] [-t threads] [-b buffers] [-s schedalg] [-v yes](custom for visualization)\n");
	    exit(1);
	}

    // run out of this directory
    chdir_or_die(root_dir);

	//deciding on basis of schedulalg
	if(fifo){ //FIFO
		put = put_fifo;
		get = get_fifo;
	}
	else{  // SFF
		put = put_sff;
		get = get_sff;
	}

    // now, get to work
    int listen_fd = open_listen_fd_or_die(port);
	pthread_t *slave = (pthread_t*)Malloc(threads*sizeof(pthread_t));
	fix_buffer = (buffer*)Malloc(buffers*sizeof(buffer));
	for(int i=0;i < buffers; i++){
		fix_buffer[i].size = INT_MAX;
	}
	
	fprintf(stdout, "Server starting with basedir %s port %d threads %d buffers %d schedalg %s and %s visual\n",
			root_dir, port, threads, buffers, (fifo ? "FIFO" : "SFF"), (visual)?"":"no");
	Sem_init(&empty, buffers);
	Sem_init(&full, 0);
	Sem_init(&mutex, 1);

	for(int i=0; i < threads; i++){
		Pthread_create(&slave[i], NULL, worker_thread, NULL);
	}
	printf("Parent thread accepting requests\n");
	buffer buf_i;
	if(!fifo){
		buf_i.buf = (char*)Malloc(sizeof(char)*MAXBUF);
	}
    while (1) {
	struct sockaddr_in client_addr;
	int client_len = sizeof(client_addr);
	int conn_fd = accept_or_die(listen_fd, (sockaddr_t *) &client_addr, (socklen_t *) &client_len);
	if(fifo){
		buf_i.fd = conn_fd;
		buf_i.size = -1;
		buf_i.buf = NULL;
		Sem_wait(&empty);
    	Sem_wait(&mutex);
    	put(&buf_i);
    	Sem_post(&mutex);
    	Sem_post(&full);
	}
	else{
		REQUEST_FILE(conn_fd);
		Sem_wait(&empty);
 		Sem_wait(&mutex);
 	   	put(&buf_i);
 	   	Sem_post(&mutex);
 	   	Sem_post(&full);
	}
    }
    return 0;
}