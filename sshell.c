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
void printList(struct node *head)
{
    struct node *temp = head;

    //iterate the entire linked list and print the data
    while(temp != NULL)
    {
         printf("%s->", temp->val);
         temp = temp->next;
    }
    //printf("NULL\n");
}
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
        //printf("array of args: ");
        for (int i = 0; i<size; i++) {
                printf("%s ", args[i]);
        }
        //printf("\n");
}
void cmd_cd(char* cmd, int num_args, char* args[]) {
        int cd_status = 0;
        int status;
        if (num_args == 1) { // command is "cd"
                //printf("home directory\n");
                cd_status = chdir("/");
        } else { // num_args  > 1
                //printf("specific directory to %s\n", args[1]);
                cd_status = chdir(args[1]);
        }
        //printf("cd status: %d\n", cd_status);
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
        //printf(" made it to fork\n");
        int status;
        pid_t pid;
        pid = fork();
        if (pid == 0) {
                //printf("child process\n");
                //printf("arg0: %s\n", args[0]);
                execvp(args[0], args);
                fprintf(stderr, "Error: command not found\n");
                exit(1);
        } else if (pid > 0) {
                //printf("parent process\n");
                waitpid(pid, &status, 0);
                fprintf(stdout, "+ completed '%s' [%d]\n",
                        cmd, WEXITSTATUS(status));
        }       
}
int linked_list(char* cmd, struct node** head) {
        //printf("called linked list\n");
        int num_args = 0;
        char cmd_copy[CMDLINE_MAX];
        strcpy(cmd_copy,cmd);
        //printf("total before: %s\n", cmd_copy);
        char* token_args = strtok(cmd_copy, " ");
        // loop through the string to extract all other tokens
        while( token_args != NULL ) {
                //char token[strlen(token_args)+1];
                //strcpy(token,token_args);
                //token[strlen(token_args)] = '\0';
                //printf("token: %s\n", token_args);
                
                // create new node with value of pattern
                struct node *newNode = (struct node*) malloc(sizeof(struct node));
                newNode->val = malloc(strlen(token_args) + 1);
                strcpy(newNode->val,token_args);
                newNode->next = NULL;
                //printf("node insertion: %s\n", newNode->val);

                //if head is NULL, the list is empty
                if(*head == NULL) {
                        *head = newNode;
                        //printf("head called\n");
                }
                //else, find the last node and add newNode
                else {
                        //add the newNode at the end of the linked list
                        //printf("add to end\n");
                        struct node *lastNode = *head;
                
                        //last node's next address will be NULL.
                        while(lastNode->next != NULL) {
                                //printf("%s ", lastNode->val);
                                lastNode = lastNode->next;
                        }
                        //printf("\n");

                        //add the newNode at the end of the linked list
                        lastNode->next = newNode;
                }
                (num_args)++;
                token_args = strtok(NULL, " "); 
        }              
        //printf("list:\n");
        //printList(*head);
        return num_args;
}
void pipeline(char* cmd1, char* cmd2, char* cmd3) {
        pid_t p1, p2, p3;
        int fd1[2];
        int fd2[2];
        pipe(fd1);
        if (cmd3 != NULL) {
                pipe(fd2);
        }

        if (!(p1 = fork())) { /* Child #1 */
                close(fd1[0]); /* No need for read access for pipe 1*/
                dup2(fd1[1], STDOUT_FILENO); /* Replace stdout with pipe */
                close(fd1[1]); /* Close now unused FD */
                exec(cmd1); /* Child #1 becomes command1 */
        }
        if (!(p2 = fork())) { /* Child #2 */
                close(fd1[1]); /* No need for write access for pipe 1*/
                close(fd2[0]); /* No need for read access for pipe 2*/
                dup2(fd1[0], STDIN_FILENO); /* Replace stdin with pipe 1*/
                dup2(fd2[1], STDOUT_FILENO); /* Replace stdout with pipe 2*/
                close(fd1[0]); /* Close now unused FD */
                exec(cmd2); /* Child #2 becomes command2 */
        }

        close(fd[0]); /* Pipe no longer needed in parent */
        close(fd[1]);
        waitpid(p1, NULL, 0); /* Parent waits for two children */
        waitpid(p2, NULL, 0);
}
int main(void) {
        char cmd[CMDLINE_MAX];
        struct node *head_arg = NULL;

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
                //printf("test0\n");
                if (file_name != NULL)  {
                        // means we found a ">" character
                        //printf("found a '>' at %s\n", file_name);
                        int length_redir = file_name - cmd; // find index of ">"
                        char redir_cmd[length_redir];
                        memcpy(redir_cmd, cmd, length_redir); // copy substring before ">" to redir_cmd
                        redir_cmd[length_redir] = '\0'; //end string with null character
                        file_name = file_name + 1;
                        //printf("redir_cmd: %s, file_name: %s\n", redir_cmd, file_name);
                        int fd;
                        fd = open(file_name, O_CREAT | O_WRONLY | O_TRUNC, 0644);
                        //printf("create fd\n");
                        dup2(fd, STDOUT_FILENO);
                        //printf("dup2\n");
                        num_args = linked_list(redir_cmd, &head_arg);
                        char* args[(num_args)+1];
                        int arg_pos = 0;
                        //exit(0);
                        //printf("num args: %d\n", num_args);
                        //printf("current2: %s\n", current_arg->val);
                        //printf("insert into args:\n");
                        struct node* temp = head_arg;
                        //printList(temp);
                        while (temp != NULL) {
                                //printf("%d %s\n", arg_pos, head_arg->val);
                                //printf("temp: %s args: %s\n", temp->val, args[arg_pos]);
                                args[arg_pos] = malloc(strlen(temp->val)+1);
                                strcpy(args[arg_pos],temp->val); //CHECK OUT
                                //printf("temp: %s\n", temp->val);
                                //memcpy(args[arg_pos], temp->val, strlen(temp->val));
                                //printf("temp: %s args:\n", temp->val);
                                temp = temp->next;
                                arg_pos++;
                                //printf("end of loop\n");
                        }
                        args[arg_pos] = NULL;
                        //printf("redir cmd: %s\n",redir_cmd);
                        //print_arr(args, num_args);
                        //exit(0);
                        //printf("forking now:\n");
                        forking(redir_cmd, args); 
                        close(fd);
                        dup2(STDERR_FILENO,STDOUT_FILENO);
                }
                else {
                        //printf("test1\n");
                        
                        //printf("%d\n", num_args);
                        //print_arr(args, num_args);
                        //printf("%d\n", arg_pos);
                        //printf("command: %s\n", cmd);
                        //char* path = getenv("PATH");
                        //char* token_path = strtok(path, ":");
                        // loop through the string to extract all other tokens
                        //check if it is a built in command, then we don't have to fork
                        num_args = linked_list(cmd, &head_arg);
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
                        //printf("test2\n");
                        if (strcmp(args[0], "cd") == 0) {
                                cmd_cd(cmd,num_args,args);
                        } else if (strcmp(args[0], "pwd") == 0) {
                                char pwd_name[1024];
                                getcwd(pwd_name, sizeof(pwd_name));
                                printf("%s\n",pwd_name);
                        }
                        else {
                                //printf("args[0]: %s", args[0]);
                                forking(cmd,args);
                        }
                }
                head_arg = NULL;
        }
        return EXIT_SUCCESS;
}

