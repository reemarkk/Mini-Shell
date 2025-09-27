#include <iostream>
#include <string>
#include <vector>
#include <sstream>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <fstream>

#define MAX_HISTORY 100

struct Command {
    std::vector<std::string> args;
    std::string separator; // "&&", "||""

    Command(const std::vector<std::string>& a, const std::string& s) : args(a), separator(s) {}
};

// todo: handle redirections
void handleRedirections(std::vector<std::string> &tokenslist){
    // This function will handle input/output redirections
    for(size_t i = 0; i<tokenslist.size();i++){
        if(tokenslist[i] == "<"){
            if( i+1 <tokenslist.size()){
                freopen(tokenslist[i+1].c_str(), "r", stdin);
                tokenslist.erase(tokenslist.begin()+i, tokenslist.begin()+i+2);
                i--;
            }
            else {
                std::cerr<<"\033[31mError: no input file specified\033[0m"<<std::endl;
            }

        }
        if(tokenslist[i] == ">"){
            if( i+1 <tokenslist.size()){
                freopen(tokenslist[i+1].c_str(), "w", stdout);
                tokenslist.erase(tokenslist.begin()+i, tokenslist.begin()+i+2);
                i--;
            }
            else {
                std::cerr<<"\033[31mError: no output file specified\033[0m"<<std::endl;
            }
        }
    }


}

void SaveToTheFile(const std::vector<char*> &history){
    // this function saves the history to a file 
    std::ofstream out("historyFile.txt", std::ios::out | std::ios::trunc);
    if(!out){
        std::cerr<<"\033[31mError opening history file\033[0m"<<std::endl;
        return;
    }
    for(const auto &cmd : history){
        if(history.size() > MAX_HISTORY) break;
        out<<cmd<<std::endl;
    }
    out.close();
}

void LoadFromFile(std::vector<char*> &history){
    // this function loads the history from a file
    std::ifstream in("historyFile.txt");
    if(!in){
        std::cerr<<"\033[31mError opening history file\033[0m"<<std::endl;
        return;
    }
   std::string line;
    while(std::getline(in, line)){
        if(!line.empty())
            history.push_back(strdup(line.c_str()));
    }
    in.close();
}

void Search(int number, const std::vector<char*> &history){
    if(number < 1 || number > history.size()) return;
    std::cout<<history[number-1]<<std::endl;
}

std::vector<Command> ParseCommands(std::vector<std::string> &tokensList){
    std::vector<Command> command;
    std::vector<std::string> current; 

   for(const auto &tok : tokensList){
        if(tok == "&&" || tok == "||"){
            command.push_back(Command(current, tok));
            current.clear();
        } else {
            current.push_back(tok);
        }
    }
    if(!current.empty()){
        command.push_back(Command(current, ""));
    }
    return command;
}


int ExecuteCommand(const std::vector<char*> args){
    
    pid_t p_id = fork();

    if(p_id == 0){
        execvp(args[0], args.data());
        perror("\033[31mexecvp failed\033[0m");
        exit(1); // this means execvp failed
    }
    else if(p_id > 0){
        int status = 0;
        waitpid(p_id, &status, 0);
        if(WIFEXITED(status))
           return WEXITSTATUS(status); // return exit status of child
        else return 1; // abnormal termination
    }
    else {
       perror("\033[31mexecvp failed\033[0m");
       return 1;
       }
    for(size_t i = 0; i < args.size() - 1; ++i) {
        delete [] args[i];
    }
    return 0;
}

void ExecuteCommandsWithSep(std::vector<std::string> &tokensList){
    auto commands = ParseCommands(tokensList);
    int lastStatus = 0;

    if(commands.empty()) return;

    for(size_t i = 0; i < commands.size(); ++i){
    
        if(i > 0){
            std::string sep = commands[i-1].separator;
            if(sep == "&&" && lastStatus != 0) break;
            if(sep == "||" && lastStatus == 0) break;
        }

        std::vector<char*> argument;
        for(const auto &arg : commands[i].args){
            argument.push_back(strdup(arg.c_str()));
        }
        argument.push_back(nullptr);

        lastStatus = ExecuteCommand(argument);

        argument.clear();
        
    }}


