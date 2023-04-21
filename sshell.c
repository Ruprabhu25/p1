#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <ctype.h>
#include <sys/wait.h>

#define CMDLINE_MAX 512
#define NUM_ENV_VARS 26

// make a global array of environment variables strings
char* env_vars[NUM_ENV_VARS];
// node struct to hold string values (commands or arguments)
struct node {
	char* val;
	struct node* next;
};
// function to free linked list
void freeList(struct node* head)
{
        struct node* current;

        // iterate through each node starting at the head
        // free each node as we iterate
        while (head != NULL) {
                current = head;
                head = head->next;
                free(current);
        }

}
// helper function to print the respective statuses of pipeline commands
void print_status_arr(int status_arr[], int size) {
        for (int i = 0; i<size; i++) {
                printf("[%d]", status_arr[i]);
        }
        printf("\n");
}
// trim the leading and ending whitespace
char* trimwhitespace(char *file_name) {
        // Trim leading space
        while(isspace((unsigned char)*file_name)) file_name++;

        // Trim trailing space
        char* end = file_name + strlen(file_name) - 1;
        while(end > file_name && isspace((unsigned char)*end)) end--;

        // write null character at the end of the filename
        end[1] = '\0';

        return file_name;
}
// function for cd command
void cmd_cd(char* cmd, int num_args, char* args[]) {
        int cd_status = 0;
        int status;
        // if command is simply cd, navigate to home directory
        if (num_args == 1) { 
                cd_status = chdir("/");
        // else, attempt to navigate to the directory specified
        } else { 
                cd_status = chdir(args[1]);
        }
        // check status to determine if an error occurred
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
// function for set command
void cmd_set(char* cmd, int num_args, char* args[]) {
        int status = 0;
        // check if if the number of arguments and variable name is valid
	if (num_args < 2 && strlen(args[1]) != 1) {
                fprintf(stderr, "Error: invalid variable name\n");
        	status = 1;
	}
	else {
                // use ASCII to store a-z variables in indices 0-25
        	int index = args[1][0] - 97; 
                // check if index falls within acceptable range
        	if (index < 0 || index > 25) {
                	fprintf(stderr, "Error: invalid variable name\n");
                	status = 1;
                	return;
        	}   
        	else {
                	if (num_args == 2) { // set environment variable to ""
                        	env_vars[index] = malloc(sizeof(char));
                        	env_vars[index] = "";
                	}
                	else { // set environment variable to the third argument string value
                        	env_vars[index] = malloc(sizeof(args[2]));
                        	strcpy(env_vars[index],args[2]);
                	}
        	}
	}
        fprintf(stderr, "+ completed '%s' [%d]\n", cmd, status);
}
// forking function to execute child processes
// takes set of arguments for execvp and the file descriptors of standard streams
int forking(char* args[], int read_fd, int write_fd, int err_fd) {
        int status;
        pid_t pid;
        // forks and checks if pid = 0 (child), if so execute 
        if (!(pid = fork())) {
                // check if read, write, and err descriptors are not standard
                // if they are not standard, then use dup2() to redirect stream
                if (read_fd != 0) {
                        dup2(read_fd, STDIN_FILENO);
                        close(read_fd); // not needed anymore
                }
                if (write_fd != 1) {
                        if (err_fd != 2) {
                                dup2(err_fd, STDERR_FILENO);
                        }
                        dup2(write_fd, STDOUT_FILENO);
                        close(write_fd);
                }
                if (err_fd != 2) {
                        dup2(err_fd, STDERR_FILENO);
                        close(err_fd);
                }
                // call execvp after all streams have been redirected
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
        // create copy of command to modify via strtok
        strcpy(cmd_copy,cmd);
        char* token_args = strtok(cmd_copy, delimiter);

        // loop through the command string to extract all other tokens
        while( token_args != NULL ) {
                // create new node with value of pattern
                struct node *newNode = (struct node*) malloc(sizeof(struct node));
                // check if token is an environment variable
                int len = strlen(token_args);
                // also check for no piping, this only applies to argument linked lists
                if (token_args[0] == '$' && strcmp("|", delimiter) != 0) {
                        // check that the variable fits the parameters
                        if (len != 2 && num_args == 0) {
                                fprintf(stderr, "Error: invalid variable name\n");
                        }
                        // check that this isn't the first token_arg, then find index if so
                        else if (strcmp(token_args, cmd) != 0) {
                                int index = token_args[1] - 97;
                                if (index >= 0 && index <= 25) {
                                        // allocate space for copying the string to node
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
                        // allocate space for copying the string to node
                        newNode->val = malloc(len + 1);
                        strcpy(newNode->val,token_args);
                }
                // set next node as null in the list
                newNode->next = NULL;

                //if head is NULL, the list is empty
                if(*head == NULL) {
                        *head = newNode;
                }
                //else, find the last node and add newNode
                else {
                        //add the newNode at the end of the linked list
                        struct node *lastNode = *head;
			
                
                        //iterate lastNode->next until will be NULL.
                        while(lastNode->next != NULL) {
                                lastNode = lastNode->next;
                        }
                        //last node->next is now newNode, appending it to the list
                        lastNode->next = newNode;
                }
                (num_args)++;
                token_args = strtok(NULL, delimiter);
        }              
        return num_args;
}
// iterate over the linked list and store the node string values into the array
char** ll_to_arr(struct node* head, int num_args) {
        int arg_pos = 0;
        char** args = (char**) malloc(sizeof(char*) * (num_args+1));
        // set temporary node to iterate over linked list
        struct node* temp = head;
        while (temp != NULL) {
                args[arg_pos] = malloc(strlen(temp->val)+1);
                // copy string from node to array position
                strcpy(args[arg_pos],temp->val); 
                temp = temp->next;
                arg_pos++;
        }
        // set last argument of args as NULL for execvp
        args[arg_pos] = NULL;
        return args;
}
// function for redirecting stdout and stderr to file
void redirection(char* cmd, char* file_name) {
        int num_args;
        int redir_err = 0;
        struct node* head_arg = NULL;
        // find index of '>'
        int length_redir = file_name - cmd+1; 
        char redir_cmd[length_redir];
        // copy substring before ">" to redir_cmd
        memcpy(redir_cmd, cmd, length_redir); 
        redir_cmd[length_redir-1] = '\0'; //end string with null character
        // check if we must also redirect stderr, trimming whitespace as we do so
        if ((file_name + 1)[0] == '&') {
                redir_err = 1;
                file_name = trimwhitespace(file_name+2);
        }
        else {
                file_name = trimwhitespace(file_name+1);
        }
        // error manage for a filename
	if (strlen(file_name) == 0) {
		fprintf(stderr, "Error: no output file\n");
		return;
	}
        // create or truncate a the file, and store the file descriptor
        int fd;
        fd = open(file_name, O_CREAT | O_RDWR | O_TRUNC, 0644);
        if (fd < 0) {
		fprintf(stderr, "Error: cannot open output file\n");
		return;
	}
        // create a linked list of arguments before the redirection character
        num_args = linked_list(redir_cmd, &head_arg, " ");
        // error manage for a missing command
        if (num_args == 0) {
		fprintf(stderr, "Error: missing command\n");
	}
	else {
                // store linked list to array
        	char** args = ll_to_arr(head_arg,num_args);
            
        	int status;
                // fork after checking to redirect stderr to file or not
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
// redirection helper function, only called if '>' is present in the command
int find_redirection(char* cmd) {
        char *file_name;
        file_name = strchr(cmd, '>');
        if (file_name != NULL)  {
                redirection(cmd,file_name);
                return 1;
        }
        return 0;
}
// pipeline helper function to search for combined redirection 
void pipeline_helper(struct node** head_pipe, int** err_fd, int length) {
        // create array of which pipes must have stderr piped as well
        *err_fd = malloc(length * sizeof(int));
        int count = 1;
        while (*head_pipe != NULL) {
                // parse each command in the pipe linked list for '&'
                // set array position to 1/true if found
                if (((*head_pipe)->val)[0] == '&') {
                        (*head_pipe)->val = trimwhitespace((*head_pipe)->val + 1);
                        (*err_fd)[count-1] = 1;
                }
                else {
                        // set array position to 0/false if not found
                        (*err_fd)[count-1] = 0;
                }
                head_pipe = &((*head_pipe)->next);
                count++;
        }
        // set last command to not found
        //we don't have to redirect stderr for the last command
	(*err_fd)[count-1] = 0;

}
// function to determine if we need to pipe commands
int pipeline_general(char* cmd) {
        // intial check to see if piping character is present
	if (strchr(cmd,'|') == NULL) {
		return 0;
	}
        // else, create a linked list of piped commands
        struct node* head_pipe = NULL;
        int num_commands = linked_list(cmd,&head_pipe,"|"); // find number of commands
        // check if stderr has to be redirected for each command
        int* err_fd_arr;
        pipeline_helper(&(head_pipe->next),&err_fd_arr,num_commands);
        int i;
        int fd[2];
        int input_fd = 0; 
        // for first child process, stdin is the default
        // we are not reading from other pipes
        int num_args;
        // initialize array of statuses for each command
        int status_arr[num_commands];
        // default output redirection for the last command 
	int file_fd = 1;
        for (i = 0; i < num_commands; i++) {
                int std_err_fd = 2; // default error fd
                struct node* head_arg = NULL;
                // for each command, we pipe
                pipe(fd);
		char** args;
                if (head_pipe != NULL) {
                        // check for redirection in each command
			char* file_name;
                        file_name = strchr(head_pipe->val, '>');
			if (file_name != NULL) {
                                // only redirect if it is the last command
				if (i == num_commands - 1) {
					redirection(head_pipe->val, file_name);	
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
                // check if for the current command, we are redirected output
		if (err_fd_arr[i] == 1) {
			std_err_fd = fd[1];
		}
                // if this is the last command, redirect the output to file
                if (i == num_commands - 1) {
                        status_arr[i] = forking(args, input_fd,file_fd,std_err_fd);
                }
                else {
                        status_arr[i] = forking(args, input_fd, fd[1],std_err_fd);
                }
                // close write access
                close(fd[1]);
                // update input file descriptor 
                input_fd = fd[0];
                // free linked list of arguments
                freeList(head_arg);
                // free argument array
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
        // malloc space for environment variables
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
                        // check number of arguments
			if (num_args > 16) {
				fprintf(stderr, "Error: too many process arguments\n");
			}
			else if (num_args > 0) {
                                // check/perform inbuilt commands
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
                                // perform regular commands via fork at regular standard streams
                                else {
                                        int status = forking(args,0,1,2);
                                        fprintf(stderr, "+ completed '%s' [%d]\n",
                                                cmd, WEXITSTATUS(status));
                                }
                                // free argument array
                                for (int i = 0; i < num_args; i++) {
                                        free(args[i]);
                                }
                                free(args);
                        }
                        // free linked list of arguments
                        freeList(head_arg);
                }
        }
        return EXIT_SUCCESS;
}

