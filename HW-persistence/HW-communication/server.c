#include <stdio.h>
#include "prpc.h"

// server code
int main(int argc, char *argv[]) {
    int sd = Channel_Init(10000);
    assert(sd > -1);
    while (1) {
	char* message = NULL;
	printf("server:: waiting...\n");
	int rc = preceive(sd, message);
	printf("server:: read message [size:%d contents:(%s)]\n", rc, message);
	if (rc > 0) {
            char reply[BUFFER_SIZE];
            sprintf(reply, "goodbye world");
            rc = psend(sd,reply);
	    printf("server:: reply\n");
	}
    }
    return 0;
}




