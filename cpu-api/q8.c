#include <unistd.h>
#include <sys/errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <fcntl.h>

int fork_or_exit()
{
    int rc = fork();
    if (rc < 0)
    {
        fprintf(stderr, "[%i] fork() failed (%i)\n", getpid(), errno);
        exit(1);
    }
    return rc;
}

// Mimic the following
// $ echo "hello\nworld" | wc -l
//     2
int main(int argc, char *argv[])
{
    char buffer_fn[] = "/tmp/tmp.ostep-cpu-api-q8";
    int rc_b = fork_or_exit();
    if (rc_b == 0)
    {
        fprintf(stdout, "[%i] i am b\n", getpid());
        close(STDOUT_FILENO);
        if (open(buffer_fn, O_WRONLY | O_CREAT | O_TRUNC, S_IRWXU) < 0)
        {
            fprintf(stderr, "[%i] open() failed (%i)\n", getpid(), errno);
        }
        // exec should now WRITE to above file
        execl("/bin/echo", "/bin/echo", "'ello\nworld'", NULL);
    }
    else
    {
        int rc_c = fork_or_exit();
        if (rc_c == 0)
        {
            fprintf(stdout, "[%i] i am c\n", getpid());
            close(STDIN_FILENO);
            if (open(buffer_fn, O_RDONLY, S_IRWXU) < 0)
            {
                fprintf(stderr, "[%i] open() failed (%i)\n", getpid(), errno);
            }
            // exec should now READ from the above file
            execl("/usr/bin/wc", "/usr/bin/wc", "-l", NULL);
        }
        else
        {
            fprintf(stdout, "[%i] i am a\n", getpid());

            // how am I creating this array without allocating memory?
            int desc[2];

            // How can I obtain the references to the file descriptors of each process?
            // int desc[2];
            // desc[0] = ?;
            // desc[1] = ?;
            // if (pipe(desc) < 0)
            // {
            //     fprintf(stderr, "[%i] pipe() failed (%i)\n", getpid(), errno);
            //     exit(1);
            // }
            if (wait(NULL) < 0)
            {
                fprintf(stderr, "[%i] wait() failed (%i)\n", getpid(), errno);
                exit(1);
            }
        }
    }
}