#ifndef __common_h__
#define __common_h__

#include <sys/time.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <assert.h>
#include <unistd.h>
#include <sys/syscall.h>
#include <fcntl.h>
#define gettid() syscall(SYS_gettid)
#define Fstat(fd, sbuf)         assert(fstat(fd,sbuf) >= 0)

int Open(char *file, int flags){
    int fd = open(file, flags);
    assert(fd >= 0);
    return fd;
}

double GetTime() {
    struct timeval t;
    int rc = gettimeofday(&t, NULL);
    assert(rc == 0);
    return (double) t.tv_sec + (double) t.tv_usec/1e6;
}

void Spin(int howlong) {
    double t = GetTime();
    while ((GetTime() - t) < (double) howlong)
	; // do nothing in loop
}

#endif // __common_h__
