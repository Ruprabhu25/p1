# SSHELL: Simple Shell

## Summary
This program, `sshell.c`, is a simple shell that accepts inputs from user in 
the form of command lines and executes them.

## Implementation
The implementation of this program is followed by 7 distinct phases starting 
from phase 0.

### Phase 0
This phase is rather straight forward, as a skeleton shell is already given. 
However, the `system()` function that was used is not viable for creating a 
realistic shell since it does not support output redirection or piping which 
will be implemented later on.

A `Makefile` is written to generate an executable sshell from the sshell.c 
file, including the flags `-Wall -Wextra -Werror` and removing any generated 
files and puts the directory back to its original state. 

### Phase 1
In getting rid of the `system()` function, the program is modified so that it 
uses the *fork + exec + wait* method. A `forking()` function is then created to
incorporate this method. The function takes in 4 parameters `(char* args[], 
int read_fd, int write_fd, int err_fd)`. During the child process, for regular
commands, `read_fd`, `write_fd`, and `err_fd` are 0, 1, and 2 respectively 
because we are not redirecting any input/output to a file. `execvp()` is 
called to retrieve command from `$PATH` which is an environment variable. In 
the case that `execvp()` fails, an error message will be printed. During the
parent process, we forced the parent to wait until the child has return an 
exit status before processing.

To test this function, we simply used commands such as `echo` and `ls`.

### Phase 2
In order to parse the command lines and still maintain a proper data structure 
for a later phase, we decided to use `linked lists`. The pros of using this 
data structure include efficiency because command lines only needed to be 
parsed once that will later be useful in piping.

To implement this, a `linked_list()` function is created. The way it works is 
that it parses through a command line and look for commands and arguments 
separated by white spaces and store them in a node on the linked list 
respectively. This function returns the number of nodes in the linked list 
which is used later.

This function was easily tested by writing a random command and arguments onto
the command line and once it returns each of the tokens separated individually
then it has proven to parse successfully.

An addition to the `linked_list()` function, a `ll_to_arr()` function was also 
created to store the nodes of the linked list into an argument array.

### Phase 3
The `cd` command was implemented by using the `chdir()` function to change the 
directory on the terminal. The `cmd_cd()` function that was used to implement 
the `cd` command takes in 3 parameters `(char* cmd, int num_args, 
char* args[])`. When `num_args = 1`, meaning there is only 1 argument and the 
user is trying to go back to the home directory, `chdir("/")` will return a 
status number of 0 which means the process was successful. When `num_args > 1`,
the same process occur except `chdir()` will change the directory to `args[1]`
of the command line which is an argument followed by the `cd` command. A 
status of 0 will then return a message showing that the directory is changed 
successfully. However, a status other than 0 (i.e status = 1) will output an 
error message.

The `pwd` command was easily implemented with just the `getcwd()` function and
printing the result to the terminal together with the 
*Process Completion Status*.

### Phase 4
There are 2 functions created and implemented mainly for output redirection,
`redirection(char* cmd, char* file_name)` and `find_redirection(char* cmd)`.
The `redirection()` function was implemented so that it parses through a 
command line and look for the character `>`. Once it is found, the index of 
the `>` character will be recorded so that we can separate the substrings 
before and after `>`. A null character `'\0'` will also be included to end the
substring. 

After that, a file descriptor is created to open the file that was extracted 
from the command line. But before opening the file, we had to make sure that 
there are not any white spaces surrounding the file name. To solve that, the 
function `trimwhitespace()` was created to remove any leading and trailing 
white spaces. Once the file name is free of unnescessary white spaces, it can 
then be open with the function `open()` with the flags `O_CREAT`, `O_RDWR`, and
`O_TRUNC`. The first flag is used to create a file if it doesn't exist. The 
second flag is to give the ability to read and write to the file. The third 
flag is used to overwrite/truncate the file's contents if it exist already.

The `find_redirection()` function was created to look for a `>` character 
within the command line using `strchr()`. It should return a 0 if none was 
found, and call `redirection()` function if a `>` was found and return the 
value 1. This *helper* function was created so that we don't need to parse 
through every command line and rather just skip to the next step.

### Phase 5
For piping, there are 2 main functions `pipeline_helper()` and 
`pipeline_general()`. 

`pipeline_helper()` determines if we have to redirect std_err in the pipeline 
as well. This is done by parsing each command in the pipeline to see if it 
contains the `&` character after the `|` character.
(This is done before trimming)

`pipeline_general()` finds the number of commands by counting the `'|'` in a 
command line. When `num_commands = 1`, it will be treated as a regular command 
and no piping is required. Otherwise, each command between pipes will be 
treated as a regular function after redirecting its input/output using the 
`pipe()` function.


### Phase 6
1. The `set` command is a feature that sets the value of an environment 
variable to a string. It can only be done if the number of arguments is greater
than 2 and the second argument is 1 character long. We use ASCII to check if 
the second aargument is a letter `a-z`, and subtract the integer value 97 to 
index from `0-25`. When all conditions are met, `set` will assign the third 
argument to `env_vars[index]` using `strcpy()`. It can also *unset* a variable 
back to an empty string when no third argument is provided.

2. For combined redirection, small tweaks have been made in the `redirection()`
and `pipeline_helper()` function to account for the errors issued by commands 
that may be printed onto the terminal instead of being redirected or piped to a
specified file or command followed by a pipe. We did so by parsing for the 
character `&` after the occurrence of a `>` or `|`. If so, then we would set a 
variable std_err_fd to the updated std_out file descriptor. When the 
`forking()` function is called, each read, write, error file descriptor is 
checked to see if it is anything different than the default. If so then we use 
`dup2()` to redirect that output to that file descriptor.

## Sources:
https://stackoverflow.com/questions/122616/how-do-i-trim-leading-trailing-whitespace-in-a-standard-way

https://stackoverflow.com/questions/8082932/connecting-n-commands-with-pipes-in-a-shell

https://man7.org/linux/man-pages/man2/open.2.html

https://linux.die.net/man/3/execvp

https://man7.org/linux/man-pages/man2/chdir.2.html

https://www.tutorialspoint.com/c_standard_library/c_function_strtok.htm



