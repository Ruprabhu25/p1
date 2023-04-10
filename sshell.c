#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#define CMDLINE_MAX 512
struct node {
	char* val;
	struct node* next;
};

struct node *head_arg = NULL;
struct node *current_arg = NULL;

struct node *head_path = NULL;
struct node *current_path = NULL;

void add_node(char* str_value, struct node **head, struct node **current) {
	// create new node with value of pattern
	struct node *newNode = malloc(sizeof(struct node));
	newNode->val = str_value;
	newNode->next = NULL;

	//if head is NULL, the list is empty
	if(*head == NULL) {
		*head = newNode;
		*current = *head;
	}
	//else, find the last node and add newNode
	else
	{
		//add the newNode at the end of the linked list
		(*current)->next = newNode;
		current = &((*current)->next);
	}
}

int main(void)
{
        char cmd[CMDLINE_MAX];
        pid_t pid;
        //const char* path = getenv("PATH");
        //printf("%s\n", path);
        //exit(0);
        //char *cmd = "/bin/echo";
        //char *args[] = {cmd, "ECS150", NULL};
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
                char* token_args = strtok(cmd, " ");
                // loop through the string to extract all other tokens
                while( token_args != NULL ) {
                        add_node(token_args, &head_arg, &current_arg);
                        printf( " %s\n", token_args ); //printing each token
                        token_args = strtok(NULL, " ");
                }
                printf("command: %s\n", cmd);
                char* path = getenv("PATH");
                char* token_path = strtok(path, ":");
                // loop through the string to extract all other tokens
                while( token_path != NULL ) {
                        //add_node(token_path, &head_path, &current_path);
                        //printf( " %s\n", token_path ); //printing each token
                        printf("token_path: %s\n", token_path);
                        char pathCmd[4096];
                        snprintf(pathCmd, 4096,"%s%s%s", token_path, "/", cmd);
                        //char* cmd_str = strcat(token_path, "/");
                        //cmd_str =  strcat(cmd_str, head_arg->val);
                        printf("pathCmd: %s\n", pathCmd);
                        token_path = strtok(NULL, ":");
                }
                //char* path_cmd = strcat(path, cmd);
                //printf("%s\n", path_cmd);
                pid = fork();
                if (pid == 0) {
                        /*total_command = cat(path /cmd
                        args = {total_command, }
                        execvp(cmd, args);
                        execvp(cat(cmd), {cmd, });
                        perror("execv");*/
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

