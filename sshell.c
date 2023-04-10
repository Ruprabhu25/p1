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
		*current = (*current)->next;
	}
}
void print_arr(char* args[], int size) {
        printf("array of args: ");
        for (int i = 0; i<size; i++) {
                printf("%s ", args[i]);
        }
        printf("\n");
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
                int num_args = 0;

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
                        //printf( "token: %s\n", token_args ); //printing each token
                        add_node(token_args, &head_arg, &current_arg);
                        num_args++;
                        token_args = strtok(NULL, " "); 
                        //printf("current node: %s\n", current_arg->val);                        token_args = strtok(NULL, " ");
                }
                //printf("current1: %s\n", current_arg->val);
                char* args[num_args+1];
                int arg_pos = 0;
                //printf("args:\n");
                //printf("current2: %s\n", current_arg->val);
                while (head_arg != NULL) {
                        //printf("%d %s\n", arg_pos, head_arg->val);
                        //printf("current: %s\n", current_arg->val);
                        args[arg_pos] = head_arg -> val;
                        head_arg = head_arg->next;
                        arg_pos++;
                }
                args[arg_pos] = NULL;
                //printf("%d\n", num_args);
                //print_arr(args, num_args);
                //printf("%d\n", arg_pos);
                //printf("command: %s\n", cmd);
                //char* path = getenv("PATH");
                //char* token_path = strtok(path, ":");
                // loop through the string to extract all other tokens
                pid = fork();
                if (pid == 0) {
                        execvp(args[0], args);
                        //perror("execvp");
                        exit(0);
                } else if (pid > 0) {
                        waitpid(pid, &status, 0);
                }
                head_arg = NULL;
                //}
                //retval = system(cmd);
                fprintf(stdout, "Return status value for '%s': %d\n",
                        cmd, status);
        }
        return EXIT_SUCCESS;
}

