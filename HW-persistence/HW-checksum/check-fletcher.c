#include<stdio.h>
#include<stdlib.h>
#include"common.h"

int main(int argc, char *argv[]){

    if(argc != 2){
        fprintf(stderr, "Usage: ./check-fletcher [input_file]\n");
        exit(1);
    }

    FILE* file = fopen(argv[1], "r");
    assert(file != NULL);

    double stime,etime;
    stime = GetTime();
    u_int8_t s1=0, s2=0, i=0;

    while (fscanf (file, "%hhu", &i) == 1)
    {
        s1 = (s1 + i)%255;
        s2 = (s1 + s2)%255;
    }
    etime = GetTime();

    printf("Time taken by fletcher operation is %f sec and result checksum1 %hhu checksum2 %hhu\n", etime-stime, s1, s2);

    fclose(file);

    return 0;
}