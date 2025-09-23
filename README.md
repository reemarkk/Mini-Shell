## Description

This is a simple command-line shell implemented in C++. It supports basic execution of system commands using fork() and execvp(). The shell reads user input, parses it into arguments, and runs the specified command in a child process.
- Note: This is a minimal implementation. The shell currently supports only basic single commands and lacks features like command piping, redirection, and background execution. It is intended as a foundation for further improvements and learning. This shell is designed to work on **Unix-like** environments (Linux, macOS) and may not function correctly on **Windows without modification.**
