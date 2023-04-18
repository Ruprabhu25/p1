#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <ctype.h>

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
        //printf("node insertion: %s\n", newNode->val);

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
        printf("\n");
}
char* trimwhitespace(char *str)
{
  char *end;

  // Trim leading space
  while(isspace((unsigned char)*str)) str++;

  if(*str == 0)  // All spaces?
    return str;

  // Trim trailing space
  end = str + strlen(str) - 1;
  while(end > str && isspace((unsigned char)*end)) {
        end--;
  } 

  // Write new null terminator character
  end[1] = '\0';

  return str;
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
void forking(char* cmd, char* args[], int fd) {
        //printf(" made it to fork\n");
        int status;
        pid_t pid;

        if (!(pid = fork())) {
                //printf("child process\n");
                //printf("arg0: %s\n", args[0]);
                if (fd != -1) {
                        dup2(fd, STDOUT_FILENO);
                }
                execvp(args[0], args);
                //perror("execvp");
                fprintf(stderr, "Error: command not found\n");
                exit(1);
        } else {
                //printf("parent process\n");
                waitpid(pid, &status, 0);
                fprintf(stdout, "+ completed '%s' [%d]\n",
                        cmd, WEXITSTATUS(status));
        }       
}
int linked_list(char* cmd, struct node** head, char* delimiter) {
        //printf("called linked list\n");
        int num_args = 0;
        char cmd_copy[CMDLINE_MAX];
        strcpy(cmd_copy,cmd);
        //printf("total before: %s\n", cmd_copy);
        char* token_args = strtok(cmd_copy, delimiter);
        // loop through the string to extract all other tokens
        while( token_args != NULL ) {
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
                token_args = strtok(NULL, delimiter); 
        }              
        //printf("list:\n");
        //printList(*head);
        return num_args;
}
char** ll_to_arr(struct node* head, int num_args) {
        int arg_pos = 0;
        char** args = (char**) malloc(sizeof(char*) * (num_args+1));
        struct node* temp = head;
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
        //print_arr(args, arg_pos);
        return args;
}
void redirection(char* cmd, char* file_name, int* num_args, struct node* head_arg) {
        // means we found a ">" character
        //printf("found a '>' at %s\n", file_name);
        int length_redir = file_name - cmd; // find index of ">"
        char redir_cmd[length_redir];
        memcpy(redir_cmd, cmd, length_redir); // copy substring before ">" to redir_cmd
        redir_cmd[length_redir] = '\0'; //end string with null character
        file_name = trimwhitespace(file_name+1);
        //printf("redir_cmd: %s, file_name: %s\n", redir_cmd, file_name);
        int fd;
        fd = open(file_name, O_CREAT | O_RDWR | O_TRUNC, 0644);
        //printf("create fd\n");
        //dup2(fd, STDOUT_FILENO);
        //printf("dup2\n");
        *num_args = linked_list(redir_cmd, &head_arg, " ");
        //printf("%s\n", head_arg->val);
        //printf("\n");
        char** args = ll_to_arr(head_arg,*num_args);
        //printf("redir cmd: %s\n",redir_cmd);
        //print_arr(args, num_args);
        //exit(0);
        //printf("forking now:\n");
        forking(redir_cmd, args, fd); 
        close(fd);
        //dup2(STDERR_FILENO,STDOUT_FILENO);
}
void pipeline(char *command1[], char *command2[], char* cmd) {
        pid_t p1, p2;
        int fd[2];
        int status1, status2;
        pipe(fd);

        if (!(p1 = fork())) { /* Child #1 */
                close(fd[0]); /* No need for read access */
                dup2(fd[1], STDOUT_FILENO); /* Replace stdout with pipe */
                close(fd[1]); /* Close now unused FD */
                execvp(command1[0], command1); /* Child #1 becomes command1 */
        }
        if (!(p2 = fork())) { /* Child #2 */
                close(fd[1]); /* No need for write access */
                dup2(fd[0], STDIN_FILENO); /* Replace stdin with pipe */
                close(fd[0]); /* Close now unused FD */
                execvp(command2[0], command2); /* Child #2 becomes command2 */
        }

        close(fd[0]); /* Pipe no longer needed in parent */
        close(fd[1]);
        waitpid(p1, &status1, 0); /* Parent waits for two children */
        waitpid(p2, &status2, 0);
        fprintf(stdout, "+ completed '%s' [%d][%d]\n",
                cmd, WEXITSTATUS(status1), WEXITSTATUS(status2));
}

void double_pipeline(char* cmd1[], char* cmd2[], char* cmd3[], char* cmd) {
        pid_t p1, p2, p3;
        int status1, status2, status3;
        int fd1[2];
        int fd2[2];
        pipe(fd1);
        pipe(fd2);
        //printf("%s \n", cmd1[0]);
        //printf("%s \n", cmd2[0]);
        //printf("%s \n", cmd3[0]);
        if (!(p1 = fork())) { /* Child #1 */
                printf("child1\n");
                close(fd1[0]); /* No need for read access for pipe 1*/
                dup2(fd1[1], STDOUT_FILENO); /* Replace stdout with pipe */
                close(fd1[1]); /* Close now unused FD */
                execvp(cmd1[0], cmd1); /* Child #1 becomes command1 */
                perror("execvp");
                exit(1);
        }
        if (!(p2 = fork())) { /* Child #2 */
        printf("child2\n");
                close(fd1[1]); /* No need for write access for pipe 1*/
                close(fd2[0]); /* No need for read access for pipe 2*/
                dup2(fd1[0], STDIN_FILENO); /* Replace stdin with pipe 1*/
                dup2(fd2[1], STDOUT_FILENO); /* Replace stdout with pipe 2*/
                close(fd1[0]); /* Close now unused FD */
                close(fd2[1]);
                execvp(cmd2[0], cmd2); /* Child #2 becomes command2 */
                perror("execvp");
                exit(1);
        }
        if (!(p3 = fork())) {
                printf("child3\n");
                close(fd2[1]);
                dup2(fd2[0], STDIN_FILENO);
                close(fd2[0]);
                execvp(cmd3[0], cmd3);
                perror("execvp");
                exit(1);
        }

        close(fd1[0]); /* Pipe no longer needed in parent */
        close(fd1[1]);
        close(fd2[0]);
        close(fd2[1]);
        waitpid(p1, &status1, 0); /* Parent waits for three children */
        //printf("child 1 done");
        waitpid(p2, &status2, 0);
        //printf("child 2 done");
        waitpid(p3, &status3, 0);
        //printf("child 3 done");
        fprintf(stdout, "+ completed '%s' [%d][%d][%d]\n",
        cmd, WEXITSTATUS(status1), WEXITSTATUS(status2), WEXITSTATUS(status3));
}
int pipeline_search(char* cmd, struct node* head_pipe) { // try to make each command in pipeline a linked list of args?
        int num_pipes = linked_list(cmd,&head_pipe,"|") - 1;
        if (!num_pipes) {
                return 0;
        }
        else { // find way to create linked list of args per node of linked list of pipeline
                if (num_pipes == 1) {
                        struct node* head_args1 = NULL;
                        struct node* head_args2 = NULL;
                        int num_args1 = linked_list(head_pipe->val, &head_args1, " ");
                        int num_args2 = linked_list(head_pipe->next->val, &head_args2, " ");
                        char** args1 = ll_to_arr(head_args1, num_args1);
                        char** args2 = ll_to_arr(head_args2, num_args2);
                        //print_arr();
                        //printf("single piping");
                        pipeline(args1, args2, cmd);
                }
                else if (num_pipes == 2) {
                        struct node* head_args1 = NULL;
                        struct node* head_args2 = NULL;
                        struct node* head_args3 = NULL;
                        /*printf("1). %s\n", head_pipe->val);
                        printf("2). %s\n", head_pipe->next->val);
                        printf("3). %s\n", head_pipe->next->next->val);*/
                        int num_args1 = linked_list(head_pipe->val, &head_args1, " ");
                        int num_args2 = linked_list(head_pipe->next->val, &head_args2, " ");
                        int num_args3 = linked_list(head_pipe->next->next->val, &head_args3, " ");
                        char** args1 = ll_to_arr(head_args1, num_args1);
                        char** args2 = ll_to_arr(head_args2, num_args2);
                        char** args3 = ll_to_arr(head_args3, num_args3);
                        /*printf("1. ");
                        print_arr(args1, num_args1);
                        printf("2. ");
                        print_arr(args2, num_args2);
                        printf("3. ");
                        print_arr(args3, num_args3);*/
                        printf("double piping\n");
                        double_pipeline(args1, args2, args3, cmd);
                }
                else if (num_pipes == 3) {
                        
                }
                else {
                        fprintf(stderr, "too many pipes");
                        exit(0);
                }
        }
        return 1;
}
int main(void) {
        char cmd[CMDLINE_MAX];
        struct node *head_arg = NULL;
        struct node *head_pipe = NULL;

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
                /* Search for piping first */
                if (!pipeline_search(cmd, head_pipe)) {
                        //exit(0);

                        // output redirection
                        char *file_name;
                        file_name = strchr(cmd, '>');
                        //printf("test0\n");
                        if (file_name != NULL)  {
                                //printf("redirected\n");
                                redirection(cmd,file_name,&num_args,head_arg);
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
                                num_args = linked_list(cmd, &head_arg, " ");
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
                                        forking(cmd,args,-1);
                                }
                        }
                        head_arg = NULL;
                }
        }
        return EXIT_SUCCESS;
}

