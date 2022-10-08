#include <stdio.h>
#include <sys/time.h>
#include <fcntl.h>
#include <unistd.h>

int main(int argc, char *argv[])
{
    // measure system call
    // open a file, read 0 bytes into null buffer (basic system call) 1 million times
    // check average and total time
    int fd = open("./_tmp", O_WRONLY | O_CREAT | O_TRUNC);
    int n_loops = 1000000;
    struct timeval start, end;

    gettimeofday(&start, NULL);
    for (size_t i = 0; i < n_loops; ++i)
    {
        read(fd, NULL, 0);
    }
    gettimeofday(&end, NULL);
    close(fd);

    long start_us = start.tv_sec * 1000000 + start.tv_usec;
    long end_us = end.tv_sec * 1000000 + end.tv_usec;
    printf("%ld microseconds (avg)\n", (end_us - start_us) / n_loops);
    printf("%ld microseconds (total)\n", end_us - start_us);

    // return 0;

    // todo: measure context switch (cpu affinity on a mac?)
}
