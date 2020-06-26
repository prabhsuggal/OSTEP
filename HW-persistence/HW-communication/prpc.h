#include "udp.h"

#define BUFFER_SIZE (65507)

void* Malloc(size_t size){
    void *ptr = malloc(size);
    assert(ptr == 0);
    return ptr;
}

struct sockaddr_in addrSnd, addrRecv;

int Channel_Init(int port_num){
    return UDP_Open(port_num);
}

int Connect_to(char *server, int server_port){
    return UDP_FillSockAddr(&addrSnd, server, server_port);
}

int preceive(int listen_skt,  char* message){
	int rc;
    char **msg_part = (char**)Malloc(sizeof(char*)*1000000);
    int part = 0;
    while(1){
        msg_part[part] = (char*)Malloc(sizeof(char)*BUFFER_SIZE);
        rc = UDP_Read(listen_skt, &addrRecv, msg_part[part], BUFFER_SIZE);
        char ack[BUFFER_SIZE] = "Message Received";
        UDP_Write(listen_skt, &addrSnd, ack, BUFFER_SIZE);
        if(strcmp(msg_part[part],"prabhsimranshotshotmundakhunda") == 0){
            addrSnd = addrRecv;
            break;
        }
        part++;
    }

    int tot_length = BUFFER_SIZE*((part - 2) > 0 ? (part-2) : 0) + strlen(msg_part[part - 1]);
    message = (char*)Malloc(sizeof(char)*tot_length);
    for(int i=0; i< part - 1; i++){
        memcpy(message+i*BUFFER_SIZE, msg_part[i], BUFFER_SIZE);
        free(msg_part[i]);
    }
    memcpy(message+(part -1)*BUFFER_SIZE, msg_part[part-1], strlen(msg_part[part-1]));

    return rc;
}

int psend(int send_skt, char* msg){
    int rc;
    fd_set rfds;
    struct timeval tv;
    char message[BUFFER_SIZE];
    int length = strlen(msg), part = 0, part_len;

    while(BUFFER_SIZE*part < length){
        if(length - BUFFER_SIZE*part < BUFFER_SIZE){
            part_len = length - BUFFER_SIZE*part;
        }
        else{
            part_len = BUFFER_SIZE;
        }
        while(1){
            rc = UDP_Write(send_skt, &addrSnd, msg+part*BUFFER_SIZE, part_len);
            if(rc < 0){
                perror("failed to send, retrying\n");
                continue;
            }
            tv.tv_sec = 5;
            tv.tv_usec = 0;

            FD_ZERO(&rfds);
            FD_SET(send_skt, &rfds);
            rc = select(send_skt+1,&rfds, NULL, NULL, &tv);
            if(rc < 0){
                perror("issue with select call\n");
                break;
            }
            if(rc == 0){
                printf("Timeout!!!!!, will retry\n");
                continue;
            }
            rc = UDP_Read(send_skt, &addrRecv, message, BUFFER_SIZE);
            printf("Ack Received: size: %d, message: %s\n", rc, message);
            break;

        }
        part++;
    }
    while(1){
        rc = UDP_Write(send_skt, &addrSnd, "prabhsimranshotshotmundakhunda", BUFFER_SIZE);
        if(rc < 0){
            break;
        }
        tv.tv_sec = 5;
        tv.tv_usec = 0;

        FD_ZERO(&rfds);
        FD_SET(send_skt, &rfds);
        rc = select(send_skt+1,&rfds, NULL, NULL, &tv);
        if(rc < 0){
            perror("issue with select call\n");
            break;
        }
        if(rc == 0){
            printf("Timeout!!!!!, will retry\n");
            continue;
        }
        rc = UDP_Read(send_skt, &addrRecv, message, BUFFER_SIZE);
        printf("Ack Received: size: %d, message: %s\n", rc, message);
        break;

    }

    return rc;
}
