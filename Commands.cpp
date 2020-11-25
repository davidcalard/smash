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
  for(std::string s; iss.str().find("|") != std::string::npos || iss >> s; ) {
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

SmallShell::SmallShell(): prompt("smash> "), last_dir_init(false), curr_cmd(nullptr) {}

SmallShell::~SmallShell() {
// TODO: add your implementation
}

/**
* Creates and returns a pointer to Command class which matches the given command line (cmd_line)
*/
Command * SmallShell::CreateCommand(const char* cmd_line) {
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
    string cmd_s = string(cmd_line);
    int pipe_index = cmd_s.find("|");
    if (pipe_index != string::npos){
        if (cmd_s.find("|&")){
            return new PipeCommand(cmd_line, pipe_index, ERR_PIPE);
        }
        else{
            return new PipeCommand(cmd_line, pipe_index, PIPE);
        }
    }
    else if (cmd_s.find("timeout") == 0) {
        return new TimeOutCommand(cmd_line);
    }
    else if (cmd_s.find("chprompt") == 0) {
        return new ChangePromptCommand(cmd_line);
    }
    else if (cmd_s.find("ls") == 0) {
        return new LetSeeCommand(cmd_line);
    }
    else if (cmd_s.find("showpid") == 0) {
        return new ShowPidCommand(cmd_line);
    }
    else if (cmd_s.find("pwd") == 0) {
        return new GetCurrDirCommand(cmd_line);
    }
    else if (cmd_s.find("cd") == 0) {
        return new ChangeDirCommand(cmd_line);
    }
    else if (cmd_s.find("jobs") == 0) {
        return new JobsCommand(cmd_line);
    }
    else if (cmd_s.find("kill") == 0) {
        return new KillCommand(cmd_line);
    }
    else if (cmd_s.find("fg") == 0) {
        return new ForegroundCommand(cmd_line);
    }
    else if (cmd_s.find("bg") == 0) {
        return new BackgroundCommand(cmd_line);
    }
    else if (cmd_s.find("quit") == 0) {
        return new QuitCommand(cmd_line);
    }
    else {
      return new ExternalCommand(cmd_line);
    }
    return nullptr;
}

void SmallShell::executeCommand(const char *cmd_line, int alarm_time) {
    Command* cmd = CreateCommand(cmd_line);
    smash.getJobsList().removeFinishedJobs();
    if (alarm_time != NOT_ALARM) cmd->setAlarmTime(alarm_time);
    cmd->execute();

  // for example:
  // Command* cmd = CreateCommand(cmd_line);
  // cmd->execute();
  // Please note that you must fork smash process for some commands (e.g., external commands....)

}

//---------------------------------------------------------------------------------//
//-------------------------------------Command-------------------------------------//
//---------------------------------------------------------------------------------//

Command::Command(const char* cmd_line): pid(0), alarm_time(NOT_ALARM) {
    cmd_num_args = _parseCommandLine(cmd_line,cmd_args);
    strcpy(cmd, cmd_line);
}

//---------------------------------------------------------------------------------//
//--------------------------------Built-In-Commands--------------------------------//
//---------------------------------------------------------------------------------//

void ChangePromptCommand::execute() {
    string newPrompt;
    if (getNumArguments() == 1) newPrompt = "smash> ";
    if (getNumArguments() == 2) newPrompt = getArguments()[1];
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
    int pid = getpid(); //this func always success
    cout << "smash pid is " << pid << endl;
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
    getcwd(smash.lastDir(), PATH_MAX);
    if (!smash.lastDir()){
        perror("smash error: getcwd failed");
        return;
    }
    smash.lastDirIsSet();
    if(chdir(buffer) < 0){
        perror("smash error: chdir failed");
        return;
    }
}

void JobsCommand::execute() {
    smash.getJobsList().removeFinishedJobs();
    smash.getJobsList().printJobsList();
}

void KillCommand::execute() {
    if (getNumArguments() != 3){
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
    if (kill(job->cmd->getMyPid(), sig) < 0){
        perror("smash error: kill failed");
        return;
    }
}

void ForegroundCommand::execute() {
    if (getNumArguments() > 2){
        cout << "smash error: fg: invalid arguments" << endl;
        return;
    }
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
    }
    cout << getCmd() << " : " << pid << endl;
    smash.setCurrCmd(cmd);
    smash.getJobsList().removeJobById(job_id);
    if(kill(pid, SIGCONT) < 0){
        perror("smash error: kill failed");
        smash.unsetCurrCmd();
        return;
    }
    if (waitpid(pid, NULL, NULL) < 0){
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
        cout << "sending SIGKILL signal to " << job_list_size << " jobs:" << endl;
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
        char* const args[] = {"/bin/bash", "-c", getCmd(), nullptr};
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
        smash.getJobsList().addJob(this); //TODO: check bg and fg
        if (waitpid(pid, NULL, WSTOPPED) < 0){
            perror("smash error: waitpid failed");
            return;
        }
        smash.getJobsList().removeLastAddedJob(); //TODO: check bg and fg
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

JobsList::JobsList(): max_job_id(0), job_list(std::map<int,JobEntry>()) {}

void JobsList::addJob(Command* cmd, bool isStopped){
    smash.getJobsList().removeFinishedJobs();
    max_job_id++;
    job_list.emplace(max_job_id, JobEntry(cmd,isStopped));
}

void JobsList::printJobsList() {
    smash.getJobsList().removeFinishedJobs();
    for (auto iterator = job_list.begin(); iterator != job_list.end(); ++iterator){
        time_t curr_time;
        time(&curr_time);
        cout << "[" << iterator.operator*().first << "]" <<
        iterator.operator*().second.cmd->getCmd() << ":" << iterator.operator*().second.cmd->getMyPid() << " " <<
        difftime(curr_time,iterator.operator*().second.start_time) << " secs";
        if(iterator.operator*().second.is_stopped) cout << " (stopped)";
        cout << endl;
    }
}

JobsList::JobEntry *JobsList::getJobById(int jobId) {
    auto iterator = smash.getJobsList().job_list.find(jobId);
    JobEntry* our_job = &iterator->second;
    return our_job;
}

void JobsList::removeJobById(int jobId) {
    smash.getJobsList().job_list.erase(jobId);
}

void JobsList::removeFinishedJobs() {
    for (auto iterator = smash.getJobsList().job_list.begin(); iterator != smash.getJobsList().job_list.end();){
        int flag = waitpid(iterator->second.cmd->getMyPid(), NULL, WNOHANG);
        if (flag < 0){
            perror("smash error: waitpid failed");
            return;
        }
        else if (flag == 0){
            ++iterator;
            continue;
        }
        else{
            auto temp = iterator;
            ++iterator;
            smash.getJobsList().removeJobById(temp->first);
        }
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

void JobsList::removeLastAddedJob() {
    job_list.erase(max_job_id);
}

//---------------------------------------------------------------------------------//
//---------------------------------Special-Command---------------------------------//
//---------------------------------------------------------------------------------//


PipeCommand::PipeCommand(const char *cmd_line, const int index, Mode mode) : Command(cmd_line), mode(mode) {
    cmd_num_args2 = _parseCommandLine(cmd_line+sizeof(char)*(index+1+mode), cmd_args2);
    strcpy(cmd2, sizeof(char)*index+getCmd());
}

void PipeCommand::execute() {

}

void TimeOutCommand::execute() {
    alarm(atoi(getArguments()[1])); //Always success
    string s_cmd(getCmd());
    smash.executeCommand(s_cmd.substr(s_cmd.find(getArguments()[2])).c_str(), atoi(getArguments()[1]));

}
