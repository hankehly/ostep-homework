#include <unistd.h>
#include <sys/errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/uio.h>
#include <unistd.h>
#include <assert.h>

int fork_or_die()
{
    int rc = fork();
    assert(rc >= 0);
    return rc;
}

// Output:
// [5805] I am child 1
// [5804] I am the parent
// [5806] I am child 2
//       17
// [5804] Parent exiting
int main(int argc, char *argv[])
{
    int pipefd[2];
    assert(pipe(pipefd) == 0);

    pid_t pid_child1;
    pid_t pid_child2;

    // (the argument list should be terminated by a null pointer)
    char *cmd0[] = {"ls", "-la", NULL};
    char *cmd1[] = {"wc", "-l", NULL};

    if ((pid_child1 = fork_or_die()) == 0)
    {
        // Child 1 code (left-end of pipe)
        fprintf(stdout, "[%i] I am child 1\n", getpid());

        // Copy the pipe write-end fd to stdout (now stdout is set to write-end of pipe)
        dup2(pipefd[1], STDOUT_FILENO);

        // Commenting out the below 2 lines doesn't affect the output, but through experience
        // seems to reduce unexpected behavior
        close(pipefd[0]);
        close(pipefd[1]);

        // fprintf(stdout, "I am child 1, my PID is %i!", getpid());
        execvp(cmd0[0], cmd0);
    }
    else
    {
        if ((pid_child2 = fork_or_die()) == 0)
        {
            // Child 2 code (right-end of pipe)
            fprintf(stdout, "[%i] I am child 2\n", getpid());

            // Copy read-end pipe fd to stdin (now stdin is the pipe read-end)
            dup2(pipefd[0], STDIN_FILENO);

            // Note: Here and in the parent process, if you don't close the write-end of the pipe, the process
            // hangs at "I am child 2" – but it doesn't happen in child 1 ..
            close(pipefd[0]);
            close(pipefd[1]);

            execvp(cmd1[0], cmd1);
        }
        else
        {
            // Parent code
            close(pipefd[0]);
            close(pipefd[1]);

            fprintf(stdout, "[%i] I am the parent\n", getpid());

            waitpid(pid_child1, NULL, WUNTRACED);
            waitpid(pid_child2, NULL, WUNTRACED);

            fprintf(stdout, "[%i] Parent exiting\n", getpid());
        }
    }
    return 0;
}