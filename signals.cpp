#include <iostream>
#include <signal.h>
#include "signals.h"
#include "Commands.h"

using namespace std;
extern SmallShell& smash;

void ctrlZHandler(int sig_num) {
    cout << "smash: got ctrl-Z" << endl;
    if (smash.getCurrCmd() != nullptr){
        smash.getJobsList().addJob(smash.getCurrCmd(), true);
        if (kill(smash.getCurrCmd()->getMyPid(), SIGTSTP) < 0){
            perror("smash error: kill failed");
            return;
        }
        cout << "smash: process " << smash.getCurrCmd()->getMyPid() << " was stopped" << endl;
    }
}

void ctrlCHandler(int sig_num) {
    cout << "smash: got ctrl-C" << endl;
    if (smash.getCurrCmd() != nullptr){
        if (kill(smash.getCurrCmd()->getMyPid(), SIGKILL) < 0){
            perror("smash error: kill failed");
            return;
        }
        cout << "smash: process " << smash.getCurrCmd()->getMyPid() << " was killed" << endl;
    }
}

void alarmHandler(int sig_num) {
    // TODO: Add your implementation
}
