#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include<string.h>


int
main(int argc, char *argv[])
{
    int p[2];

    char s[256];
    char* msg = "this is working";
    printf("%s\n",msg);
    int msglen = strlen(msg)+1;
    if(pipe(p)<0)
        exit(1);

    int rc=fork();
    if (rc < 0) {
        // fork failed; exit
        fprintf(stderr, "fork failed\n");
        exit(1);
    } else if (rc == 0) {
        // child (new process)

        printf("hello, I am child1 (pid:%d)\n", (int) getpid());
        if(close(p[0])==-1){
            printf("error closing reading end of pipe");
            exit(1);
        }
        if(write(p[1],msg,msglen)!=msglen){
            printf("error writing to pipe");
            exit(1);
        }

        if(close(p[1])==-1){
            printf("error closing writing end of pipe");
            exit(1);
        }
        
        printf("exiting child 1\n");
        
    } else {
        //wait(NULL);
        // parent goes down this path (original process)
        int rc1 = fork();

        if(rc1<0){
            printf("fuck, this is bad");
            exit(1);
        }
        else if(rc1==0){
            printf("This is child2 (pid:%d)\n", (int)getpid());

            if(close(p[1])==-1){
                printf("error closing writing end of pipe");
                exit(1);
            }

            while(read(p[0],s,msglen)>0)
                printf("%s:%d\n",s,msglen);
            
            if(close(p[0]==-1)){
                printf("error closing reading end of pipe");
            }
            printf("msg recieved from child 1:%s\n",s);
            printf("exiting child 2\n");

        }
        else{
            wait(NULL);
            wait(NULL);
            printf("our story ends,boy (pid:%d)\n",(int)getpid());
        }

    }
    return 0;
}
