#include "Commands.h"

using namespace std;

const std::string WHITESPACE = " \n\r\t\f\v";

#if 0
#define FUNC_ENTRY()  \
  cerr << __PRETTY_FUNCTION__ << " --> " << endl;

#define FUNC_EXIT()  \
  cerr << __PRETTY_FUNCTION__ << " <-- " << endl;
#else
#define FUNC_ENTRY()
#define FUNC_EXIT()
#endif

#define DEBUG_PRINT cerr << "DEBUG: "

#define EXEC(path, arg) \
  execvp((path), (arg));

extern SmallShell& smash;

string _ltrim(const std::string& s)
{
  size_t start = s.find_first_not_of(WHITESPACE);
  return (start == std::string::npos) ? "" : s.substr(start);
}

string _rtrim(const std::string& s)
{
  size_t end = s.find_last_not_of(WHITESPACE);
  return (end == std::string::npos) ? "" : s.substr(0, end + 1);
}

string _trim(const std::string& s)
{
  return _rtrim(_ltrim(s));
}

int _parseCommandLine(const char* cmd_line, char** args) {
  FUNC_ENTRY()
  int i = 0;
  std::istringstream iss(_trim(string(cmd_line)).c_str());
  for(std::string s; iss >> s; ) {
      if(s.find("|") != std::string::npos) break;
      args[i] = (char*)malloc(s.length()+1);
      memset(args[i], 0, s.length()+1);
      strcpy(args[i], s.c_str());
      args[++i] = NULL;
  }
  return i;

  FUNC_EXIT()
}

bool _isBackgroundComamnd(const char* cmd_line) {
  const string str(cmd_line);
  return str[str.find_last_not_of(WHITESPACE)] == '&';
}

void _removeBackgroundSign(char* cmd_line) {
  const string str(cmd_line);
  // find last character other than spaces
  unsigned int idx = str.find_last_not_of(WHITESPACE);
  // if all characters are spaces then return
  if (idx == string::npos) {
    return;
  }
  // if the command line does not end with & then return
  if (cmd_line[idx] != '&') {
    return;
  }
  // replace the & (background sign) with space and then remove all tailing spaces.
  cmd_line[idx] = ' ';
  // truncate the command line string up to the last non-space character
  cmd_line[str.find_last_not_of(WHITESPACE, idx) + 1] = 0;
}

//---------------------------------------------------------------------------------//
//-----------------------------------Small-Shell-----------------------------------//
//---------------------------------------------------------------------------------//

SmallShell::SmallShell(): prompt("smash> "), last_dir_init(false), curr_cmd(nullptr), smash_pid(getpid()) {}

SmallShell::~SmallShell() {
// TODO: add your implementation
}

/**
* Creates and returns a pointer to Command class which matches the given command line (cmd_line)
*/
Command * SmallShell::CreateCommand(const std::string cmd_s) {
// For example:
/*
  string cmd_s = string(cmd_line);
  if (cmd_s.find("pwd") == 0) {
    return new GetCurrDirCommand(cmd_line);
  }
  else if ...
  .....
  else {
    return new ExternalCommand(cmd_line);
  }
*/
    int pipe_index = cmd_s.find_first_of("|");
    if (pipe_index != string::npos){
        if (string::npos != cmd_s.find("|&")){
            return new PipeCommand(cmd_s.c_str(), pipe_index+1, ERR_PIPE);
        }
        else{
            return new PipeCommand(cmd_s.c_str(), pipe_index+1, PIPE);
        }
    }
    int redir_index = cmd_s.find_first_of(">");
    if (redir_index != string::npos){
        if (cmd_s[redir_index+1] == '>'){
            return new RedirectionCommand(cmd_s.c_str(),redir_index+1, APPEND);
        }
        else{
            return new RedirectionCommand(cmd_s.c_str(), redir_index+1, OVERRIDE);
        }
    }
    else if (cmd_s.find("timeout") == 0) {
        return new TimeOutCommand(cmd_s.c_str());
    }
    else if (cmd_s.find("chprompt") == 0) {
        return new ChangePromptCommand(cmd_s.c_str());
    }
    else if (cmd_s.find("ls") == 0) {
        return new LetSeeCommand(cmd_s.c_str());
    }
    else if (cmd_s.find("showpid") == 0) {
        return new ShowPidCommand(cmd_s.c_str());
    }
    else if (cmd_s.find("pwd") == 0) {
        return new GetCurrDirCommand(cmd_s.c_str());
    }
    else if (cmd_s.find("cd") == 0) {
        return new ChangeDirCommand(cmd_s.c_str());
    }
    else if (cmd_s.find("jobs") == 0) {
        return new JobsCommand(cmd_s.c_str());
    }
    else if (cmd_s.find("kill") == 0) {
        return new KillCommand(cmd_s.c_str());
    }
    else if (cmd_s.find("fg") == 0) {
        return new ForegroundCommand(cmd_s.c_str());
    }
    else if (cmd_s.find("bg") == 0) {
        return new BackgroundCommand(cmd_s.c_str());
    }
    else if (cmd_s.find("quit") == 0) {
        return new QuitCommand(cmd_s.c_str());
    }
    else {
      return new ExternalCommand(cmd_s.c_str());
    }
    return nullptr;
}

