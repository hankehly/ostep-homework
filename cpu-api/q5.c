#include <stdio.h>
#include <sys/wait.h>
#include <unistd.h>
#include <sys/errno.h>
#include <stdlib.h>

int main(int argc, char *argv[])
{
    fprintf(stderr, "[%i] started\n", getpid());
    int rc = fork();
    if (rc < 0)
    {
        fprintf(stderr, "[%i] fork failed\n", getpid());
        exit(1);
    }
    else if (rc == 0)
    {
        // child
        fprintf(stdout, "[%i] sleeping for 1 second\n", getpid());
        sleep(1);

        // call wait() in child context
        // this fails with ECHILD(10) because there is no process to wait on
        fprintf(stdout, "[%i] gonna hang out for a while..\n", getpid());
        if (wait(NULL) < 0)
        {
            fprintf(stderr, "[%i] wait failed (%i)\n", getpid(), errno);
            exit(1);
        }
    }
    else
    {
        // wait returns the pid of the child process, or -1 on error
        int pid_c = wait(NULL);
        if (pid_c < 0)
        {
            fprintf(stderr, "[%i] wait failed\n", getpid());
            exit(1);
        }
        else
        {
            fprintf(stdout, "[%i] wait returned (%i)\n", getpid(), pid_c);
        }
    }
    return 0;
}