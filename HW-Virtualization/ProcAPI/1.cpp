#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

int main(int argc, char *argv[])
{
    printf("hello (pid:%d)\n", (int) getpid());
    int x =100;
    printf("value of x : %d\n",x);
    int rc = fork();
    if (rc < 0) {
        // fork failed; exit
        fprintf(stderr, "fork failed\n");
        exit(1);
    } else if (rc == 0) {
        // child (new process)
        printf("hello, I am child (pid:%d)\n", (int) getpid());
        x=120;
        printf("value of x : %d\n",x);
    } else {
        // parent goes down this path (original process)
        printf("hello, I am parent of %d (pid:%d)\n",
           rc, (int) getpid());
        x=130;
        printf("value of x : %d\n",x);
    }
    return 0;
}
