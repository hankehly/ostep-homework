#include <unistd.h>
#include <stdio.h>
#include <sys/errno.h>
#include <sys/wait.h>
#include <stdlib.h>

int main(int argc, char *argv[])
{
    int rc = fork();
    if (rc < 0)
    {
        fprintf(stderr, "[%i] fork failed (%i)\n", getpid(), errno);
        exit(1);
    }
    else if (rc == 0)
    {
        fprintf(stdout, "[%i] child here. closing STDOUT_FILENO now\n", getpid());
        close(STDOUT_FILENO);
        fprintf(stdout, "[%i] hello?\n", getpid());
        fprintf(stdout, "[%i] is anybody there?\n", getpid());
    }
    else
    {
        fprintf(stdout, "[%i] wait for child\n", getpid());
        if (wait(NULL) == -1)
        {
            fprintf(stderr, "[%i] wait() failed (%i)\n", getpid(), errno);
            exit(1);
        }
        fprintf(stdout, "[%i] the wait() is over\n", getpid());
    }
    return 0;
}