void SmallShell::executeCommand(const std::string cmd_line, time_t alarm_time) {
    Command* cmd = CreateCommand(cmd_line);
    smash.getJobsList().removeFinishedJobs();
    if (alarm_time != NOT_ALARM){
        cmd->setAlarmTime(alarm_time);
        smash.alarm_jobs.push_front(cmd);
    }
    cmd->execute();

  // for example:
  // Command* cmd = CreateCommand(cmd_line);
  // cmd->execute();
  // Please note that you must fork smash process for some commands (e.g., external commands....)

}

//---------------------------------------------------------------------------------//
//-------------------------------------Command-------------------------------------//
//---------------------------------------------------------------------------------//

Command::Command(const char* cmd_line): pid(0), alarm_time(NOT_ALARM), cmd_job_id(UNDEFINED), is_pipe(false), pid2(UNDEFINED), is_external(false){
    strcpy(cmd, cmd_line);
    cmd_num_args = _parseCommandLine(cmd_line,cmd_args);
    time(&init_time);
}

//---------------------------------------------------------------------------------//
//--------------------------------Built-In-Commands--------------------------------//
//---------------------------------------------------------------------------------//

void ChangePromptCommand::execute() {
    string newPrompt;
    if (getNumArguments() == 1) newPrompt = "smash";
    else newPrompt = getArguments()[1];
    smash.changePrompt(newPrompt);
}

void LetSeeCommand::execute() {
    struct dirent **namelist;
    int n = scandir(".", &namelist, NULL, alphasort);
    if (n < 0)
        perror("smash error: scandir failed");
    for (int i = 2; i < n; i++) {
        printf("%s\n", namelist[i]->d_name);
    }
}

void ShowPidCommand::execute() {
    cout << "smash pid is " << smash.getSmashPid() << endl;
}

void GetCurrDirCommand::execute() {
    char buffer[PATH_MAX];
    getcwd(buffer, PATH_MAX);
    if (!buffer){
        perror("smash error: getcwd failed");
        return;
    }
    cout << buffer << endl;
}

void ChangeDirCommand::execute() {
    if (getNumArguments() > 2){
        cout << "smash error: cd: too many arguments" << endl;
        return;
    }
    char buffer[PATH_MAX];
    if (!strcmp(getArguments()[1],"-")){
        if (!smash.isLastDirSet()){
            cout << "smash error: cd: OLDPWD not set" << endl;
            return;
        }
        strcpy(buffer, smash.lastDir());
    }
    else {
        strcpy(buffer, getArguments()[1]);
    }
    char buffer_last[PATH_MAX];
    if (!getcwd(buffer_last, PATH_MAX)){
        perror("smash error: getcwd failed");
        return;
    }
    if(chdir(buffer) < 0){
        perror("smash error: chdir failed");
        return;
    }
    else{
        strcpy(smash.lastDir(), buffer_last);
        smash.lastDirIsSet();
    }
}

void JobsCommand::execute() {
    smash.getJobsList().removeFinishedJobs();
    smash.getJobsList().printJobsList();
}

void KillCommand::execute() {
    if (getNumArguments() != 3 || *getArguments()[1] != '-' || atoi(getArguments()[2]) == 0 || atoi(getArguments()[1]+1) == 0){
        cout << "smash error: kill: invalid arguments" << endl;
        return;
    }
    int sig = atoi(getArguments()[1]+1);
    int job_id =  atoi(getArguments()[2]);
    JobsList::JobEntry* job = smash.getJobsList().getJobById(job_id);
    if (!job){
        cout << "smash error: kill: job-id " << job_id <<" does not exist" << endl;
        return;
    }
    int pid_kill = job->cmd->getMyPid();
    if (kill(pid_kill, sig) < 0){
        perror("smash error: kill failed");
        return;
    }
    cout << "signal number " << sig << " was sent to pid " << pid_kill << endl;
}

