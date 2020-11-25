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
    else{
        smash.printPrompt();
        cout << flush;
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
    else{
        smash.printPrompt();
        cout << flush;
    }
}

void alarmHandler(int sig_num) {
    cout << "smash got an alarm" << endl;
    //string str(smash.getJobsList().job_list.rbegin()->second.cmd->getCmd());
    //bool bg_flag = str[str.find_last_not_of(" ")] == '&';
    for (auto iterator = smash.getJobsList().job_list.begin(); iterator != smash.getJobsList().job_list.end(); ++iterator) {
        time_t curr_time;
        time(&curr_time);
        if (iterator->second.cmd->getAlarmTime() <= difftime(curr_time,iterator.operator*().second.start_time)){
            cout << "smash: " << iterator->second.cmd->getCmd() << " timed out!" << endl;
            if(kill(iterator->second.cmd->getMyPid(), SIGKILL) < 0){
                perror("smash error: kill failed");
                return;
            }
        }
    }
    //smash.printPrompt();
    //cout << flush;
}
