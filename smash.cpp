#include <iostream>
#include <unistd.h>
#include <sys/wait.h>
#include <signal.h>
#include "Commands.h"
#include "signals.h"
//
int main(int argc, char* argv[]) {
    struct sigaction AL,CZ,CC;
    AL.sa_handler=&alarmHandler;
    AL.sa_flags=SA_RESTART;
    CZ.sa_handler = &ctrlZHandler;
    CZ.sa_flags=SA_RESTART;// SA_RESTART;
    CC.sa_handler = &ctrlCHandler;
    CC.sa_flags=SA_RESTART;
    if(sigaction(SIGALRM,&AL, nullptr)==-1)
    {
        perror("smash error: failed to set alarm handler");
    }
   if(sigaction(SIGTSTP,&CZ, nullptr)==-1) {//signal(SIGTSTP, ctrlZHandler)==SIG_ERR
        perror("smash error: failed to set ctrl-Z handler");
    }

    if(sigaction(SIGINT,&CC, nullptr)==-1){
        perror("smash error: failed to set ctrl-C handler");
    }

      SmallShell& smash = SmallShell::getInstance();
      externalSmash=&smash;
     while(is_running) {
         smash.get_chprompt();
         std::string cmd_line;
         std::getline(std::cin, cmd_line);
         smash.executeCommand(cmd_line.c_str());}
    return 0;
}