void ForegroundCommand::execute() {
    if (getNumArguments() > 2){
        cout << "smash error: fg: invalid arguments" << endl;
        return;
    }
    bool is_stopped = false;
    int pid, job_id;
    Command* cmd;
    if (getNumArguments() == 1){
        auto iterator = smash.getJobsList().job_list.rbegin();
        if (iterator == smash.getJobsList().job_list.rend()){
            cout << "smash error: fg: jobs list is empty" << endl;
            return;
        }
        pid = iterator->second.cmd->getMyPid();
        job_id = iterator->first;
        cmd = iterator->second.cmd;
        is_stopped = iterator->second.is_stopped;
    }
    else {
        auto iterator = smash.getJobsList().job_list.find(atoi(getArguments()[1]));
        if (iterator == smash.getJobsList().job_list.end()){
            cout << "smash error: fg: job-id " << getArguments()[1] << " does not exist" << endl;
            return;
        }
        pid = iterator->second.cmd->getMyPid();
        job_id = iterator->first;
        cmd = iterator->second.cmd;
        is_stopped = iterator->second.is_stopped;
    }
    smash.setCurrCmd(cmd);
    smash.getJobsList().removeJobById(job_id);
    cout << cmd->getCmd() << " : " << pid << endl;
    if(is_stopped){
        if(kill(pid, SIGCONT) < 0){
            perror("smash error: kill failed");
            smash.unsetCurrCmd();
            return;
        }
    }
    if (waitpid(pid, NULL, WUNTRACED) < 0){
        perror("smash error: waitpid failed");
        smash.unsetCurrCmd();
        return;
    }
    smash.unsetCurrCmd();
}

void BackgroundCommand::execute() {
    if (getNumArguments() > 2){
        cout << "smash error: bg: invalid arguments" << endl;
        return;
    }
    else if (getNumArguments() == 1){
        for(auto iterator = smash.getJobsList().job_list.rbegin(); iterator != smash.getJobsList().job_list.rend(); ++iterator){
            if (iterator->second.is_stopped){
                cout << iterator->second.cmd->getCmd() << " : " << iterator->second.cmd->getMyPid() << endl;
                iterator->second.resume();
                if(kill(iterator->second.cmd->getMyPid(), SIGCONT) < 0){
                    perror("smash error: kill failed");
                    return;
                }
                return;
            }
        }
        cout << "smash error: bg: there is no stopped jobs to resume" << endl;
    }
    else{
        auto iterator = smash.getJobsList().job_list.find(atoi(getArguments()[1]));
        if (iterator == smash.getJobsList().job_list.end()){
            cout << "smash error: bg: job-id " << getArguments()[1] << " does not exist" << endl;
            return;
        }
        if (!iterator->second.is_stopped){
            cout << "smash error: bg: job-id " << getArguments()[1] << " is already running in the background" << endl;
            return;
        }
        cout << iterator->second.cmd->getCmd() << " : " << iterator->second.cmd->getMyPid() << endl;
        iterator->second.resume();
        if(kill(iterator->second.cmd->getMyPid(), SIGCONT) < 0){
            perror("smash error: kill failed");
            return;
        }
    }
}

void QuitCommand::execute() {
    if (getNumArguments() > 1 && !strcmp(getArguments()[1], "kill")){
        int job_list_size = smash.getJobsList().job_list.size();
        cout << "smash: sending SIGKILL signal to " << job_list_size << " jobs:" << endl;
        for (auto iterator = smash.getJobsList().job_list.begin(); iterator != smash.getJobsList().job_list.end(); ++iterator){
            cout << iterator->second.cmd->getMyPid() << ": " << iterator->second.cmd->getCmd() << endl;
        }
        smash.getJobsList().killAllJobs();
    }
    exit(0); //TODO: check
}

void ExternalCommand::execute() {
    int pid = fork();
    if (pid == 0){
        setpgrp();
        if (_isBackgroundComamnd(getCmd())){
            _removeBackgroundSign(getCmd());
        }
        char* const args[] = {(char*)"/bin/bash", (char*)"-c", getCmd(), nullptr};
        execv("/bin/bash", (char* const*)args);
        perror("smash error: execv failed");
        exit(0);
    }
    else if (pid > 0){
        setPid(pid);
        if(_isBackgroundComamnd(getCmd())){
            smash.getJobsList().addJob(this);
            return;
        }
        smash.setCurrCmd(this);
        //TODO: check bg and fg
        if (waitpid(pid, NULL, WSTOPPED) < 0){
            perror("smash error: waitpid failed");
            return;
        }
        //TODO: check bg and fg
        smash.unsetCurrCmd();
    }
    else{
        perror("smash error: fork failed");
        return;
    }
}

