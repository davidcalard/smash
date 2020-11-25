#include <iostream>
#include <unistd.h>
#include <sys/wait.h>
#include <signal.h>
#include "Commands.h"
#include "signals.h"

SmallShell& smash = SmallShell::getInstance();

int main(int argc, char* argv[]) {

    if(signal(SIGTSTP , ctrlZHandler)==SIG_ERR) {
        perror("smash error: failed to set ctrl-Z handler");
    }
    if(signal(SIGINT , ctrlCHandler)==SIG_ERR) {
        perror("smash error: failed to set ctrl-C handler");
    }
    struct sigaction alarm_act;
    //memset(alarm_act, '\0', sizeof(alarm_act));
    alarm_act.sa_flags = SA_RESTART;
    alarm_act.sa_handler = alarmHandler;
    if (sigaction(SIGALRM, &alarm_act, NULL) < 0){
        perror("smash error: failed to set alarm handler");
    }

   while(true) {
       smash.printPrompt();
       std::string cmd_line;
       std::getline(std::cin, cmd_line);
       smash.executeCommand(cmd_line.c_str(), NOT_ALARM);
   }
   return 0;
}
