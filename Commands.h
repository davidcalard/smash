#ifndef SMASH_COMMAND_H_
#define SMASH_COMMAND_H_

#include <utility>
#include <map>
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
#include <list>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#define COMMAND_LINE_MAX_LENGTH (80)
#define COMMAND_MAX_ARGS (20)
#define COMMAND_MAX_LENTH (50)
#define PIPE 0
#define ERR_PIPE 1
#define NOT_ALARM -1
#define READ 0
#define WRITE 1
#define OVERRIDE 0
#define APPEND 1
#define UNDEFINED -1
#define PIPE 0
#define FIRST 1
#define SECOND 2
#define COUNT_MAX 1024

typedef bool Mode;

class Command {
    char* cmd_args[COMMAND_MAX_ARGS];
    char cmd[COMMAND_LINE_MAX_LENGTH];
    int cmd_num_args;
    int pid;
    time_t alarm_time;
    time_t init_time;
    int cmd_job_id;
    bool is_pipe;
    int pid2;
public:
    Command(char* cmd_line, char* full_cmd);
    virtual ~Command() {}
    virtual void execute() = 0;
    char** getArguments() { return cmd_args; }
    int getNumArguments() { return cmd_num_args; }
    char* getCmd() { return cmd; }
    int& getMyPid() { return pid; }
    void setPid(int n) { pid = n; }
    time_t getAlarmTime() { return alarm_time; }
    time_t getInitTime() { return init_time; }
    void setAlarmTime(time_t m) { alarm_time = m; }
    int getCmdJobID() { return cmd_job_id; }
    void setCmdJobId(int job_id) { cmd_job_id = job_id; }
    bool isPipe() { return is_pipe; }
    void setIsPipe(bool status) { is_pipe = status; }
    int& getMyPid2() { return pid2; }
    void setPid2(int n) { pid2 = n; }
    bool is_external;
    bool is_gone;
    bool is_bg;
    char full_cmd[COMMAND_LINE_MAX_LENGTH];
    //virtual void prepare();
    // virtual void cleanup();
};

class BuiltInCommand : public Command {
public:
    BuiltInCommand(char* cmd_line, char* full_cmd) : Command(cmd_line,full_cmd) {}
    virtual ~BuiltInCommand() {}
};

class ExternalCommand : public Command {
public:
    ExternalCommand(char* cmd_line, char* full_cmd): Command(cmd_line,full_cmd) { is_external = true; }
    virtual ~ExternalCommand() {}
    void execute() override;
};

class PipeCommand : public Command {
    char* cmd_args2[COMMAND_MAX_ARGS];
    char* cmd_args1[COMMAND_MAX_ARGS];
    char cmd2[COMMAND_LINE_MAX_LENGTH];
    Mode mode;
    char cmd1[COMMAND_LINE_MAX_LENGTH];
public:
    bool is_done[2];
    PipeCommand(char* cmd_line, char* full_cmd, const int index, Mode mode);
    virtual ~PipeCommand() {}
    void execute() override;
};

class RedirectionCommand : public Command {
    Mode mode;
    const int index;
    char* command;
public:
    RedirectionCommand(char* cmd_line, char* full_cmd, const int index, Mode mode);
    ~RedirectionCommand() override;
    void execute() override;
    Mode getMode() { return mode; }
    //void prepare() override;
    // void cleanup() override;
};

class ChangePromptCommand : public BuiltInCommand {
public:
    ChangePromptCommand(char* cmd_line, char* full_cmd): BuiltInCommand(cmd_line,full_cmd) {}
    virtual ~ChangePromptCommand() {}
    void execute() override;
};

class LetSeeCommand : public BuiltInCommand {
public:
    LetSeeCommand(char* cmd_line, char* full_cmd): BuiltInCommand(cmd_line,full_cmd) {}
    virtual ~LetSeeCommand() {}
    void execute() override;
};

class ChangeDirCommand : public BuiltInCommand {
public:
    ChangeDirCommand(char* cmd_line, char* full_cmd): BuiltInCommand(cmd_line,full_cmd) {};
    virtual ~ChangeDirCommand() {}
    void execute() override;
};

