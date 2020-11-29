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
    /*else{
        smash.printPrompt();
        cout << flush;
    }*/
}

void ctrlCHandler(int sig_num) {
    cout << "smash: got ctrl-C" << endl;
    if (smash.getCurrCmd() != nullptr){
        if (kill(smash.getCurrCmd()->getMyPid(), SIGKILL) < 0){
            perror("smash error: kill failed");
            return;
        }
        cout << "smash: process " << smash.getCurrCmd()->getMyPid() << " was killed" << endl;
        smash.getJobsList().removeFinishedJobs();
    }
    /*else{
        smash.printPrompt();
        cout << flush;
    }*/
}

void alarmHandler(int sig_num) {
    cout << "smash got an alarm" << endl;
    time_t curr_time;
    time(&curr_time);
    //string str(smash.getJobsList().job_list.rbegin()->second.cmd->getCmd());
    //bool bg_flag = str[str.find_last_not_of(" ")] == '&';
    for (auto iterator = smash.alarm_jobs.begin(); iterator != smash.alarm_jobs.end(); ) {
        if (iterator.operator*()->getAlarmTime() <= difftime(curr_time,iterator.operator*()->getInitTime())){
            cout << "smash: " << iterator.operator*()->getCmd() << " timed out!" << endl;
            if(kill(iterator.operator*()->getMyPid(), SIGKILL) < 0){
                perror("smash error: kill failed");
                return;
            }
            auto temp = iterator;
            ++iterator;
            smash.alarm_jobs.remove(temp.operator*());
            continue;
        }
        ++iterator;
    }
    time_t next_alarm_time = smash.alarm_jobs.begin() == smash.alarm_jobs.end() ? NOT_ALARM :
            (smash.alarm_jobs.begin().operator*()->getAlarmTime() - difftime(curr_time, smash.alarm_jobs.begin().operator*()->getInitTime()));
    if(next_alarm_time != NOT_ALARM) {
        for (auto iterator = smash.alarm_jobs.begin(); iterator != smash.alarm_jobs.end(); ++iterator) {
            time_t temp = (iterator.operator*()->getAlarmTime() -
                           difftime(curr_time, iterator.operator*()->getInitTime()));
            if (temp < next_alarm_time) {
                next_alarm_time = temp;
            }
        }
        alarm(next_alarm_time); //Always success
    }
    //smash.printPrompt();
    //cout << flush;
}
