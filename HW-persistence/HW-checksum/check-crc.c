#include<stdio.h>
#include<stdlib.h>
#include"common.h"

int highest_set_bit(int x){
    int msb = 0;
    while( x >>= 1 )
        msb++;
    return msb;
}

int main(int argc, char *argv[]){

    if(argc < 2){
        fprintf(stderr, "Usage: ./check-crc [input_file] (optional: [check_sum generator(default: 69665)])\n");
        exit(1);
    }

    uint32_t signal=0, generator = 0b10001000000100001;
    if(argc == 3){
        generator = atoi(argv[2]);
    }


    FILE* file = fopen(argv[1], "r");
    assert(file != NULL);

    double stime,etime;
    stime = GetTime();
    uint16_t crc = 0;

    uint8_t bits_read, bit_flag = 0, data = 0, crc_bits = highest_set_bit(generator);

    while (fscanf (file, "%hhu", &data) == 1)
    {
        bits_read = 0;
        while(bits_read < 8){
            signal <<= 1;
            signal |= (data >> (7 - bits_read++)) & 1;

            bit_flag  = signal >> (crc_bits - 1);

            if(bit_flag)
                signal = signal ^ generator;

        }
    }
    crc = signal;
    etime = GetTime();

    printf("Time taken by crc operation is %f sec and result checksum %hu\n", etime-stime, crc);

    fclose(file);

    return 0;
}