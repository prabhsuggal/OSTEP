#include<stdio.h>
#include<stdlib.h>
#include<string.h>

char EXIT[]="exit\n";
#define NUM_OF_ARGS 20

void operate(char* buffer){
    char **args = (char**)malloc(NUM_OF_ARGS*sizeof(char*));
    char *arg;
    int i=0;

    while((arg = strsep(&buffer," ")) != NULL){
        args[i] = (char*)malloc((strlen(arg)+1)*sizeof(char));
        strcpy(args[i],arg);

        printf("%s ",args[i]);
        i++;
    }



}


void interactive(){

    char *buffer;
    size_t bufsize = 32,len;

    buffer = (char*)malloc(bufsize * sizeof(char));
    while(1){
        printf("wish> ");
        len = getline(&buffer, &bufsize, stdin);

        if(strcmp(buffer,EXIT) == 0){
            exit(0);
        }
        
        operate(buffer);

        //printf("%zu,%s", len, buffer);

    }

    free(buffer);
    return;
}

void batch(char* file){
    printf("%s\n",file);

    FILE *fp = fopen(file,"r");
    if(!fp){
        printf("Error while Openin file: %s\n", file);
        exit(-1);
    }

    size_t bufsize = 32;
    int len;
    char* buffer = (char*)malloc(sizeof(char)*bufsize);

    len = getline(&buffer, &bufsize, fp);

    while(len >= 0){
    //    printf("%d,%s", len, buffer);
        
        if(strcmp(buffer,EXIT) == 0){
            exit(0);
        }
        
        operate(buffer);

        len = getline(&buffer,&bufsize,fp);

    }

    return;

}

int main(int argc, char* argv[]){

    if(argc == 1){
        interactive();
    }
    else if(argc == 2)
        batch(argv[1]);

    return 0;
}