int main(){
    std::string input; // to store command line
    std::vector<char*> history;
    // create file if not exists
    std::ofstream outfile("historyFile.txt", std::ios::app);
    LoadFromFile(history);

    while(true){
        char* user = getenv("USER");
        char hostname[128];
        gethostname(hostname, sizeof(hostname));
        char cwd[1024];
        getcwd(cwd, sizeof(cwd));
        std::cout<<"\033[32m"<<(user? user: ("Unknown")) <<"@"<<hostname<<"\033[0m:\033[34m"<<cwd<<"\033[0m ";

        // getting command line
        //std::cout<<"\033[36mMini shell> \033[0m"<<std::flush;
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
        if(input == "search"){
            std::cout<<"Enter command number to search: "<<std::flush;
            int num;
            std::cin>>num;
            std::cin.ignore();
            Search(num, history);
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
            else {
                if(chdir(tokenslist[1].c_str()) != 0){
                    perror("\033[31mcd failed\033[0m");
                }
                else {
                    if(getcwd(cwd, sizeof(cwd)) != nullptr){
                        setenv("PWD", cwd, 1);
                    }
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
    else if (tokenslist[0] == "help") {
        std::cout << "Welcome to the Mini Shell!\n";
        std::cout << "Built-in commands:\n";
        std::cout << "  cd [dir]       Change the current directory to 'dir'\n";
        std::cout << "  mkdir [dir]    Create a new directory named 'dir'\n";
        std::cout << "  clear          Clear the terminal screen\n";
        std::cout << "  echo [args]    Display arguments to the terminal\n";
        std::cout << "  whoami         Display the current username\n";
        std::cout << "  set VAR=VALUE  Set an environment variable\n";
        std::cout << "  unset VAR      Unset an environment variable\n";
        std::cout << "  history        Show command history\n";
        std::cout << "  search         Search command history by number\n";
        std::cout << "  exit           Exit the shell\n";
        continue;
    }
    else if(std::find(tokenslist.begin(), tokenslist.end(), "&&") != tokenslist.end() || 
            std::find(tokenslist.begin(), tokenslist.end(), "||") != tokenslist.end()){
        ExecuteCommandsWithSep(tokenslist);
        continue;
    }
       
   else if ( tokenslist[0] == "echo"){
        for(size_t i = 1; i<    tokenslist.size(); ++i){
           std::string arg = tokenslist[i];
              if(arg[0] == '$'){
                const char* var = getenv(arg.substr(1).c_str());
                if(var) std::cout<<var;
              }
              else {
                std::cout<<arg;
              }
              if(i != tokenslist.size() - 1) std::cout<<" ";
            }
        std::cout<<std::endl;
        continue;
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
   else if(tokenslist[0] == "set"){
    if(tokenslist.size() < 2 || tokenslist[1].find('=') == std::string::npos){
        std::cerr<<"\033[31mset: invalid format. Use VAR=VALUE\033[0m"<<std::endl;
    }
    else {
        auto pos = tokenslist[1].find('=');
        std::string var = tokenslist[1].substr(0, pos);
        std::string value = tokenslist[1].substr(pos + 1);
        if(setenv(var.c_str(), value.c_str(), 1) != 0){
            perror("setenv failed");
        }
    }
    continue;
    }
    else if(std::find(tokenslist.begin(), tokenslist.end(), "<") != tokenslist.end() ||
            std::find(tokenslist.begin(), tokenslist.end(), ">") != tokenslist.end()){
        handleRedirections(tokenslist);
    }
    else if( tokenslist[0] == "unset"){
        if(tokenslist.size() < 2){
            std::cerr<<"\033[31munset: missing variable name\033[0m"<<std::endl;
        }
        else {
            if(unsetenv(tokenslist[1].c_str()) != 0){
                perror("unsetenv failed");
            }
        }
        continue;
    }

    //---- External commands ----
    for(auto & t : tokenslist){
        args.push_back(strdup(t.c_str()));
    }
        args.push_back(nullptr); // execvp needs null-terminated array
 
        // child process for execute
        ExecuteCommand(args);
        args.clear();
        SaveToTheFile(history);
    }
    return 0;
}