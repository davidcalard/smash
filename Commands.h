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

#define COMMAND_LINE_MAX_LENGTH (80)
#define COMMAND_MAX_ARGS (20)
#define COMMAND_MAX_LENTH (50)

class Command {
    char* cmd_args[COMMAND_MAX_ARGS];
    int cmd_num_args;
    char cmd[COMMAND_LINE_MAX_LENGTH];
    int pid;
public:
    Command(const char* cmd_line);
    virtual ~Command() {}
    virtual void execute() = 0;
    char** getArguments() { return cmd_args; }
    int getNumArguments() { return cmd_num_args; }
    char* getCmd() { return cmd; }
    int& getMyPid() { return pid; }
    void setPid(int n) { pid = n; }
    //virtual void prepare();
    // virtual void cleanup();
};

class BuiltInCommand : public Command {
public:
    BuiltInCommand(const char* cmd_line) : Command(cmd_line) {}
    virtual ~BuiltInCommand() {}
};

class ExternalCommand : public Command {
public:
    ExternalCommand(const char* cmd_line) : Command(cmd_line) {}
    virtual ~ExternalCommand() {}
    void execute() override;
};

class PipeCommand : public Command {
    // TODO: Add your data members
public:
    PipeCommand(const char* cmd_line);
    virtual ~PipeCommand() {}
    void execute() override;
};

class RedirectionCommand : public Command {
    // TODO: Add your data members
public:
    explicit RedirectionCommand(const char* cmd_line);
    virtual ~RedirectionCommand() {}
    void execute() override;
    //void prepare() override;
    // void cleanup() override;
};

class ChangePromptCommand : public BuiltInCommand {
public:
    ChangePromptCommand(const char *cmd_line): BuiltInCommand(cmd_line) {}
    virtual ~ChangePromptCommand() {}
    void execute() override;
};

class LetSeeCommand : public BuiltInCommand {
public:
    LetSeeCommand(const char* cmd_line): BuiltInCommand(cmd_line) {}
    virtual ~LetSeeCommand() {}
    void execute() override;
};

class ChangeDirCommand : public BuiltInCommand {
public:
    ChangeDirCommand(const char* cmd_line): BuiltInCommand(cmd_line) {};
    virtual ~ChangeDirCommand() {}
    void execute() override;
};

class GetCurrDirCommand : public BuiltInCommand {
public:
    GetCurrDirCommand(const char* cmd_line): BuiltInCommand(cmd_line) {}
    virtual ~GetCurrDirCommand() {}
    void execute() override;
};

class ShowPidCommand : public BuiltInCommand {
public:
    ShowPidCommand(const char* cmd_line): BuiltInCommand(cmd_line) {}
    virtual ~ShowPidCommand() {}
    void execute() override;
};

class JobsList;

class QuitCommand : public BuiltInCommand {
public:
    QuitCommand(const char* cmd_line): BuiltInCommand(cmd_line) {}
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
    int max_job_id;
    std::map<int,JobEntry> job_list;
    JobsList();
    ~JobsList() = default;
    void addJob(Command* cmd, bool isStopped = false);
    void printJobsList();
    void killAllJobs();
    void removeFinishedJobs();
    JobEntry * getJobById(int jobId);
    void removeJobById(int jobId);
    //JobEntry * getLastJob(int* lastJobId);
    //JobEntry *getLastStoppedJob(int *jobId);
};

class JobsCommand : public BuiltInCommand {
    // TODO: Add your data members
public:
    JobsCommand(const char* cmd_line): BuiltInCommand(cmd_line) {}
    virtual ~JobsCommand() {}
    void execute() override;
};

class KillCommand : public BuiltInCommand {
public:
    KillCommand(const char* cmd_line): BuiltInCommand(cmd_line) {}
    virtual ~KillCommand() {}
    void execute() override;
};

class ForegroundCommand : public BuiltInCommand {
public:
    ForegroundCommand(const char* cmd_line): BuiltInCommand(cmd_line) {}
    virtual ~ForegroundCommand() {}
    void execute() override;
};

class BackgroundCommand : public BuiltInCommand {
public:
    BackgroundCommand(const char* cmd_line): BuiltInCommand(cmd_line) {}
    virtual ~BackgroundCommand() {}
    void execute() override;
};

// TODO: add more classes if needed 
// maybe ls, timeout ?

class SmallShell {
private:
    std::string prompt;
    char last_dir[PATH_MAX];
    bool last_dir_init;
    JobsList jobs_list;
    Command* curr_cmd;
    SmallShell();

public:
    Command *CreateCommand(const char *cmd_line);
    SmallShell(SmallShell const &) = delete; // disable copy ctor
    void operator=(SmallShell const &) = delete; // disable = operator
    static SmallShell &getInstance() // make SmallShell singleton
    {
        static SmallShell instance; // Guaranteed to be destroyed.
        // Instantiated on first use.
        return instance;
    }
    ~SmallShell();
    void executeCommand(const char *cmd_line);
    void printPrompt() { std::cout << prompt; }
    char* lastDir() { return last_dir; }
    bool isLastDirSet() { return last_dir_init; }
    void lastDirIsSet() { last_dir_init = true; }
    void changePrompt(std::string s) { prompt = s; }
    void setCurrCmd(Command* cmd) { curr_cmd = cmd; }
    void unsetCurrCmd() { curr_cmd = nullptr; }
    Command* getCurrCmd() { return curr_cmd; }
    JobsList& getJobsList() { return jobs_list; }
};
#endif //SMASH_COMMAND_H_