//---------------------------------------------------------------------------------//
//------------------------------------Jobs-List------------------------------------//
//---------------------------------------------------------------------------------//

JobsList::JobEntry::JobEntry(Command* cmd, bool is_stopped):
    cmd(cmd), is_stopped(is_stopped) {
    time(&start_time);
}

JobsList::JobsList(): job_list(std::map<int,JobEntry>()) {}

void JobsList::addJob(Command* cmd, bool isStopped){
    smash.getJobsList().removeFinishedJobs();
    if(cmd->getCmdJobID() == UNDEFINED){
        cmd->setCmdJobId(getMaxAvailableID());
    }
    job_list.emplace(cmd->getCmdJobID(), JobEntry(cmd,isStopped));
}

void JobsList::printJobsList() {
    smash.getJobsList().removeFinishedJobs();
    for (auto iterator = job_list.begin(); iterator != job_list.end(); ++iterator){
        time_t curr_time;
        //time(&curr_time);
        curr_time = time(NULL);
        cout << "[" << iterator.operator*().first << "] " <<
        iterator.operator*().second.cmd->getCmd() << " : " << iterator.operator*().second.cmd->getMyPid() << " " <<
        difftime(curr_time,iterator.operator*().second.start_time) << " secs";
        if(iterator.operator*().second.is_stopped) cout << " (stopped)";
        cout << endl;
    }
}

JobsList::JobEntry *JobsList::getJobById(int jobId) {
    auto iterator = smash.getJobsList().job_list.find(jobId);
    if(iterator == smash.getJobsList().job_list.end()) return nullptr;
    JobEntry* our_job = &iterator->second;
    return our_job;
}

void JobsList::removeJobById(int jobId) {
    smash.getJobsList().job_list.erase(jobId);
}

void JobsList::removeFinishedJobs() {
    for (auto iterator = smash.getJobsList().job_list.begin(); iterator != smash.getJobsList().job_list.end();){
        bool p_flag[3] = {iterator->second.cmd->isPipe(), false, false};
        if(p_flag[PIPE]){
            PipeCommand* our_command = static_cast<PipeCommand*>(iterator->second.cmd);
            p_flag[FIRST] = our_command->is_done[0];
            p_flag[SECOND] = our_command->is_done[1];
        }
        if(!p_flag[PIPE] || (p_flag[PIPE] && !p_flag[FIRST])) {
            int flag = waitpid(p_flag[PIPE] ? iterator->second.cmd->getMyPid2() : iterator->second.cmd->getMyPid(), NULL, WNOHANG);
            if (flag < 0) {
                perror("smash error: waitpid failed");
                return;
            } else if (flag == 0) {
                ++iterator;
                continue;
            }
            if(p_flag[PIPE]) p_flag[FIRST] = true;
        }
        if(p_flag[PIPE]){
            PipeCommand* our_command = static_cast<PipeCommand*>(iterator->second.cmd);
            our_command->is_done[0] = p_flag[FIRST];
            if(!p_flag[SECOND]) {
                int flag2 = waitpid(iterator->second.cmd->getMyPid(), NULL, WNOHANG);
                if (flag2 < 0) {
                    perror("smash error: waitpid failed");
                    return;
                } else if (flag2 == 0) {
                    ++iterator;
                    continue;
                }
                our_command->is_done[1] = true;
                p_flag[SECOND] = true;
            }
        }
        auto temp = iterator;
        ++iterator;
        if(p_flag[PIPE] && (!p_flag[FIRST] || !p_flag[SECOND])) continue;
        smash.getJobsList().removeJobById(temp->first);
    }
}

void JobsList::killAllJobs() {
    for (auto iterator = smash.getJobsList().job_list.begin(); iterator != smash.getJobsList().job_list.end(); ++iterator){
        if(kill(iterator->second.cmd->getMyPid(), SIGKILL) < 0){
            perror("smash error: kill failed");
            return;
        }
    }
}

int JobsList::getMaxAvailableID() {
    if(job_list.rbegin() == job_list.rend()) return 1;
    else return job_list.rbegin()->first+1;
}

//---------------------------------------------------------------------------------//
//---------------------------------Special-Command---------------------------------//
//---------------------------------------------------------------------------------//


PipeCommand::PipeCommand(const char *cmd_line, const int index, Mode mode) : Command(cmd_line), mode(mode) {
    strcpy(cmd2, sizeof(char)*(index+mode)+getCmd());
    getCmd()[index-1] = '\0';
    strcpy(cmd1, getCmd());
    getCmd()[index-1] = '|';
    setIsPipe(true);
    is_done[0] = false;
    is_done[1] = false;
}

