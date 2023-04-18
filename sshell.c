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
struct proc {
        //char* cmd;
        char** args;
        pid_t pid;
        int retval;
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
int forking(char* args[], int read_fd, int write_fd) {
        //printf(" made it to fork\n");
        int status;
        pid_t pid;
        //printf("read %d, write %d\n", read_fd, write_fd);
        if (!(pid = fork())) {
                //printf("child process %d\n", getpid());

                if (read_fd != 0) {
                        //printf("closed read: %d\n", read_fd);
                        dup2(read_fd, STDIN_FILENO);
                        close(read_fd); // not needed anymore
                }
                if (write_fd != 1) {
                        //printf("closed write: %d\n", write_fd);
                        dup2(write_fd, STDOUT_FILENO);
                        close(write_fd); // not needed anymore
                }
                execvp(args[0], args);
                perror("execvp");
                fprintf(stderr, "Error: command not found\n");
                exit(1);
        } else {
                //close(read_fd);
                //close(write_fd);
                waitpid(pid, &status, 0);
                //printf("parent process\n");
                return status;
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
void redirection(char* cmd, char* file_name) {
        int num_args;
        struct node* head_arg;
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
        num_args = linked_list(redir_cmd, &head_arg, " ");
        //printf("%s\n", head_arg->val);
        //printf("\n");
        char** args = ll_to_arr(head_arg,num_args);
        //printf("redir cmd: %s\n",redir_cmd);
        //print_arr(args, num_args);
        //exit(0);
        //printf("forking now:\n");
        forking(args, 0,fd); 
        close(fd);
        //dup2(STDERR_FILENO,STDOUT_FILENO);
}
int find_redirection(char* cmd) {
        char *file_name;
        file_name = strchr(cmd, '>');
        //printf("test0\n");
        if (file_name != NULL)  {
        //printf("redirected\n");
                redirection(cmd,file_name);
                return 1;
        }
        return 0;
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
int pipeline_general(char* cmd) {
        struct node* head_pipe;
        int num_commands = linked_list(cmd,&head_pipe,"|"); // find number of commands
        int i;
        int fd[2];
        int input_fd = 0; // for first child process, stdin is the default, we are not reading from other pipes
        int num_args;
        int status;
        if (num_commands == 1) { // treat as regular command, can exit
                return 1;
        }
        for (i = 0; i < num_commands; i++) {
                struct node* head_arg = NULL;
                char** args;
                pipe(fd);
                if (head_pipe != NULL) {
                        num_args = linked_list(head_pipe->val, &head_arg, " ");
                        args = ll_to_arr(head_arg,num_args);
                        head_pipe = head_pipe->next;
                }
                if (i == num_commands - 1) {
                        status = forking(args, input_fd, 1);
                }
                else {
                        status = forking(args, input_fd, fd[1]);
                }
                printf("status: %d\n", status);
                close(fd[1]);
                input_fd = fd[0];
                freeList(head_arg);
                for (int i = 0; i < num_args; i++) {
                        free(args[i]);
                }
                free(args);
        }
        return 0;
}
int main(void) {
        char cmd[CMDLINE_MAX];

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

                /* Piped commands */
                if (pipeline_general(cmd)) {

                }

                /* Redirected commands*/
                else if (find_redirection(cmd)) {

                }
                else {/* Regular, single command */
                        int num_args;
                        struct node* head_arg = NULL;
                        num_args = linked_list(cmd, &head_arg, " ");
                        char** args = ll_to_arr(head_arg, num_args);
                        if (strcmp(args[0], "cd") == 0) {
                                cmd_cd(cmd,num_args,args);
                        } 
                        else if (strcmp(args[0], "pwd") == 0) {
                                char pwd_name[1024];
                                getcwd(pwd_name, sizeof(pwd_name));
                                printf("%s\n",pwd_name);
                        }
                        else {
                                //printf("args[0]: %s", args[0]);
                                forking(args,0,1);
                        }
                        freeList(head_arg);
                        for (int i = 0; i < num_args; i++) {
                                free(args[i]);
                        }
                        free(args);
                }
        }
        return EXIT_SUCCESS;
}

