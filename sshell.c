#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#define CMDLINE_MAX 512

int main(void)
{
        char cmd[CMDLINE_MAX];
        pid_t pid;
        const char* path = getenv("PATH");
        //printf("%s\n", path);
        //exit(0);
        //char *cmd = "/bin/echo";
        char *args[] = {cmd, "ECS150", NULL};
        int status;

        while (1) {
                char *nl;
                //int retval;

                /* Print prompt */
                printf("sshell$ ");
                fflush(stdout);

                /* Get command line */
                fgets(cmd, CMDLINE_MAX, stdin);

                /* Print command line if stdin is not provided by terminal */
                if (!isatty(STDIN_FILENO)) {
                        printf("%s", cmd);
                        fflush(stdout);
                }

                /* Remove trailing newline from command line */
                nl = strchr(cmd, '\n');
                if (nl)
                        *nl = '\0';

                /* Builtin command */
                if (!strcmp(cmd, "exit")) {
                        fprintf(stderr, "Bye...\n");
                        break;
                }

                /* Regular command */
                pid = fork();
                if (pid == 0) {
                        execv(cmd, args);
                        perror("execv");
                        exit(1);
                } else if (pid > 0) {
                        waitpid(pid, &status, 0);
                }
                //retval = system(cmd);
                fprintf(stdout, "Return status value for '%s': %d\n",
                        cmd, status);
        }

        return EXIT_SUCCESS;
}
