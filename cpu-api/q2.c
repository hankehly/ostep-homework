#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/errno.h>
#include <sys/stat.h>
#include <unistd.h>
#include <assert.h>
#include <string.h>

int main(int argc, char *argv[])
{
    fprintf(stdout, "[pid:%i][0] argc=%i argv[0]=%s\n", getpid(), argc, argv[0]);
    int fd = open("/tmp/ostep-q2.txt", O_WRONLY | O_CREAT | O_TRUNC, S_IRWXU);
    if (fd == -1)
    {
        fprintf(stderr, "[pid:%i][1] open failed (%i)\n", getpid(), errno);
        exit(1);
    }

    int rc = fork();
    if (rc == -1)
    {
        fprintf(stderr, "[pid:%i][2] fork failed (%i)\n", getpid(), errno);
        exit(1);
    }
    else if (rc == 0)
    {
        // child
        fprintf(stdout, "[pid:%i][3] child context\n", getpid());
        write(fd, "child1\n", strlen("child1\n"));
        write(fd, "child2\n", strlen("child2\n"));
    }
    else
    {
        // parent (rc is pid of child)
        fprintf(stdout, "[pid:%i][6] parent of %i\n", getpid(), rc);
        write(fd, "parent1\n", strlen("parent1\n"));
        write(fd, "parent2\n", strlen("parent2\n"));
    }
    return 0;
}
