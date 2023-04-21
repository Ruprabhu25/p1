### Piping
For piping, there are 2 main functions `pipeline_helper()` and `pipeline_general()`. 

`pipeline_helper()` determines if we have to redirect std_err in the pipeline as well. This is done by parsing each command in the pipeline to see if it contains the `&` character after the `|` character.(This is done before trimming)

`pipeline_general()` finds the number of commands by counting the `'|'` in a command line. When `num_commands = 1`, it will be treated as a regular command and no piping is required. Otherwise, each command between pipes will be treated as a regular function after redirecting its input/output using the `pipe()` function.


### Extra feature
1. The `set` command is a feature that sets the value of an environment variable to a string. It can only be done if the number of arguments is greater than 2 and the second argument is 1 character long. We use ASCII to check if the second aargument is a letter `a-z`, and subtract the integer value 97 to index from `0-25`. When all conditions are met, `set` will assign the third argument to `env_vars[index]` using `strcpy()`. It can also *unset* a variable back to an empty string when no third argument is provided.

2. For combined redirection, small tweaks have been made in the `redirection()` and `pipeline_helper()` function to account for the errors issued by commands that may be printed onto the terminal instead of being redirected or piped to a specified file or command followed by a pipe. We did so by parsing for the character `&` after the occurence of a `>` or `|`. If so, then we would set a variable std_err_fd to the updated std_out file descriptor. When the `forking()` function is called, each read, write, error file descriptor is checked to see if it is anything different than the default. If so then we use `dup2()` to redirect that output to that file descriptor. 