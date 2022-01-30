#include <stdio.h>
#include <unistd.h>
#include <sys/errno.h>
#include <sys/wait.h>
#include <unistd.h>
#include <stdlib.h>

int main(int argc, char *argv[])
{
    int rc = fork();
    if (rc == -1)
    {
        fprintf(stderr, "fork failed (%i)\n", errno);
        exit(1);
    }
    else if (rc == 0)
    {
        // child
        fprintf(stdout, "hello\n");
    }
    else
    {
        // parent
        wait(NULL);
        // sleep(1);
        fprintf(stdout, "goodbye\n");
    }
    return 0;
}
