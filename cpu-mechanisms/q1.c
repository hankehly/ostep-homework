#include <stdio.h>
#include <sys/time.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <sys/wait.h>

int main(int argc, char *argv[])
{
    // 1. Measure cost of system call
    printf("1. Measure cost of system call\n");

    // open a file, read 0 bytes into null buffer (basic system call) 1 million times
    // check average and total time
    int fd = open("./_tmp", O_WRONLY | O_CREAT | O_TRUNC);
    int n_loops = 10000;
    struct timeval start, end;

    gettimeofday(&start, NULL);
    for (size_t i = 0; i < n_loops; ++i)
    {
        read(fd, NULL, 0);
    }
    gettimeofday(&end, NULL);
    close(fd);
    long start_us = start.tv_sec * n_loops + start.tv_usec;
    long end_us = end.tv_sec * n_loops + end.tv_usec;
    // careful, dividing by an integer will truncate the result (getting zero)
    // 10,000 iterations:
    // macOS
    // 0.315500 microseconds (avg)
    // 3155 microseconds (total)
    // debian (aws t2.micro)
    // 0.503900 microseconds (avg)
    // 5039 microseconds (total)
    printf("%f microseconds (avg)\n", (end_us - start_us) / (float)n_loops);
    printf("%ld microseconds (total)\n", end_us - start_us);

    //
    // 2. Measure cost of context switch
    //
    printf("\n2. Measure cost of context switch\n");

    int n_iter = 10000;
    // struct timeval start, end;
    gettimeofday(&start, NULL);
    // Create two pipes
    int pipe1[2], pipe2[2];
    if (pipe(pipe1) == -1 || pipe(pipe2) == -1)
    {
        perror("pipe");
        exit(EXIT_FAILURE);
    }

    // Create two children
    pid_t pid1, pid2;
    pid1 = fork();
    if (pid1 == -1)
    {
        perror("fork");
        exit(EXIT_FAILURE);
    }
    else if (pid1 == 0)
    {
        /* child1 */
        printf("child1\n");

        // pipe1 producer, pipe2 consumer
        close(pipe1[0]);
        close(pipe2[1]);

        // child1 writes to first, then reads on second
        for (size_t i = 0; i < n_iter; i++)
        {
            int x = 1;
            if (write(pipe1[1], &x, sizeof(x)) == -1)
            {
                perror("write");
                exit(EXIT_FAILURE);
            }
            // printf("[%i] child 1 wrote to pipe1: %i\n", i, x);
            int y;
            if (read(pipe2[0], &y, sizeof(y)) == -1)
            {
                perror("read");
                exit(EXIT_FAILURE);
            }
            // printf("[%i] child 1 read from pipe2: %i\n", i, y);
        }
    }
    else
    {
        /* parent */
        pid2 = fork();
        if (pid2 == -1)
        {
            perror("fork");
            exit(EXIT_FAILURE);
        }
        else if (pid2 == 0)
        {
            /* child2 */
            printf("child 2\n");

            // pipe1 consumer, pipe2 producer
            close(pipe1[1]);
            close(pipe2[0]);

            // child2 reads on first, then writes to second
            for (size_t i = 0; i < n_iter; i++)
            {
                int y;
                if (read(pipe1[0], &y, sizeof(y)) == -1)
                {
                    perror("read");
                    exit(EXIT_FAILURE);
                }
                // printf("[%i] child 2 read from pipe1: %i\n", i, y);
                int x = 1;
                if (write(pipe2[1], &x, sizeof(x)) == -1)
                {
                    perror("write");
                    exit(EXIT_FAILURE);
                }
                // printf("[%i] child 2 wrote to pipe2: %i\n", i, x);
            }
        }
        else
        {
            /* parent */
            close(pipe1[0]);
            close(pipe1[1]);
            close(pipe2[0]);
            close(pipe2[1]);

            waitpid(pid1, NULL, WUNTRACED);
            waitpid(pid2, NULL, WUNTRACED);

            gettimeofday(&end, NULL);

            // Single core debian (aws t2.micro)
            // 6.281410 microseconds (avg)
            // 628141 microseconds (total)
            // Don't know how to set CPU affinity on a mac..
            long start_us = start.tv_sec * n_iter + start.tv_usec;
            long end_us = end.tv_sec * n_iter + end.tv_usec;
            printf("%f microseconds (avg)\n", (end_us - start_us) / (float)n_iter);
            printf("%ld microseconds (total)\n", end_us - start_us);
        }
    }
    return 0;
}
