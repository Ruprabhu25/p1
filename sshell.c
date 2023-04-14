#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
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
	struct node *newNode = (struct node*) malloc(sizeof(struct node));
	newNode->val = str_value;
	newNode->next = NULL;
        printf("node insertion: %s\n", newNode->val);

	//if head is NULL, the list is empty
	if(*head == NULL) {
		*head = newNode;
                printf("head called\n");
		*current = *head;
	}
	//else, find the last node and add newNode
	else {
		//add the newNode at the end of the linked list
                printf("add to end\n");
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
void cmd_cd(char* cmd, int num_args, char* args[]) {
        int cd_status = 0;
        int status;
        if (num_args == 1) { // command is "cd"
                printf("home directory\n");
                cd_status = chdir("/");
        } else { // num_args  > 1
                printf("specific directory to %s\n", args[1]);
                cd_status = chdir(args[1]);
        }
        printf("cd status: %d\n", cd_status);
        if (cd_status != 0) {
                printf("Error: cannot cd into directory\n");
                status = 1;
        }
        else {
                status = 0;
        }
        fprintf(stdout, "+ completed '%s' [%d]\n",
                        cmd, status);
}
void forking(char* cmd, char* args[]) {
        printf(" made it to fork\n");
        int status;
        pid_t pid;
        pid = fork();
        if (pid == 0) {
                printf("child process\n");
                printf("arg0: %s\n", args[0]);
                execvp(args[0], args);
                fprintf(stderr, "Error: command not found\n");
                exit(1);
        } else if (pid > 0) {
                printf("parent process\n");
                waitpid(pid, &status, 0);
                fprintf(stdout, "+ completed '%s' [%d]\n",
                        cmd, WEXITSTATUS(status));
        }       
}
int linked_list(char* cmd, struct node** head_arg, struct node** current_arg) {
        printf("called linked list\n");
        int num_args = 0;
        char cmd_copy[CMDLINE_MAX];
        strcpy(cmd_copy,cmd);
        //printf("total before: %s\n", cmd_copy);
        char* token_args = strtok(cmd_copy, " ");
        // loop through the string to extract all other tokens
        while( token_args != NULL ) {
                //printf( "token: %s %lu \n", token_args,strlen(token_args) ); //printing each token
                char token[strlen(token_args)+1];
                strcpy(token,token_args);
                token[strlen(token_args)] = '\0';
                printf("token: %s\n", token);
                
                	// create new node with value of pattern
                struct node *newNode = (struct node*) malloc(sizeof(struct node));
                newNode->val = token;
                newNode->next = NULL;
                printf("node insertion: %s\n", newNode->val);

                //if head is NULL, the list is empty
                if(*head_arg == NULL) {
                        *head_arg = newNode;
                        printf("head called\n");
                        *current_arg = *head_arg;
                }
                //else, find the last node and add newNode
                else {
                        //add the newNode at the end of the linked list
                        printf("add to end\n");
                        (*current_arg)->next = newNode;
                        *current_arg = (*current_arg)->next;
                }
                printf("head arg: %s\n", (*head_arg) ->val);
                (num_args)++;
                token_args = strtok(NULL, " "); 
                //printf("current node: %s\n", current_arg->val);                        token_args = strtok(NULL, " ");
        }                //printf("current1: %s\n", current_arg->val);
        return num_args;
}
int main(void) {
        char cmd[CMDLINE_MAX];

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

                // output redirection
                char *file_name;
                file_name = strchr(cmd, '>');
                printf("test0\n");
                if (file_name != NULL)  {
                        // means we found a ">" character
                        printf("found a '>' at %s\n", file_name);
                        int length_redir = file_name - cmd; // find index of ">"
                        char redir_cmd[length_redir];
                        memcpy(redir_cmd, cmd, length_redir); // copy substring before ">" to redir_cmd
                        redir_cmd[length_redir] = '\0'; //end string with null character
                        file_name = file_name + 1;
                        printf("redir_cmd: %s, file_name: %s\n", redir_cmd, file_name);
                        int fd;
                        fd = open(file_name, O_CREAT | O_WRONLY | O_TRUNC, 0644);
                        printf("create fd\n");
                        dup2(fd, STDOUT_FILENO);
                        printf("dup2\n");
                        num_args = linked_list(redir_cmd, &head_arg, &current_arg);
                        char* args[(num_args)+1];
                        int arg_pos = 0;
                        //printf("args:\n");
                        //printf("current2: %s\n", current_arg->val);
                        printf("insert into args:\n");
                        while (head_arg != NULL) {
                                //printf("%d %s\n", arg_pos, head_arg->val);
                                //printf("current: %s\n", current_arg->val);
                                args[arg_pos] = head_arg -> val;
                                printf("head: %s args: %s\n", head_arg -> val, args[arg_pos]);
                                head_arg = head_arg->next;
                                arg_pos++;
                        }
                        args[arg_pos] = NULL;
                        printf("redir cmd: %s\n",redir_cmd);
                        //print_arr(args, num_args);
                        printf("forking now:\n");
                        forking(redir_cmd, args); 
                        close(fd);
                }
                exit(0);
                printf("test1\n");
                
                //printf("%d\n", num_args);
                //print_arr(args, num_args);
                //printf("%d\n", arg_pos);
                //printf("command: %s\n", cmd);
                //char* path = getenv("PATH");
                //char* token_path = strtok(path, ":");
                // loop through the string to extract all other tokens
                //check if it is a built in command, then we don't have to fork
                num_args = linked_list(cmd, &head_arg, &current_arg);
                char* args[(num_args)+1];
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
                printf("test2\n");
                if (strcmp(args[0], "cd") == 0) {
                        cmd_cd(cmd,num_args,args);
                } else if (strcmp(args[0], "pwd") == 0) {
                        char pwd_name[1024];
                        getcwd(pwd_name, sizeof(pwd_name));
                        printf("%s\n",pwd_name);
                }
                else {
                        printf("args[0]: %s", args[0]);
                        forking(cmd,args);
                }
                head_arg = NULL;
        }
        return EXIT_SUCCESS;
}

