# ash Shell 
This shell was written for Fall 2020 - Operating Systems course in C++.


## Build Instructions
1. navigate to project folder `./ash`
2. run `make -f makefile` or simply `make` to compile the code.
3. run `./ash` 
* *note that the code utilises the GNU readline library for history and reading command input.*

## Features

* prompt message with current working directory
* pipelining multiple commands
* support for various built-in commands (see *Built-in Commands*)
* common aliases such as `ls --color=auto` and `df -h`
* a history of recent commands
* conversion of `~/` to `$HOME/`
* parameter passing
* inter-process communication using shared memory between shells
* batch mode and interactive mode
* `ctrl+D` termination and `ctrl+C` abortion

## Built-in Commands
* `help` displays a help screen with a list of available commands
* `cd [path]` changes directory, supports `~/`
* `pwd` prints workings directory in colour
* `shid` prints the shell id assigned by user
* `send [shid] [message]` sends a message to the other terminal
* `receive` reads all unread messages from the other terminal
* `clear` clears terminal output
* `history` prints a history of recent commands
* `quit` leaves the terminal (with a message)




