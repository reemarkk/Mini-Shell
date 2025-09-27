## Mini Shell
Mini Shell is a command-line interpreter written in C++ that provides basic shell functionality. It supports both built-in commands and the execution of external programs using fork and execvp, making it behave similarly to a traditional Unix shell.
### Features
**Built-in Commands** 

cd <dir> – change directory
mkdir <dir> – create a directory
clear – clear the terminal
echo [args] – print text or environment variables
whoami – show the current username
set VAR=VALUE – set environment variables
unset VAR – unset environment variables
history – show command history
search – search command history by number
exit – quit the shell

**Logical Operators**

cmd1 && cmd2 – run the second command only if the first succeeds
cmd1 || cmd2 – run the second command only if the first fails

**External Commands**

Any executable in the system PATH can be run (e.g., ls, cat, g++ main.cpp).

**History Persistence**

Saves history in historyFile.txt and loads it on startup.

**Technical Details**

The shell uses system calls like fork, execvp, wait, chdir, getcwd, setenv, unsetenv, and freopen to manage processes, directories, environment variables. It is designed as a learning project to demonstrate core concepts of how a Unix shell works.

Note: This is a minimal implementation. The shell currently supports only basic single commands and lacks features like command piping, redirection, and background execution. It is intended as a foundation for further improvements and learning. This shell is designed to work on **Unix-like** environments (Linux, macOS) and may not function correctly on **Windows without modification.**

