#include "udp.h"

#define BUFFER_SIZE (1000)

struct sockaddr_in addrSnd, addrRecv;

int Channel_Init(int port_num){
    return UDP_Open(port_num);
}

int Connect_to(char *server, int server_port){
    return UDP_FillSockAddr(&addrSnd, server, server_port);
}

int preceive(int listen_skt,  char* message){
	int rc = UDP_Read(listen_skt, &addrRecv, message, BUFFER_SIZE);
    addrSnd = addrRecv;
    char ack[BUFFER_SIZE] = "Message Received";
    UDP_Write(listen_skt, &addrSnd, ack, BUFFER_SIZE);

    return rc;
}

int psend(int send_skt, char* msg){
    int rc;
    fd_set rfds;
    struct timeval tv;
    char message[BUFFER_SIZE];
    while(1){
        rc = UDP_Write(send_skt, &addrSnd, msg, BUFFER_SIZE);
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
