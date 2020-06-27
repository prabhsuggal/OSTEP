#include "udp.h"

#define BUFFER_SIZE (100)
typedef struct pieces_{
    char msg[BUFFER_SIZE];
    int idx;
}pieces_t;

void* Malloc(size_t size){
    void *ptr = malloc(size);
    assert(ptr != NULL);
    return ptr;
}

void* Calloc(size_t size, size_t num){
    void *ptr = calloc(num, size);
    assert(ptr != NULL);
    return ptr;
}

char end_msg[] = "prabhsimranshotshotmundakhunda";
int end_msg_len = strlen("prabhsimranshotshotmundakhunda");

struct sockaddr_in addrSnd, addrRecv;

int Channel_Init(int port_num){
    return UDP_Open(port_num);
}

int Connect_to(char *server, int server_port){
    return UDP_FillSockAddr(&addrSnd, server, server_port);
}

int preceive(int listen_skt,  char** message){
	int rc;
    char **msg_part = (char**)Calloc(sizeof(char*), 1000);
    int parts = 0, tot_parts = __INT_MAX__;
    pieces_t tmp;
    while(1){
        rc = UDP_Read(listen_skt, &addrRecv, &tmp, sizeof(tmp));
        // printf("Msg Read : %s\n", tmp.msg);
        if(msg_part[tmp.idx] != NULL){
            continue;
        }
        msg_part[tmp.idx] = (char*)Malloc(sizeof(char)*BUFFER_SIZE);
        memcpy(msg_part[tmp.idx], tmp.msg, BUFFER_SIZE);
        parts++;
        if(strcmp(msg_part[tmp.idx], end_msg) == 0){
            tot_parts = tmp.idx+1;
            addrSnd = addrRecv;
        }
        if(parts == tot_parts){
            char ack[BUFFER_SIZE] = "Message Received";
            rc = UDP_Write(listen_skt, &addrRecv, ack, BUFFER_SIZE);
            // printf("Ack Sent: %s res %d", ack, rc);
            break;
        }
    }

    int tot_length = BUFFER_SIZE*((parts - 2) > 0 ? (parts-2) : 0) + strlen(msg_part[parts - 2]) + 1;
    *message = (char*)Malloc(sizeof(char)*tot_length);
    for(int i=0; i< parts - 2; i++){
        memcpy(*message+i*BUFFER_SIZE, msg_part[i], BUFFER_SIZE);
        free(msg_part[i]);
    }
    memcpy(*message+(parts - 2)*BUFFER_SIZE, msg_part[parts-2], strlen(msg_part[parts-2])+1);
    free(msg_part[parts-2]);
    free(msg_part);

    return rc;
}

int psend(int send_skt, char* msg){
    int rc;
    fd_set rfds;
    struct timeval tv;
    char message[BUFFER_SIZE];
    int length = strlen(msg)+1, part_len, part = 0;
    //int parts = ((length-1)/BUFFER_SIZE) + 1;
    pieces_t piece;
    
    while(1){
        while(BUFFER_SIZE*part < length){
            if(length - BUFFER_SIZE*part < BUFFER_SIZE){
                part_len = length - BUFFER_SIZE*part;
            }
            else{
                part_len = BUFFER_SIZE;
            }
            memcpy(piece.msg, msg+part*BUFFER_SIZE, part_len);
            piece.idx = part;
            rc = UDP_Write(send_skt, &addrSnd, &piece, sizeof(piece));
            // printf("Msg Sent : %s\n", piece.msg);
            if(rc < 0){
                perror("failed to send, retrying\n");
                continue;
            }
            part++;
        }
        memcpy(piece.msg, end_msg, end_msg_len+1);
        piece.idx = part;
        rc = UDP_Write(send_skt, &addrSnd, &piece, sizeof(piece));
        // printf("Msg Sent : prabhsimranshotshotmundakhunda\n");
        if(rc < 0){
            perror("failed to send, retrying\n");
            break;
        }
            
        tv.tv_sec = 500;
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
        // printf("Ack Received: size: %d, message: %s\n", rc, message);
        break;
    }

    return rc;
}
