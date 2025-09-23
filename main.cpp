#include <iostream>
#include <string>
#include <vector>
#include <sstream>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <sys/types.h>

int main(){
    std::string input; // to store command line
    std::vector<char*> history;

    while(true){
        // getting command line
        std::cout<<"Mini shell> "<<std::flush;
        if(!std::getline(std::cin, input)) break;

        
        if(!input.empty()){
            history.push_back(strdup(input.c_str()));
        }
        else continue;

        if(input == "history"){
            for(size_t i = 0; i < history.size(); ++i){
                std::cout<<i+1<<" "<<history[i]<<std::endl;
            }
            continue;
        }
        

        // ------ Built-in commands ------
        // to tokenize the shell
        std::istringstream iss(input);
        std::vector<char*> args;
        std::string token;
        std::vector<std::string> tokenslist; 

        while(iss>>token) tokenslist.push_back(token);

        if(tokenslist.empty()) continue;
        if(tokenslist[0]=="exit") break;
        else if(tokenslist[0]=="cd"){
            if(tokenslist.size()<2){
                std::cerr<<"cd: missing argument"<<std::endl;
            }
            else {if(chdir(tokenslist[1].c_str()) != 0){
                perror("cd failed");
            }
        }
        continue;
    }
    else if(tokenslist[0] == "mkdir"){
        if(tokenslist.size()<2){
            std::cerr<<"mkdir: missing directory name "<<std::endl;
        }
        else {
            if(mkdir(tokenslist[1].c_str(), 0755) != 0){
                perror("mkdir failed");
            }
        }
    }
    else if (tokenslist[0] == "help") {
        std::cout << "Built-in commands:\n"
                  << "  cd <dir>   - change directory\n"
                  << "  exit       - exit shell\n"
                  << "  help       - show this message\n"
                  << "Other commands are run via execvp.\n";
        continue;   // don’t fork
    }

    
    //---- External commands ----
    for(auto & t : tokenslist){
        args.push_back(strdup(t.c_str()));
    }
        args.push_back(nullptr); // execvp needs null-terminated array
 
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