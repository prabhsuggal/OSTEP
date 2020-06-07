#include <stdio.h>
#include "request.h"
#include "io_helper.h"
#include "mythreads.h"
#include "common.h"
#include "common_threads.h"

char default_root[] = ".";
char default_schedalg[] = "FIFO";
int *fix_buffer;
int buffers, fill = 0, use = 0;
sem_t empty, full, mutex;

void put(int fd){
	fix_buffer[fill] = fd;
	fill = (fill + 1)%buffers;
}

int get(){
	int tmp = fix_buffer[use];
	use = (use + 1)%buffers;
	return tmp;
}

void *worker_thread(void *arg){
	printf("starting slave thread pid %ld\n", gettid());
	while(1){
		Sem_wait(&full);
		Sem_wait(&mutex);
		int tmp_fd = get();
		Sem_post(&mutex);
		Sem_post(&empty);
		request_handle(tmp_fd);
		close_or_die(tmp_fd);
	}
	return NULL;
}
//
// ./wserver [-d <basedir>] [-p <portnum>] 
// 
int main(int argc, char *argv[]) {
    int c;

    char *root_dir = default_root;
	char *schedalg = default_schedalg;
    int port = 10000;
	int threads = 1;
	buffers = 1;
    
    while ((c = getopt(argc, argv, "d:p:t:b:s")) != -1)
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
		schedalg = optarg;
		break;
	default:
	    fprintf(stderr, "usage: wserver [-d basedir] [-p port] [-t threads] [-b buffers] [-s schedalg]\n");
	    exit(1);
	}

    // run out of this directory
    chdir_or_die(root_dir);

    // now, get to work
    int listen_fd = open_listen_fd_or_die(port);
	pthread_t *slave = (pthread_t*)Malloc(threads*sizeof(pthread_t));
	fix_buffer = (int*)Malloc(buffers*sizeof(int));
	
	fprintf(stdout, "Server starting with basedir %s port %d threads %d buffers %d schedalg %s\n",
			root_dir, port, threads, buffers, schedalg);
	Sem_init(&empty, buffers);
	Sem_init(&full, 0);
	Sem_init(&mutex, 1);

	for(int i=0; i < threads; i++){
		Pthread_create(&slave[i], NULL, worker_thread, NULL);
	}
	printf("Parent thread accepting requests\n");
    while (1) {
	struct sockaddr_in client_addr;
	int client_len = sizeof(client_addr);
	int conn_fd = accept_or_die(listen_fd, (sockaddr_t *) &client_addr, (socklen_t *) &client_len);
	Sem_wait(&empty);
    Sem_wait(&mutex);
    put(conn_fd);
    Sem_post(&mutex);
    Sem_post(&full);
    }
    return 0;
}