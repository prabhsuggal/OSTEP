#include <stdio.h>
#include "prpc.h"

// server code
int main(int argc, char *argv[]) {
    int sd = Channel_Init(10000);
    assert(sd > -1);
    while (1) {
	char* message = NULL;
	printf("server:: waiting... skt_fd %d\n", sd);
    int rc = preceive(sd, &message);
	printf("server:: read message [ret:%d ]\n", rc);
	//if (rc > 0) {
            //char reply[BUFFER_SIZE];
            //sprintf(reply, "goodbye world");
            //rc = psend(sd,reply);
	    //printf("server:: reply\n");
	//}
    free(message);
    }
    return 0;
}




