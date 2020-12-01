#include <iostream>
#include <unistd.h>
#include <sys/wait.h>
#include <signal.h>
#include "Commands.h"
#include "signals.h"

SmallShell& smash = SmallShell::getInstance();

int main(int argc, char* argv[]) {

    struct sigaction alarm_act, ctrlz_act, ctrlc_act;
    alarm_act.sa_flags = SA_RESTART;
    ctrlc_act.sa_flags = SA_RESTART;
    ctrlz_act.sa_flags = SA_RESTART;
    alarm_act.sa_handler = alarmHandler;
    ctrlc_act.sa_handler = ctrlCHandler;
    ctrlz_act.sa_handler = ctrlZHandler;

    if(sigaction(SIGTSTP , &ctrlz_act, NULL)< 0) {
        perror("smash error: failed to set ctrl-Z handler");
    }
    if(sigaction(SIGINT , &ctrlc_act, NULL) < 0) {
        perror("smash error: failed to set ctrl-C handler");
    }
    if (sigaction(SIGALRM, &alarm_act, NULL) < 0){
        perror("smash error: failed to set alarm handler");
    }

   while(true) {
       if (!smash.isOriginalShell()) break;
       smash.printPrompt();
       std::string cmd_line;
       std::getline(std::cin, cmd_line);
       smash.executeCommand(cmd_line, time_t(NOT_ALARM));
   }
   return 0;
}
