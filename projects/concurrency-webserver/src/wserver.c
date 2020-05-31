#include <stdio.h>
#include "request.h"
#include "io_helper.h"
#include "mythreads.h"

char default_root[] = ".";
char default_schedalg[] = "FIFO";
<<<<<<< HEAD
int *fix_buffer;
int buffers, fill = 0, use = 0;
=======
>>>>>>> f6baae5aed39df2d4913730ba6219ecc8780bee5

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
	
}
//
// ./wserver [-d <basedir>] [-p <portnum>] 
// 
int main(int argc, char *argv[]) {
    int c;
    char *root_dir = default_root;
	char *schedalg = default_schedalg;
    int port = 10000;
<<<<<<< HEAD
	int threads = 1;
	buffers = 1;
=======
	int threads = 1, buffers = 1;
>>>>>>> f6baae5aed39df2d4913730ba6219ecc8780bee5
    
    while ((c = getopt(argc, argv, "d:p:")) != -1)
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
	    fprintf(stderr, "usage: wserver [-d basedir] [-p port]\n");
	    exit(1);
	}

    // run out of this directory
    chdir_or_die(root_dir);

    // now, get to work
    int listen_fd = open_listen_fd_or_die(port);
	pthread_t *slave = (pthread_t*)Malloc(threads*sizeof(pthread_t));
	fix_buffer = (int*)Malloc(buffers*sizeof(int));

	for(int i=0; i < threads; i++){
		Pthread_create(&slave[i], NULL, worker_thread, NULL);
	}

    while (1) {
	struct sockaddr_in client_addr;
	int client_len = sizeof(client_addr);
	int conn_fd = accept_or_die(listen_fd, (sockaddr_t *) &client_addr, (socklen_t *) &client_len);

	request_handle(conn_fd);
	close_or_die(conn_fd);
    }
    return 0;
}


    


 
