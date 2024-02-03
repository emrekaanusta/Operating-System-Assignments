#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>

int main(int argc, char *argv[]) {
    printf("I'm SHELL process, with PID: %d - Main command is man ls | grep -m 1 -A 1 -e '-d'\n", getpid());

    int fd[2];  // for write and read two pipes: child-parent
    pipe(fd);  // pipe declaration

    int rc = fork();

    if (rc < 0) {
        fprintf(stderr, "First fork failed... \n");
        exit(1);
    } else if (rc == 0) {
        // Child process
        int rc2 = fork();  // Second fork for the grandchild
        if (rc2 < 0) {
            fprintf(stderr, "Second fork failed...\n");
            exit(1);
        } else if (rc2 == 0) {
            // Grandchild process
            printf("I'm MAN process, with PID: %d - My command is man ls\n", (int)getpid());
            close(fd[0]);

            dup2(fd[1], STDOUT_FILENO);
            close(fd[1]);

            char *command[3];
            command[0] = "man";
            command[1] = "ls";  // my option
            command[2] = NULL;
            execvp(command[0], command);
        } else {
            // Child process
            wait(NULL);

            int output = open("output.txt", O_CREAT | O_WRONLY | O_TRUNC, S_IRWXU); // Use O_WRONLY instead of O_WRONLY


            printf("I'm GREP process with PID: %d - My command is grep -m 1 -A 1 -e '-d'\n", (int)getpid());

            dup2(fd[0], STDIN_FILENO);
            dup2(output, STDOUT_FILENO);
            
            close(output);

            close(fd[0]);
            close(fd[1]);

            char *commandChild[6];
            commandChild[0] = "grep";
            commandChild[1] = "-m 1";
            commandChild[2] = "-A 1";
            commandChild[3] = "-e";
            commandChild[4] = "-d";
            commandChild[5] = NULL;

            execvp(commandChild[0], commandChild);
        }
    } else {
        // Parent process (shell)
        wait(NULL);

        printf("I'm SHELL process, with PID: %d - Execution is completed, you can find the results in output.txt\n", (int)getpid());
    }

    return 0;
}
