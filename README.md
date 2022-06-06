# Swish_shell

This program creates a command-line shell.

It involves significant use of system calls, specifically system calls for process creation, process management, I/O, and signal handling. While a single system call by itself is not always interesting, the real challenge and excitement of systems programming is in combining system calls to build useful and powerful tools.

Whenever you are using a terminal program, you are really interacting with a shell process. Command-line shells allow one access to the capabilities of a computer using simple, interactive means. Type the name of a program and the shell will bring it to life as a new process, run it, and show output. 


This program will cover a number of important systems programming topics:

    String tokenization using strtok()
    Getting and setting the current working directory with getcwd() and chdir()
    Program execution using fork() and exec()
    Child process management using wait() and waitpid()
    Input and output redirection with open() and dup2()
    Signal handling using setpgid(), tcsetpgrp(), and sigaction()
    Managing foreground and background job execution using signals and kill()
