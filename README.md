# swish_shell

this program involves significant use of system calls, specifically system calls for process creation, process management, I/O, and signal handling. While a single system call by itself is not always interesting, the real challenge and excitement of systems programming is in combining system calls to build useful and powerful tools. This program demonstrates a simple shell.

Whenever you are using a terminal program, you are really interacting with a shell process. Command-line shells allow one access to the capabilities of a computer using simple, interactive means. Type the name of a program and the shell will bring it to life as a new process, run it, and show output. 


The goal of this program is to write a simplified command-line shell. This shell will be less functional in many ways than standard shells like bash (the default on most Linux machines), but it will still have some useful features.

This program will cover a number of important systems programming topics:

    String tokenization using strtok()
    Getting and setting the current working directory with getcwd() and chdir()
    Program execution using fork() and exec()
    Child process management using wait() and waitpid()
    Input and output redirection with open() and dup2()
    Signal handling using setpgid(), tcsetpgrp(), and sigaction()
    Managing foreground and background job execution using signals and kill()