void PipeCommand::execute() {
    bool bg_flag = _isBackgroundComamnd(cmd2);
    if (bg_flag) _removeBackgroundSign(cmd2);
    int p[2];
    if (pipe(p) < 0){
        perror("smash error: pipe failed");
        return;
    }
    int pid1 = fork();
    if (pid1 < 0){
        perror("smash error: fork failed");
        return;
    }
    if (pid1 == 0){
        setpgrp();
        if (mode == PIPE){
            if (dup2(p[WRITE], fileno(stdout)) < 0){
                perror("smash error: dup2 failed");
                return;
            }
        }
        else if (mode == ERR_PIPE){
            if (dup2(p[WRITE], fileno(stderr)) < 0){
                perror("smash error: dup2 failed");
                return;
            }
        }
        close(p[READ]);
        close(p[WRITE]);
        Command* our_cmd1 = smash.CreateCommand(string(cmd1));
        if(our_cmd1->is_external){
            char* const args[] = {(char*)"/bin/bash", (char*)"-c", cmd1, nullptr};
            execv("/bin/bash", (char* const*)args);
            perror("smash error: execv failed");
            exit(0);
        }
        smash.executeCommand(string(cmd1), NOT_ALARM);
        return;
    }
    else {
        int pid2 = fork();
        if (pid2 < 0) {
            perror("smash error: fork failed");
            return;
        }
        if (pid2 == 0) {
            setpgrp();
            if (dup2(p[READ], fileno(stdin)) < 0){
                perror("smash error: dup2 failed");
                return;
            }
            close(p[READ]);
            close(p[WRITE]);
            Command* our_cmd2 = smash.CreateCommand(string(cmd2));
            if(our_cmd2->is_external){
                char* const args[] = {(char*)"/bin/bash", (char*)"-c", cmd2, nullptr};
                execv("/bin/bash", (char* const*)args);
                perror("smash error: execv failed");
                exit(0);
            }
            smash.executeCommand(string(cmd2), NOT_ALARM);
            return;
        }
        else{
            this->setPid(pid2);
            this->setPid2(pid1);
            close(p[READ]);
            close(p[WRITE]);
            if (!bg_flag) {
                smash.setCurrCmd(this);
                if(waitpid(pid1, NULL, 0) < 0){
                    perror("smash error: waitpid failed");
                    return;
                }
                if (waitpid(pid2, NULL, 0) < 0) {
                    perror("smash error: waitpid failed");
                    return;
                }
                smash.unsetCurrCmd();
            }
            else{
                smash.getJobsList().addJob(this);
            }
        }
    }
}

void TimeOutCommand::execute() {
    time_t next_alarm_time(atoi(getArguments()[1]));
    time_t curr_time;
    time(&curr_time);
    for(auto iterator = smash.alarm_jobs.begin(); iterator != smash.alarm_jobs.end(); ++iterator){
        time_t temp = (iterator.operator*()->getAlarmTime() - difftime(curr_time, iterator.operator*()->getInitTime()));
        if(temp < next_alarm_time){
            next_alarm_time = temp;
        }
    }
    alarm(next_alarm_time); //Always success
    string s_cmd(getCmd());
    smash.executeCommand(s_cmd.substr(s_cmd.find(getArguments()[2])).c_str(), atoi(getArguments()[1]));
}

RedirectionCommand::RedirectionCommand(const char *cmd_line, const int index, Mode mode) : Command(cmd_line), index(index), mode(mode) {
    command = (char*)malloc(index*sizeof(char));
    strncpy(command, cmd_line, index-1);
}

RedirectionCommand::~RedirectionCommand() {
    free(command);
}

void RedirectionCommand::execute() {
    string output_s = _trim(getCmd()+sizeof(char)*(index+mode));
    int old_out = dup(fileno(stdout));
    if (old_out < 0){
        perror("smash error: dup2 failed");
        return;
    }
    int output;
    if(mode == OVERRIDE){
        output = open(output_s.c_str(), O_WRONLY | O_CREAT | O_TRUNC, 0777);
        if (output < 0){
            perror("smash error: waitpid failed");
            return;
        }
    }
    else {
        output = open(output_s.c_str(), O_WRONLY | O_CREAT | O_APPEND, 0777);
        if (output < 0){
            perror("smash error: waitpid failed");
            return;
        }
    }
    if (dup2(output, fileno(stdout)) < 0){
        perror("smash error: dup2 failed");
        return;
    }
    close(output);
    smash.executeCommand(command, NOT_ALARM);
    if (dup2(old_out, fileno(stdout)) < 0){
        perror("smash error: dup2 failed");
        return;
    }
    close(old_out);
}



