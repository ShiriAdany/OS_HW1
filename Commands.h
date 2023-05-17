#ifndef SMASH_COMMAND_H_
#define SMASH_COMMAND_H_

#include <vector>
#include <stack>
#include <list>

#define COMMAND_ARGS_MAX_LENGTH (200)
#define COMMAND_MAX_ARGS (20)

class Command {
protected:
    char* args[20];
public:
    std::string cmd_line;
    std::string original_cmd_line;
    int pid;
    bool isBackground;
    bool isExternal;
    bool isTimed;
    int duration;
    Command(const char* cmd_line, int pid);
    virtual ~Command();
    virtual void execute() = 0;
};

class BuiltInCommand : public Command {
public:
    BuiltInCommand(const char* cmd_line);
    virtual ~BuiltInCommand() {}
};

class ExternalCommand : public Command {
public:
    ExternalCommand(const char* cmd_line);
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
public:
    explicit RedirectionCommand(const char* cmd_line);
    virtual ~RedirectionCommand() {}
    void execute() override;
    //void prepare() override;
    //void cleanup() override;
};

class ChangeDirCommand : public BuiltInCommand {
public:
    ChangeDirCommand(const char* cmd_line, char** plastPwd);
    virtual ~ChangeDirCommand() {}
    void execute() override;
};

class GetCurrDirCommand : public BuiltInCommand {
public:
    GetCurrDirCommand(const char* cmd_line);
    virtual ~GetCurrDirCommand() {}
    void execute() override;
};

class ChpromptCommand : public BuiltInCommand {
private:
    std::string newName;
public:
    ChpromptCommand(const char* cmd_line);
    virtual ~ChpromptCommand() = default;
    void execute() override;
};

class ShowPidCommand : public BuiltInCommand {
public:
    ShowPidCommand(const char* cmd_line);
    virtual ~ShowPidCommand() {}
    void execute() override;
};

class JobsList;
class QuitCommand : public BuiltInCommand {
public:
    QuitCommand(const char* cmd_line, JobsList* jobs);
    virtual ~QuitCommand() {}
    void execute() override;
};

enum JobStatus {FOREGROUND , BACKGROUND, STOPPED} ;
class JobsList {
public:
    class JobEntry {
    public:
        time_t startTime;
        int jobId;
        int pid;
        JobStatus status;
        std::string cmd;
        JobEntry(int jobId, int pid, JobStatus status, std::string command);
    };
    std::list<JobEntry*> jobsList;
    int topJobId;
public:
    JobsList();
    ~JobsList() = default;
    void addJob(Command* cmd, bool isBackground = false);
    void printJobsList();
    void killAllJobs();
    void removeFinishedJobs();
    JobEntry * getJobById(int jobId);
    void removeJobById(int jobId);
    JobEntry * getLastJob();
    JobEntry *getLastStoppedJob(int *jobId);
};

class JobsCommand : public BuiltInCommand {
public:
    JobsCommand(const char* cmd_line, JobsList* jobs);
    virtual ~JobsCommand() {}
    void execute() override;
};

class ForegroundCommand : public BuiltInCommand {
public:
    ForegroundCommand(const char* cmd_line, JobsList* jobs);
    virtual ~ForegroundCommand() {}
    void execute() override;
};

class BackgroundCommand : public BuiltInCommand {
public:
    BackgroundCommand(const char* cmd_line, JobsList* jobs);
    virtual ~BackgroundCommand() {}
    void execute() override;
};


class GetFileTypeCommand : public BuiltInCommand {
public:
    GetFileTypeCommand(const char* cmd_line);
    virtual ~GetFileTypeCommand() {}
    void execute() override;
    int getSize(std::string file);
};

class ChmodCommand : public BuiltInCommand {
public:
    ChmodCommand(const char* cmd_line);
    virtual ~ChmodCommand() {}
    void execute() override;
};

class SetcoreCommand : public BuiltInCommand {
public:
    SetcoreCommand(const char* cmd_line);
    virtual ~SetcoreCommand() {}
    void execute() override;
};

class KillCommand : public BuiltInCommand {
public:
    KillCommand(const char* cmd_line, JobsList* jobs);
    virtual ~KillCommand() {}
    void execute() override;
};


class TimedProcess
{
public:
    int pid;
    int duration;
    time_t startTime;
    std::string cmd_line;
};

class TimeoutCommand : public BuiltInCommand {
public:
    explicit TimeoutCommand(const char* cmd_line);
    virtual ~TimeoutCommand() {};
    void execute() override;
};

class SmallShell {
private:
    SmallShell();
public:
    JobsList jobsList;
    //std::stack<std::string> cdHistory;
    std::string previousPath;
    std::string currentPath;
    std::string prompt = "smash";
    int pid;

    std::list<TimedProcess*> timedProcesses;


    Command *CreateCommand(const char* cmd_line);
    SmallShell(SmallShell const&)      = delete; // disable copy ctor
    void operator=(SmallShell const&)  = delete; // disable = operator
    static SmallShell& getInstance() // make SmallShell singleton
    {
        static SmallShell instance; // Guaranteed to be destroyed.
        // Instantiated on first use.
        return instance;
    }
    ~SmallShell();
    void executeCommand(const char* cmd_line, bool isTimed = false, int duration = 0, std::string original_cmd_line = "");
};

#endif //SMASH_COMMAND_H_