class GetCurrDirCommand : public BuiltInCommand {
public:
    GetCurrDirCommand(char* cmd_line, char* full_cmd): BuiltInCommand(cmd_line,full_cmd) {}
    virtual ~GetCurrDirCommand() {}
    void execute() override;
};

class ShowPidCommand : public BuiltInCommand {
public:
    ShowPidCommand(char* cmd_line, char* full_cmd): BuiltInCommand(cmd_line,full_cmd) {}
    virtual ~ShowPidCommand() {}
    void execute() override;
};

class JobsList;

class QuitCommand : public BuiltInCommand {
public:
    QuitCommand(char* cmd_line, char* full_cmd): BuiltInCommand(cmd_line,full_cmd) {}
    virtual ~QuitCommand() {}
    void execute() override;
};

class JobsList {
public:
    class JobEntry {
    public:
        Command* cmd;
        bool is_stopped;
        time_t start_time;
        JobEntry(Command* cmd, bool is_stopped);
        void resume() { is_stopped = false; }
    };
    std::map<int,JobEntry> job_list;
    JobsList();
    ~JobsList() = default;
    void addJob(Command* cmd, bool isStopped = false);
    void printJobsList();
    void killAllJobs();
    void removeFinishedJobs();
    JobEntry * getJobById(int jobId);
    void removeJobById(int jobId);
    int getMaxAvailableID();
    //JobEntry * getLastJob(int* lastJobId);
    //JobEntry *getLastStoppedJob(int *jobId);
};

class JobsCommand : public BuiltInCommand {
public:
    JobsCommand(char* cmd_line, char* full_cmd): BuiltInCommand(cmd_line,full_cmd) {}
    virtual ~JobsCommand() {}
    void execute() override;
};

class KillCommand : public BuiltInCommand {
public:
    KillCommand(char* cmd_line, char* full_cmd): BuiltInCommand(cmd_line,full_cmd) {}
    virtual ~KillCommand() {}
    void execute() override;
};

class ForegroundCommand : public BuiltInCommand {
public:
    ForegroundCommand(char* cmd_line, char* full_cmd): BuiltInCommand(cmd_line,full_cmd) {}
    virtual ~ForegroundCommand() {}
    void execute() override;
};

class BackgroundCommand : public BuiltInCommand {
public:
    BackgroundCommand(char* cmd_line, char* full_cmd): BuiltInCommand(cmd_line,full_cmd) {}
    virtual ~BackgroundCommand() {}
    void execute() override;
};

class TimeOutCommand : public Command {
public:
    TimeOutCommand(char* cmd_line, char* full_cmd): Command(cmd_line,full_cmd) {}
    virtual ~TimeOutCommand() {}
    void execute() override;
};

class CPCommand : public Command {
public:
    CPCommand(char* cmd_line, char* full_cmd): Command(cmd_line,full_cmd) {}
    virtual ~CPCommand() {}
    void execute() override;
};

class SmallShell {
private:
    std::string prompt;
    char last_dir[PATH_MAX];
    bool last_dir_init;
    JobsList jobs_list;
    Command* curr_cmd;
    int smash_pid;
    SmallShell();

public:
    Command *CreateCommand(std::string cmd_line, std::string full_cmd);
    SmallShell(SmallShell const &) = delete; // disable copy ctor
    void operator=(SmallShell const &) = delete; // disable = operator
    static SmallShell &getInstance() // make SmallShell singleton
    {
        static SmallShell instance; // Guaranteed to be destroyed.
        // Instantiated on first use.
        return instance;
    }
    ~SmallShell();
    std::list<Command*> alarm_jobs;
    void executeCommand(const std::string cmd_line,const std::string full_cmd, time_t alarm_time);
    void printPrompt() { std::cout << prompt; }
    char* lastDir() { return last_dir; }
    bool isLastDirSet() { return last_dir_init; }
    void lastDirIsSet() { last_dir_init = true; }
    void changePrompt(std::string s) { prompt = s + "> "; }
    void setCurrCmd(Command* cmd) { curr_cmd = cmd; }
    void unsetCurrCmd() { curr_cmd = nullptr; }
    Command* getCurrCmd() { return curr_cmd; }
    JobsList& getJobsList() { return jobs_list; }
    bool isOriginalShell() { return smash_pid == getpid(); }
    int getSmashPid() { return smash_pid; }
};
#endif //SMASH_COMMAND_H_
