#include <stdio.h>
#include "prpc.h"

// client code
int main(int argc, char *argv[]) {

    int sd = Channel_Init(20000);
    int rc;
    if(argc == 1){
        rc = Connect_to("localhost", 10000);
    }
    else if(argc == 2){
        rc = Connect_to(argv[1], 10000);
    }
    else if(argc == 3){
        rc = Connect_to(argv[1], atoi(argv[2]));
    }
    else{
        perror("Fuck you Buoy!! what more do you want?\n");
        exit(1);
    }


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

