#include <iostream>
#include <string>
#include <vector>
#include <sstream>
#include <unistd.h>
#include <sys/wait.h>

int main(){
    std::string input; // to store command line

    while(true){
        // getting command line
        std::cout<<"Mini shell> "<<std::flush;
        if(!std::getline(std::cin, input)) break;
        if(input.empty()) continue;

        // to tokenize the shell
        std::istringstream iss(input);
        std::vector<char*> args;
        std::string token;

        while( iss >> token ){
            // C-style string for execvp
            char* temp = new char[token.size()+1];
            std::strcpy(temp, token.c_str()); 
            args.push_back(temp);
        }
        args.push_back(nullptr);

        // child process for execute
        pid_t p_id = fork();

        if(!p_id){
            execvp(args[0], args.data());
            perror("Failed");
            exit(1);
        }
        else if(p_id > 0){
            wait(nullptr);
        }
        else {
            perror("Faled");
        }
        // free memory
        for(size_t i = 0; i < args.size() - 1; ++i) {
            delete [] args[i];
        }
        args.clear();

    }
    return 0;
}