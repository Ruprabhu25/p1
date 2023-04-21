#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <ctype.h>
#include <sys/wait.h>

#define CMDLINE_MAX 512
#define NUM_ENV_VARS 26

char* env_vars[NUM_ENV_VARS];
struct node {
	char* val;
	struct node* next;
};
void freeList(struct node* head)
{
   struct node* current;

   while (head != NULL)
    {
       current = head;
       head = head->next;
       free(current);
    }

}
void printList(struct node *head)
{
    struct node *temp = head;

    //iterate the entire linked list and print the data
    while(temp != NULL)
    {
         printf("%s->", temp->val);
         temp = temp->next;
    }
}
void add_node(char* str_value, struct node **head, struct node **current) {
	// create new node with value of pattern
	struct node *newNode = (struct node*) malloc(sizeof(struct node));
	newNode->val = str_value;
	newNode->next = NULL;

	//if head is NULL, the list is empty
	if(*head == NULL) {
		*head = newNode;
		*current = *head;
	}
	//else, find the last node and add newNode
	else {
		//add the newNode at the end of the linked list
		(*current)->next = newNode;
		*current = (*current)->next;
	}
}
void print_arr(int args[], int size) {
        for (int i = 0; i<size; i++) {
                printf("%d ", args[i]);
        }
        printf("\n");
}
void print_status_arr(int status_arr[], int size) {
        for (int i = 0; i<size; i++) {
                printf("[%d]", status_arr[i]);
        }
        printf("\n");
}
char* trimwhitespace(char *str) {

        // Trim leading space
        while(isspace((unsigned char)*str)) str++;

        return str;
}
void cmd_cd(char* cmd, int num_args, char* args[]) {
        int cd_status = 0;
        int status;
        if (num_args == 1) { // command is "cd"
                cd_status = chdir("/");
        } else { // num_args  > 1
                cd_status = chdir(args[1]);
        }
        if (cd_status != 0) {
                fprintf(stderr, "Error: cannot cd into directory\n");
                status = 1;
        }
        else {
                status = 0;
        }
        fprintf(stderr, "+ completed '%s' [%d]\n",
                        cmd, status);
}
void cmd_set(char* cmd, int num_args, char* args[]) { // maybe could include in piping
        int status = 0;
	if (num_args < 2 && strlen(args[1]) != 1) {
                fprintf(stderr, "Error: invalid variable name\n");
        	status = 1;
	}
	else {
        	int index = args[1][0] - 97; 
        	if (index < 0 || index > 25) {
                	fprintf(stderr, "Error: invalid variable name\n");
                	status = 1;
                	return;
        	}   
        	else {
                	if (num_args == 2) { // set var to ""
                        	env_vars[index] = malloc(sizeof(char));
                        	env_vars[index] = "";
                	}
                	else { // have to fix for set with more than 3 args
                        	env_vars[index] = malloc(sizeof(args[2]));
                        	strcpy(env_vars[index],args[2]);
                	}
        	}
	}
        fprintf(stderr, "+ completed '%s' [%d]\n", cmd, status);
}
int forking(char* args[], int read_fd, int write_fd, int err_fd) {
        int status;
        pid_t pid;
        if (!(pid = fork())) {
                if (read_fd != 0) {
                        dup2(read_fd, STDIN_FILENO);
                        close(read_fd); // not needed anymore
                }
                if (write_fd != 1) {
                        if (err_fd != 2) {
                                dup2(err_fd, STDERR_FILENO);
                        }
                        dup2(write_fd, STDOUT_FILENO);
                        close(write_fd); // not needed anymore
                }
                if (err_fd != 2) {
                        dup2(err_fd, STDERR_FILENO);
                }
                execvp(args[0], args);
                fprintf(stderr, "Error: command not found\n");
                exit(1);
        } else {
                waitpid(pid, &status, 0);
                return status;
        }       
}
int linked_list(char* cmd, struct node** head, char* delimiter) {
        int num_args = 0;
        char cmd_copy[CMDLINE_MAX];
        strcpy(cmd_copy,cmd);
        char* token_args = strtok(cmd_copy, delimiter);

        // loop through the string to extract all other tokens
        while( token_args != NULL ) {
                // create new node with value of pattern
                struct node *newNode = (struct node*) malloc(sizeof(struct node));
                //check if token is an environment variable
                int len = strlen(token_args);
                if (token_args[0] == '$' && strcmp("|", delimiter) != 0) {
                        if (len != 2 && num_args == 0) {
                                fprintf(stderr, "Error: invalid variable name\n");
                        }
                        else if (strcmp(token_args, cmd) != 0) {
                                int index = token_args[1] - 97;
                                if (index >= 0 && index <= 25) {
                                        newNode->val = malloc(strlen(env_vars[index]));
                                        strcpy(newNode->val,env_vars[index]);
                                }
                                else {
                                        fprintf(stderr, "Error: invalid variable name\n");
                                        return -1;  
                                }
                        }
                        else {
                                fprintf(stderr, "Error: invalid variable name\n");
                                return -1;
                        }
                }
                else {
                        newNode->val = malloc(len + 1);
                        strcpy(newNode->val,token_args);
                }
                newNode->next = NULL;

                //if head is NULL, the list is empty
                if(*head == NULL) {
                        *head = newNode;
                }
                //else, find the last node and add newNode
                else {
                        //add the newNode at the end of the linked list
                        struct node *lastNode = *head;
			
                
                        //last node's next address will be NULL.
                        while(lastNode->next != NULL) {
                                lastNode = lastNode->next;
                        }
                        lastNode->next = newNode;
                }
                (num_args)++;
                token_args = strtok(NULL, delimiter);
        }              
        return num_args;
}
char** ll_to_arr(struct node* head, int num_args) {
        int arg_pos = 0;
        char** args = (char**) malloc(sizeof(char*) * (num_args+1));
        struct node* temp = head;
        while (temp != NULL) {
                args[arg_pos] = malloc(strlen(temp->val)+1);
                strcpy(args[arg_pos],temp->val); //CHECK OUT
                temp = temp->next;
                arg_pos++;
        }
        args[arg_pos] = NULL;
        return args;
}
void redirection(char* cmd, char* file_name) {
        int num_args;
        int redir_err = 0;
        struct node* head_arg = NULL;
        int length_redir = file_name - cmd+1; // find index of ">"
        char redir_cmd[length_redir];
        memcpy(redir_cmd, cmd, length_redir); // copy substring before ">" to redir_cmd
        redir_cmd[length_redir-1] = '\0'; //end string with null character
        if ((file_name + 1)[0] == '&') {
                redir_err = 1;
                file_name = trimwhitespace(file_name+2);
        }
        else {
                file_name = trimwhitespace(file_name+1);
        }
	if (strlen(file_name) == 0) {
		fprintf(stderr, "Error: no output file\n");
		return;
	}
        int fd;
        fd = open(file_name, O_CREAT | O_RDWR | O_TRUNC, 0644);
        if (fd < 0) {
		fprintf(stderr, "Error: cannot open output file\n");
		return;
	}
        num_args = linked_list(redir_cmd, &head_arg, " ");
        if (num_args == 0) {
		fprintf(stderr, "Error: missing command\n");
	}
	else {
        	char** args = ll_to_arr(head_arg,num_args);
            
        	int status;
        	if (redir_err == 1) {
                	status = forking(args, 0,fd,fd);
        	}   
        	else {
                	status = forking(args, 0,fd,2);
        	}   
        	fprintf(stderr, "+ completed '%s' [%d]\n",
                cmd, WEXITSTATUS(status));
        	close(fd);
	
	}
}
int find_redirection(char* cmd) {
        char *file_name;
        file_name = strchr(cmd, '>');
        if (file_name != NULL)  {
                redirection(cmd,file_name);
                return 1;
        }
        return 0;
}
void pipeline_helper(struct node** head_pipe, int** err_fd, int length) {
        // find if output is redirected, if so return true and adjust cmd
        *err_fd = malloc(length * sizeof(int));
        int count = 1;
        while (*head_pipe != NULL) {
                if (((*head_pipe)->val)[0] == '&') {
                        (*head_pipe)->val = trimwhitespace((*head_pipe)->val + 1);
                        (*err_fd)[count-1] = 1;
                }
                else {
                        (*err_fd)[count-1] = 0;
                }
                head_pipe = &((*head_pipe)->next);
                count++;
        }
	(*err_fd)[count-1] = 0;

}
int pipeline_general(char* cmd) {
	if (strchr(cmd,'|') == NULL) {
		return 0;
	}
        struct node* head_pipe = NULL;
        int num_commands = linked_list(cmd,&head_pipe,"|"); // find number of commands
        int* err_fd_arr;
        pipeline_helper(&(head_pipe->next),&err_fd_arr,num_commands);
        int i;
        int fd[2];
        int input_fd = 0; // for first child process, stdin is the default, we are not reading from other pipes
        int num_args;
        int status_arr[num_commands];
	int file_fd = 1;
        for (i = 0; i < num_commands; i++) {
                int std_err_fd = 2; // default
                struct node* head_arg = NULL;
                pipe(fd);
		char** args;
                if (head_pipe != NULL) {
			char* file_name;
                        file_name = strchr(head_pipe->val, '>');
			if (file_name != NULL) {
				if (i == num_commands - 1) {
					redirection(cmd, file_name);	
				}
				else {
					fprintf(stderr, "Error: mislocated output redirection\n");
					return 1;
				}
			}
			num_args = linked_list(head_pipe->val, &head_arg, " ");
                        args = ll_to_arr(head_arg,num_args);
                        head_pipe = head_pipe->next;
                }
		if (err_fd_arr[i] == 1) {
			std_err_fd = fd[1];
		}
                if (i == num_commands - 1) {
                        status_arr[i] = forking(args, input_fd,file_fd,std_err_fd);
                }
                else {
                        status_arr[i] = forking(args, input_fd, fd[1],std_err_fd);
                }
                close(fd[1]);
                input_fd = fd[0];
                freeList(head_arg);
                for (int i = 0; i < num_args; i++) {
                        free(args[i]);
                }
                free(args);
        }
        fprintf(stderr, "+ completed '%s' ",
                cmd);
        print_status_arr(status_arr, num_commands);
        return 1;
}
int main(void) {
        char cmd[CMDLINE_MAX];
        int i;
        for (i = 0; i < 26; i++) {
                env_vars[i] = malloc(CMDLINE_MAX*sizeof(char));
                env_vars[i] = "";
        }
        while (1) {
                char *nl;

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
                        fprintf(stderr, "+ completed '%s' [%d]\n",
                                                cmd, 0);
                        break;
                }
		/* Piped commands */
                if (pipeline_general(cmd)) {
                        continue;
                }
                /* Redirected commands*/
                else if (find_redirection(cmd)) {
                        continue;
                }
                else {/* Regular, single command */
                        struct node* head_arg = NULL;
			int num_args = linked_list(cmd, &head_arg, " ");
			if (num_args > 16) {
				fprintf(stderr, "Error: too many process arguments\n");
			}
			else if (num_args > 0) {
                                char** args = ll_to_arr(head_arg, num_args);
                                if (strcmp(args[0], "cd") == 0) {
                                        cmd_cd(cmd,num_args,args);
                                } 
                                else if (strcmp(args[0], "pwd") == 0) {
                                        char pwd_name[CMDLINE_MAX];
                                        getcwd(pwd_name, sizeof(pwd_name));
                                        printf("%s\n",pwd_name);
                                        fprintf(stderr, "+ completed '%s' [0]\n",
                                                cmd);
                                }
                                else if (strcmp(args[0], "set") == 0) {
                                        cmd_set(cmd,num_args,args);
                                }
                                else {
                                        int status = forking(args,0,1,2);
                                        fprintf(stderr, "+ completed '%s' [%d]\n",
                                                cmd, WEXITSTATUS(status));
                                }
                                for (int i = 0; i < num_args; i++) {
                                        free(args[i]);
                                }
                                free(args);
                        }
                        freeList(head_arg);
                }
        }
        return EXIT_SUCCESS;
}

