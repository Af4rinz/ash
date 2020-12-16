# ash Shell 
This shell was written for Fall 2020 - Operating Systems course in C++.


## Build Instructions
1. navigate to project folder `./ash`
2. run `make -f makefile` or simply `make` to compile the code.
3. run `./ash` 
* *note that the code utilises the GNU readline library for history and reading command input.*

## Features

* prompt message with current working directory
* pipelining for two commands
* support for various built-in commands (see *Built-in Commands*)
* common aliases such as `ls --color=auto` and `df -h`
* a history of recent commands
* conversion of `~/` to `$HOME/`
* parameter passing
* inter-process communication using shared memory between shells
* batch mode and interactive mode
* `ctrl+D` termination

## Built-in Commands
* `help` displays a help screen with a list of available commands
* `cd [path]` changes directory, supports `~/`
* `pwd` prints workings directory in colour
* `shid` prints the shell id assigned by user
* `send [shid] [message]` sends a message to the other terminal
* `receive` reads all unread messages from the other terminal
* `clear` clears terminal output (using escape keys)
* `history` prints a history of recent commands
* `exit` leaves the terminal (with a message)

## Under the Hood
Some basic functions implementation & usage:
* send/receive and shared memory
	>the shell begins with a prompt to enter the 'id' of the shell (or shid). This assigns a read path to the terminal (e.g. for id 4402, it reads from `/tmp/ash-inbox-4402`). The initialization uses mkfifo() with the addresses for inbox read/write access. Then the send command simply writes to the inbox of another shell. Then the other shell may use read command to see all unread messages.
* history
	> the history function utilizes readline/history library and is initiated as soon as the shell starts. Then, all non-empty lines are added to history and can be read from its stack by running `history` command.
* built-in commands
	> built-in commands are implemented by mapping each command name to its relevant function in the code. During execution of commands, first the input is checked against this list of built-in commands and if one is found, it will be executed, else it will advance to the `execute()` function and taken care of by the system.
* aliases
	>similar to built-in commands, a map of common aliases exists in the code, and while parsing, if the command matches an alias, the necessary arguments will be replaced and then the command is processed/run.
* pipelining
	> pipes are first taken apart by `|` symbol, then during execution, it is checked if the input includes pipes. If so, the `execute()` function will create a pipe and write to it the results of first command, then hand out the read end to the next command in the pipeline.

