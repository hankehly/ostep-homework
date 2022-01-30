#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/errno.h>
#include <sys/wait.h>

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
        // malloc returns a pointer to the allocated memory
        char *arg0 = malloc(sizeof(char) * 2);
        char *arg1 = malloc(sizeof(char) * 3);
        sprintf(arg0, "ls");
        sprintf(arg1, "-la");

        char *args[3];
        args[0] = arg0;
        args[1] = arg1;
        args[2] = NULL;

        // execvp will search the default PATH for you
        execvp(args[0], args);

        // execvP searches for the filename in the search path provided
        // execvP("ls", "/bin", args);
        // execvP(args[0], "/bin", args);

        // execve let's you pass in env vars
        // execve("/bin/ls", args, NULL);

        // execv requires the full path as the first argument
        // execv("/bin/ls", args);

        // execl takes a sequence of arguments instead of an array
        // execl("/bin/ls", "ls", "-la", NULL);

        // If you replaced ? with external variable environ, I bet this would be the same as execl
        // execle("/bin/ls", "ls", "`-la", ?);

        // free deallocates the memory allocation pointed to by ptr
        free(arg0);
        free(arg1);
    }
    else
    {
        wait(NULL);
    }
    return 0;
}