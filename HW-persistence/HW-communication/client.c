#include <stdio.h>
#include "prpc.h"

// client code
int main(int argc, char *argv[]) {
    //for(int i =0;i < atoi(argv[1]);i++)
    //    fork();

    int sd = Channel_Init(20000);
    int rc = Connect_to("localhost", 10000);

    char message[BUFFER_SIZE] = "hello world";

    printf("client:: send message [%s]\n", message);
    rc = psend(sd, message);
    if (rc < 0) {
	printf("client:: failed to send\n");
	exit(1);
    }

    printf("client:: wait for reply...\n");
    rc = preceive(sd, message);
    printf("client:: got reply [size:%d contents:(%s)\n", rc, message);
    return 0;
}

