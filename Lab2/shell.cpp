/****************
LE2: Introduction to Unnamed Pipes
****************/
#include <unistd.h> // pipe, fork, dup2, execvp, close
#include <sys/wait.h>
using namespace std;

int main()
{
    // lists all the files in the root directory in the long format
    char *cmd1[] = {(char *)"ls", (char *)"-al", (char *)"/", nullptr};
    // translates all input from lowercase to uppercase
    char *cmd2[] = {(char *)"tr", (char *)"a-z", (char *)"A-Z", nullptr};

    // Create pipe
    int fds[2];
    if (pipe(fds) == -1)
    {
        return 1;
    }

    // Create child to run first command
    pid_t c1 = fork();
    if (c1 == -1)
    {
        return 1;
    }

    if (c1 == 0)
    {
        // In child, redirect output to write end of pipe
        dup2(fds[1], STDOUT_FILENO);

        // Close the read end of the pipe on the child side.
        close(fds[0]);

        // In child, execute the command
        execvp(cmd1[0], cmd1);

        close(fds[1]);
        return 0;
    }

    // Create another child to run second command
    pid_t c2 = fork();
    if (c2 == -1)
    {
        return 1;
    }

    if (c2 == 0)
    {
        // In child, redirect input to the read end of the pipe
        dup2(fds[0], STDIN_FILENO);

        // Close the write end of the pipe on the child side.
        close(fds[1]);

        // Execute the second command.
        execvp(cmd2[0], cmd2);

        close(fds[0]);
        return 0;
    }

    // Reset the input and output file descriptors of the parent.
    close(fds[0]);
    close(fds[1]);

    // Wait for both children
    int status;
    waitpid(c1, &status, 0);
    waitpid(c2, &status, 0);

    return 0;
}
