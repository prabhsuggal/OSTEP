#include<stdio.h>
#include<stdlib.h>

int main(int argc, char* argv[]){
    
    for(int i=1;i<argc;i++){
        FILE* p = fopen(argv[i],"r");
        if(p==NULL){
            printf("wcat: cannot open file\n");
            exit(1);
        }
        else{
            char str[1000];
            while(fgets(str,1000,p)!=NULL){
                printf("%s",str);
            }
        }
        fclose(p);
    }
    
    return 0;
}
