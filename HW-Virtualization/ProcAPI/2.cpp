#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <fcntl.h>
#include <assert.h>
#include <sys/wait.h>

int
main(int argc, char *argv[])
{
    int desc = open("./2.output", O_CREAT|O_WRONLY|O_TRUNC, S_IRWXU);
    printf("d:%d\n",desc);
    int rc = fork();
    if (rc < 0) {
        // fork failed; exit
        fprintf(stderr, "fork failed\n");
        exit(1);
    } else if (rc == 0) {
    // now exec "wc"...
        dprintf(desc,"Child is here, you punk");
    } else {
        // parent goes down this path (original process)

        dprintf(desc,"parent is here, you punk");
    }
    return 0;
}
