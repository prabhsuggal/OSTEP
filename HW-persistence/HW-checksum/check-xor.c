#include<stdio.h>
#include<stdlib.h>
#include"common.h"

int main(int argc, char *argv[]){

    if(argc != 2){
        fprintf(stderr, "Usage: ./check-xor [input_file]\n");
        exit(1);
    }

    FILE* file = fopen(argv[1], "r");
    assert(file != NULL);

    double stime,etime;
    stime = GetTime();
    u_int8_t xor = 0, i=0;

    while (fscanf (file, "%hhu", &i) == 1)
    {
        xor ^= i;
    }
    etime = GetTime();

    printf("Time taken by xor operation is %f sec and result checksum1 %hhu\n", etime-stime, xor);

    fclose(file);

    return 0;
}
