#include <unistd.h>
#include <string.h>
#include <iostream>
#include <vector>
#include <sstream>
#include <sys/wait.h>
#include <iomanip>
#include <stdio.h>
#include <stdlib.h>
#include <sys/dir.h>
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

// TODO: Add your implementation for classes in Commands.h

SmallShell::SmallShell(): prompt("smash> "){}

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
  if (cmd_s.find("chprompt") == 0) {
    return new ChangePromptCommand(cmd_line);
  }
  else if (cmd_s.find("ls") == 0) {
  	return new LetSeeCommand(cmd_line);
  }
  /*else if (cmd_s.find("showpid") == 0) {
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
  }*/
  return nullptr;
}

void SmallShell::executeCommand(const char *cmd_line) {

    Command* cmd = CreateCommand(cmd_line);
    cmd->execute();

  // for example:
  // Command* cmd = CreateCommand(cmd_line);
  // cmd->execute();
  // Please note that you must fork smash process for some commands (e.g., external commands....)

}

//----------------------------------------------------------------------------//
//----------------------------------OUR-CODE----------------------------------//
//----------------------------------------------------------------------------//

//-----------------------Command------------------------//

Command::Command(const char* cmd_line): pid(0) {
    cmd_num_args = _parseCommandLine(cmd_line,cmd_args);
    strcpy(cmd, cmd_line);
}

char **Command::getArguments() {
    return cmd_args;
}

int Command::getNumArguments() {
    return cmd_num_args;
}

char *Command::getCmd() {
    return cmd;
}

int Command::getMyPid() {
    return pid;
}

void Command::setPid(int n) {
    pid = n;
}

//------------------------------------------------------//





CommandsHistory::CommandHistoryEntry::CommandHistoryEntry(): dir(nullptr) {}

void CommandsHistory::addRecord(const char *cmd_line) {
    this->last_dir.dir = std::string(cmd_line);
}

void CommandsHistory::printHistory() {
    cout << this->last_dir.dir << endl;
}

void ChangePromptCommand::execute() {
    string newPrompt;
    if (getNumArguments() == 1) newPrompt = "smash> ";
    if (getNumArguments() == 2) newPrompt = getArguments()[1];
    smash.changePrompt(newPrompt);
}

void LetSeeCommand::execute() {
    struct dirent **namelist;
    int i,n;


    n = scandir(".", &namelist, NULL, alphasort);
    if (n < 0)
        perror("scandir");
    else {
        for (i = 2; i < n; i++) {
            printf("%s\n", namelist[i]->d_name);
        }
    }
}



//-----------------------JobsList------------------------//

JobsList::JobEntry::JobEntry(Command* cmd, bool is_stopped):
    cmd(cmd), is_stopped(is_stopped) {
    time(&start_time);
}

JobsList::JobsList(): max_job_id(0), job_list(std::map<int,JobEntry>()) {}

void JobsList::addJob(Command* cmd, bool isStopped){
    max_job_id++;
    job_list.emplace(max_job_id, JobEntry(cmd,isStopped));
}

void JobsList::printJobsList() {
    for (auto iterator = job_list.begin(); iterator != job_list.end(); ++iterator){
        time_t* curr_time;
        time(curr_time);
        cout << "[" << iterator.operator*().first << "]" <<
        iterator.operator*().second.cmd->getCmd() << ":" << iterator.operator*().second.cmd->getMyPid() <<
        difftime(*curr_time,iterator.operator*().second.start_time);
        if(iterator.operator*().second.is_stopped) cout << "(stopped)";
        cout << endl;
    }
}







