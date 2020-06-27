#include <stdio.h>
#include "prpc.h"

char* read_file(FILE* fp){
    long lSize;
    char *buffer;

    fseek( fp , 0L , SEEK_END);
    lSize = ftell( fp );
    rewind( fp );

    /* allocate memory for entire content */
    buffer = calloc( 1, lSize+1 );
    if( !buffer ){
        fclose(fp);
        fputs("memory alloc fails",stderr);
        exit(1);
    }

    /* copy the file into the buffer */
    if(fread( buffer , lSize, 1 , fp) != 1 ){
        fclose(fp);
        free(buffer);
        fputs("entire read fails",stderr);
        exit(1);
    }

    fclose(fp);
    return buffer;
}

// client code
int main(int argc, char *argv[]) {

    int sd = Channel_Init(20000);
    int rc;
    FILE* fptr;
    char *message = (char*)Malloc(sizeof(char)*BUFFER_SIZE);
    memcpy(message, "hello world", strlen("hello world"));

    if(argc < 2){
        rc = Connect_to("localhost", 10000);
    }
    else if(argc < 3){
        rc = Connect_to(argv[1], 10000);
    }
    else if(argc < 4){
        rc = Connect_to(argv[1], atoi(argv[2]));
    }
    else if(argc < 5){
        rc = Connect_to(argv[1], atoi(argv[2]));
        fptr = fopen(argv[3], "r");
        if(fptr == NULL){
            perror("file didn't open\n");
            exit(1);
        }
        free(message);
        message = read_file(fptr);
    }
    else{
        perror("Fuck you Buoy!! what more do you want?\n");
        exit(1);
    }

    while(1){
    printf("client:: send message\n");
    rc = psend(sd, message);
    if (rc < 0) {
	printf("client:: failed to send\n");
	exit(1);
    }

    //printf("client:: wait for reply...\n");
    //char* reply = NULL;
    //rc = preceive(sd, &reply);
    //printf("client:: got reply [size:%d contents:(%s)\n", rc, reply);
    //free(reply);
    }
    return 0;
}

