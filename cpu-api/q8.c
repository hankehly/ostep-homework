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

    // Each process has a fixed size file descriptor table.
    // Entries are numbered as integers starting at 0.
    // The following code prints:
    // 0 - STDIN_FILENO
    // 1 - STDOUT_FILENO
    // 2 - STDERR_FILENO
    // 3 - pipe (read end)
    // 4 - pipe (write end)
    // Standard in/out/err are all file descriptors in this desciptor table, and
    // When we create 2 new descriptors via pipe() we get 3 (read end) and 4 (write end)
    fprintf(stdout, "%i - STDIN_FILENO\n", STDIN_FILENO);
    fprintf(stdout, "%i - STDOUT_FILENO\n", STDOUT_FILENO);
    fprintf(stdout, "%i - STDERR_FILENO\n", STDERR_FILENO);
    fprintf(stdout, "%i - pipe (read end)\n", pipefd[0]);
    fprintf(stdout, "%i - pipe (write end)\n", pipefd[1]);
    // The following shows 122880 (that's a lot of file descriptors)
    fprintf(stdout, "The size of the descriptor table is %i\n", getdtablesize());

    // (the argument list should be terminated by a null pointer)
    char *cmd0[] = {"ls", "-la", NULL};
    char *cmd1[] = {"wc", "-l", NULL};

    if ((pid_child1 = fork_or_die()) == 0)
    {
        // Child 1 code (write-end of pipe)
        fprintf(stdout, "[%i] I am child 1\n", getpid());

        // Copy the pipe's write-end value to the standard output value.
        // Now the program will use the pipe write-end as the output sink.
        dup2(pipefd[1], STDOUT_FILENO);

        // Closing the original pipe file descriptors here does not affect stdout because
        // we already copied the pipe's write-end file descriptor to the stdout location.
        // This code will just close the original file descriptors.
        close(pipefd[0]);
        close(pipefd[1]);

        // Execute a command, write to the pipe sink (which is set to stdout in this child process)
        // Running execvp will replace the current "process image" so you can't continue to run
        // commands afterwards (the process content has been changed to cmd0!)
        execvp(cmd0[0], cmd0);
    }
    else
    {
        if ((pid_child2 = fork_or_die()) == 0)
        {
            // Child 2 code (read-end of pipe)
            fprintf(stdout, "[%i] I am child 2\n", getpid());

            // Copy read-end pipe fd to stdin (now stdin is the pipe read-end)
            dup2(pipefd[0], STDIN_FILENO);

            close(pipefd[0]);
            // Why does not closing the write-end of the pipe cause the program to hang at execvp?
            close(pipefd[1]);

            execvp(cmd1[0], cmd1);

            // We could alternatively print the output of child1 execvp by reading from the
            // pipe's read-end (which is copied to to stdin above)
            // char *buf[50];
            // read(STDIN_FILENO, buf, sizeof(buf));
            // write(STDOUT_FILENO, buf, sizeof(buf));

            // Note: This won't even get executed because execvp replaces the process image
            // fprintf(stdout, "foo\n");
        }
        else
        {
            // Parent code
            close(pipefd[0]);
            // Why does not closing the write-end of the pipe cause the program to hang at child2 execvp?
            close(pipefd[1]);

            fprintf(stdout, "[%i] I am the parent\n", getpid());

            waitpid(pid_child1, NULL, WUNTRACED);
            waitpid(pid_child2, NULL, WUNTRACED);

            fprintf(stdout, "[%i] Parent exiting\n", getpid());
        }
    }
    return 0;
}