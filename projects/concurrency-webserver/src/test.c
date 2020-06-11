#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>
#include <stdio.h>
#include <assert.h>

int main(){
    int fd = open("list", O_RDONLY);
    
    struct stat buf;
    assert(fstat(fd,&buf)==0);
    printf("%d\n",buf.st_size);
    return 0;
}
