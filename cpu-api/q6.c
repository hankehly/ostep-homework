#include <stdio.h>
#include <sys/errno.h>
#include <sys/wait.h>
#include <unistd.h>
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
        fprintf(stdout, "[%i] sleeping for 1 second\n", getpid());
        sleep(1);
        fprintf(stdout, "[%i] done\n", getpid());
    }
    else
    {
        fprintf(stdout, "[%i] waiting for child process %i\n", getpid(), rc);
        waitpid(rc, NULL, WUNTRACED);
        fprintf(stdout, "[%i] child process %i done\n", getpid(), rc);
    }
    return 0;
}
