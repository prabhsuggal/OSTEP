#include<stdio.h>
#include<stdlib.h>
#include<string.h>


int main(int argc,char *argv[]){

                
    switch(argc){
        case 1:{
                   printf("wzip: file1 [file2 ...]\n");
                   exit(1);
               }
        default:{
                    for(int i=1;i<argc;i++){
                        FILE* p = fopen(argv[i],"r");
                        if(p==NULL){
                            printf("wzip: cannot open file\n");
                            exit(1);
                        }
                        
                        char *buff = NULL;
                        size_t buffsize=32;

                        while(getline(&buff,&buffsize,p)!=-1){
                            char str;
                            int i=0,count=0;
                            str = buff[0];
                            while(i < buffsize){
                                if(str==buff[i]){
                                    count++;
                                    i++;
                                }
                                else{
                                    fwrite(&count,sizeof(count),1,stdout);
                                    printf("%d",count);
                                    printf("%c",str);
                                    str = buff[i];
                                    count=0;
                                }
                            }
                        }
                    }
                }
    }

    return 0;
}
