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
    pid_t pid_child1, pid_child2;

    // Note:
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
    // The following shows 122880 on macOS (that's a lot of file descriptors)
    fprintf(stdout, "The size of the descriptor table is %i\n", getdtablesize());

    if ((pid_child1 = fork_or_die()) == 0)
    {
        // Child 1 code (producer)
        fprintf(stdout, "[%i] I am child 1\n", getpid());

        // We aren't reading from the pipe in this process, so close the read end.
        close(pipefd[0]);

        // Tell the OS use the write-end of the pipe as standard out by
        // copying the pipe's write-end fd to the standard output fd location.
        dup2(pipefd[1], STDOUT_FILENO);

        // Closing the original pipe file descriptors here does not affect stdout because
        // we already copied the pipe's write-end file descriptor to the stdout location.
        // This code will just close the original file descriptors.
        close(pipefd[1]);

        // Execute a command, write to the pipe sink (which is set to stdout in this child process)
        // Running execvp will replace the current "process image" so you can't continue to run
        // commands afterwards (the process content has been changed to cmd0!)
        execlp("ls", "ls", "-la", NULL);
    }
    else
    {
        if ((pid_child2 = fork_or_die()) == 0)
        {
            // Child 2 code (consumer)
            fprintf(stdout, "[%i] I am child 2\n", getpid());

            // Why does not closing the write-end of the pipe cause the program to hang at execvp?
            // If write end is not closed, read will keep on hanging, but what is reading from child2?
            close(pipefd[1]);

            // Copy read-end pipe fd to stdin (now stdin is the pipe read-end)
            dup2(pipefd[0], STDIN_FILENO);

            // We're done with the read end of the pipe (copied to stdin), so close it to
            // release resources.
            close(pipefd[0]);

            execlp("wc", "wc", "-l", NULL);

            // Note: This won't even get executed because execvp replaces the process image
            // fprintf(stdout, "foo\n");
        }
        else
        {
            // Parent code
            // 
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