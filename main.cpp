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
        std::cout<<"\033[36mMini shell> \033[0m"<<std::flush;
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
        if(tokenslist[0] == "exit") break;
        else if(tokenslist[0] == "cd"){
            if(tokenslist.size()<2){
                std::cerr<<"\33[31mcd: missing argument\033[0m"<<std::endl;
            }
            else {if(chdir(tokenslist[1].c_str()) != 0){
               perror("\033[31mcd failed\033[0m");
            }
        }
        continue;
    }
    else if(tokenslist[0] == "mkdir"){
        if(tokenslist.size()<2){
            std::cerr<<"\033[31mmkdir: missing directory name\033[0m "<<std::endl;
        }
        else {
            if(mkdir(tokenslist[1].c_str(), 0755) != 0){
                perror("\033[31mmkdir failed\033[0m");

            }
        }
    }
    else if(tokenslist[0] == "clear"){
        //std::cout<<"\033[2J\033[1;1H"; // ansi escape code to clear terminal
        pid_t p = fork();
        if(p == 0){
            execlp("clear", "clear", nullptr);
            perror("\033[31mclear failed\033[0m");
            exit(1);
        }
        else if(p > 0){
            wait(nullptr);
        }
        else {
            perror("\033[31mclear failed\033[0m");
        }
        continue;
    }
    // else if (tokenslist[0] == "help") {
    //     std::cout << "Built-in commands:\n"
    //               << "  cd <dir>   - change directory\n"
    //               << "  exit       - exit shell\n"
    //               << "  help       - show this message\n"
    //               << "Other commands are run via execvp.\n";
    //     continue;   
    // }
    else if ( tokenslist[0] == "echo"){
        for(auto &t : tokenslist){
            std::cout<<t;
        }
    }
    else if(tokenslist[0] == "whoami"){
        char* username = getenv("USER");
        if(username){
            std::cout<<username<<std::endl;
        }
        else {
            std::cerr<<"\033[31mwhoami: could not get username\033[0m"<<std::endl;
        }
        continue;
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
            perror("\033[31mexecvp failed\033[0m");
            exit(1);
        }
        else if(p_id > 0){
            wait(nullptr);
        }
        else {
           perror("\033[31mexecvp failed\033[0m");
        }
        // free memory
        for(size_t i = 0; i < args.size() - 1; ++i) {
            delete [] args[i];
        }
        args.clear();

    }
    return 0;
}