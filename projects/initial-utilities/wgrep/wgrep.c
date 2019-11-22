#include<stdio.h>
#include<stdlib.h>
#include<stdbool.h>
#include<string.h>


void computeLPS(char* pat, int N, int* lps ){

    int i=1,len=0;
    lps[0]=0;
    
    while(i < N){
        if(pat[i] == pat[len]){
            len++;
            lps[i] = len;
            i++;
        }
        else{
            if(len!=0)
                len = lps[len-1];
            else{
                lps[i]=0;
                i++;
            }
        }
    }

}

bool PatMatch(char* txt, char * pat){
    int M = strlen(txt);
    int N = strlen(pat);

    int lps[N];
    computeLPS(pat, N, lps);

    int i=0,j=0;
    
    while(i < M){
        if(txt[i] == pat[j]){
            i++;
            j++;
        }

        if(j == N){
            return true;
        }

        else if( i < M && txt[i] != pat[j] ){
            if(j != 0 )
                j = lps[j-1];
            else
                i++;
        }
    }
    return false;
}



int main(int argc,char *argv[]){

                
    switch(argc){
        case 1:{
                   printf("wgrep: searchterm [file ...]\n");
                   exit(1);
               }
        case 2:{
                   char* pat =argv[1];
                   char* buff = NULL;
                   size_t buffsize=32;

                   while(getline(&buff,&buffsize,stdin)!=-1){
                       if(PatMatch(buff,pat)){
                           printf("%s",buff);
                       }
                   }
                   exit(0);
               }
        default:{
                    char* pat =argv[1];
                    for(int i=2;i<argc;i++){
                        FILE* p = fopen(argv[i],"r");
                        if(p==NULL){
                            printf("wgrep: cannot open file\n");
                            exit(1);
                        }
                        
                        char *buff = NULL;
                        size_t buffsize=32;

                        while(getline(&buff,&buffsize,p)!=-1){
                            if(PatMatch(buff,pat)){
                                printf("%s",buff);
                            }
                        }
                        
                    }
                }
    }

    return 0;
}
