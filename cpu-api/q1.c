#include <stdio.h>
#include <unistd.h>
#include <sys/errno.h>
#include <stdlib.h>

int main(int argc, char *argv[])
{
    int x = 100;
    fprintf(stdout, "[%i][1] %i\n", getpid(), x);
    int rc = fork();
    if (rc == -1)
    {
        fprintf(stderr, "[%i][2] fork failed\n", getpid());
        exit(1);
    }
    else if (rc == 0)
    {
        // child context
        fprintf(stdout, "[%i][3] %i\n", getpid(), x);
        x = 200;
        fprintf(stdout, "[%i][4] %i\n", getpid(), x);
    }
    else
    {
        // parent context
        fprintf(stdout, "[%i][5] %i\n", getpid(), x);
        x = 300;
        fprintf(stdout, "[%i][6] %i\n", getpid(), x);
    }
    // this next line gets called twice!
    fprintf(stdout, "[%i][7] %i\n", getpid(), x);
    return 0;
}