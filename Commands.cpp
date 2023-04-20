#include <unistd.h>
#include <string.h>
#include <iostream>
#include <vector>
#include <sstream>
#include <sys/wait.h>
#include <iomanip>
#include <climits>
#include "Commands.h"
#include <stack>
#include <time.h>
#include <algorithm>

using namespace std;

const std::string WHITESPACE = " \n\r\t\f\v";

#if 0
#define FUNC_ENTRY()  \
  cout << __PRETTY_FUNCTION__ << " --> " << endl;

#define FUNC_EXIT()  \
  cout << __PRETTY_FUNCTION__ << " <-- " << endl;
#else
#define FUNC_ENTRY()
#define FUNC_EXIT()
#endif

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

SmallShell::SmallShell() : previousPath("")  {
    char cwd[PATH_MAX];
    if (getcwd(cwd, sizeof(cwd)) != NULL) {
        currentPath = cwd;
    }else {
        perror("smash error: getcwd failed");
    }
// TODO: add your implementation
//printf(prompt);
}

SmallShell::~SmallShell() {
// TODO: add your implementation
}

/**
* Creates and returns a pointer to Command class which matches the given command line (cmd_line)
*/
Command * SmallShell::CreateCommand(const char* cmd_line) {
	// For example:


  string cmd_s = _trim(string(cmd_line));
  string firstWord = cmd_s.substr(0, cmd_s.find_first_of(" \n"));
  //std::cout << "hi";
  if (firstWord.compare("chprompt") == 0)
  {
      return new ChpromptCommand(cmd_line);
  }
  else if (firstWord.compare("showpid") == 0)
  {
      return new ShowPidCommand(cmd_line);
  }
  else if (firstWord.compare("pwd") == 0)
  {
      return new GetCurrDirCommand(cmd_line);
  }
  else if (firstWord.compare("cd") == 0)
  {
      return new ChangeDirCommand(cmd_line,nullptr);
  }
  else if (firstWord.compare("jobs") == 0)
  {
      return new JobsCommand(cmd_line,nullptr);
  }
  else if (firstWord.compare("fg") == 0)
  {
      return new ForegroundCommand(cmd_line,nullptr);
  }
  else if (firstWord.compare("bg") == 0)
  {
      return new BackgroundCommand(cmd_line,nullptr);
  }
  else if (firstWord.compare("quit") == 0)
  {
      return new QuitCommand(cmd_line,nullptr);
  }
  else if (firstWord.compare("kill") == 0)
  {
      return new KillCommand(cmd_line,nullptr);
  }
  else
  {
      return new ExternalCommand(cmd_line);
  }
/*
  if (firstWord.compare("pwd") == 0) {
    return new GetCurrDirCommand(cmd_line);
  }
  else if (firstWord.compare("showpid") == 0) {
    return new ShowPidCommand(cmd_line);
  }
  else if ...
  .....
  else {
    return new ExternalCommand(cmd_line);
  }
  */
  return nullptr;
}

void SmallShell::executeCommand(const char *cmd_line) {
    Command* cmd = CreateCommand(cmd_line);
    cmd->execute();
    //add support for & !!
  // TODO: Add your implementation here
  // for example:
  // Command* cmd = CreateCommand(cmd_line);
  // cmd->execute();
  // Please note that you must fork smash process for some commands (e.g., external commands....)
}

ChpromptCommand::ChpromptCommand(const char* cmd_line) : BuiltInCommand(cmd_line) {
// TODO: add your implementation
    //std::cout << args[1] << "\n";
    if (args[1] == 0)
    {
        newName = "smash";
    }
    else
    {
        newName = args[1];
    }
    //strcat(newName,"> ");
}

void ChpromptCommand::execute() {
    SmallShell::getInstance().prompt = newName;
    //SmallShell::prompt = newName;
}

BuiltInCommand::BuiltInCommand(const char *cmd_line) : Command(cmd_line, getpid()) {

}

Command::Command(const char *cmd_line, int pid) : cmd_line(cmd_line), original_cmd_line(cmd_line), pid(pid) {
    char* s = const_cast<char *>(cmd_line);
    isBackground = false;
    if (_isBackgroundComamnd(s))
    {
        _removeBackgroundSign(s);
        isBackground = true;
    }
    _parseCommandLine(cmd_line,args);
    /*
    string s = cmd_line;
    char str[s.length()];
    strcpy(str,s.c_str());
    char * pch;
    pch = strtok (str," ");
    for (int i = 0 ; i < 20; i++)
    {
        args[i] = 0;
    }
    int i = 0;
    while (pch != NULL)
    {
        args[i] = pch;
        i++;
        pch = strtok (NULL, " ");
    }
*/
}

Command::~Command() {
for (int i = 0 ; i < 20; i++)
    free(args[i]);
}

ShowPidCommand::ShowPidCommand(const char *cmd_line) : BuiltInCommand(cmd_line) {

}

void ShowPidCommand::execute() {
    std::cout << "smash pid is " << getpid() << "\n";
}

GetCurrDirCommand::GetCurrDirCommand(const char *cmd_line) : BuiltInCommand(cmd_line) {

}

void GetCurrDirCommand::execute() {
    char cwd[PATH_MAX];
    if (getcwd(cwd, sizeof(cwd)) != NULL) {
        printf("%s\n", cwd);
    } else {
        perror("smash error: getcwd failed");
    }
}


ChangeDirCommand::ChangeDirCommand(const char *cmd_line, char **plastPwd) : BuiltInCommand(cmd_line) {

}
//check for cding to folders with spaces!!!
void ChangeDirCommand::execute() {

    string newPath = args[1];
    std::string* previousPath = &SmallShell::getInstance().previousPath;
    bool pushed = false;
    if (args[2] != 0)
    {
        std::cerr << "smash error: cd: too many arguments\n";
        return;
    }
    if (strcmp(args[1],"-") == 0)
    {
        if (previousPath->empty())
        {
            std::cerr << "smash error: cd: OLDPWD not set\n";
            return;
        }
        std::string temp = *previousPath;
        *previousPath = SmallShell::getInstance().currentPath;
        SmallShell::getInstance().currentPath = temp;

    }
    else
    {
        std::string temp = SmallShell::getInstance().currentPath;
        SmallShell::getInstance().currentPath = newPath;
        *previousPath = temp;
    }

    if (chdir(SmallShell::getInstance().currentPath.c_str()) == 0) //Success
    {

    }
    else {
        perror("smash error: cd failed");
    }
}

JobsList::JobEntry::JobEntry(int jobId, int pid, JobStatus status, string command) : jobId(jobId), pid(pid), status(status), cmd(command)
{
    startTime = time(nullptr);
    //time(&startTime);
}

//can we init like that?
JobsList::JobsList() : jobsList(), topJobId(1) {

}

void JobsList::addJob(Command *cmd, bool isBackground) {
    //this isBackground isn't the one under cmd;
    JobStatus status = FOREGROUND;
    if (isBackground)
        status = BACKGROUND;
    removeFinishedJobs();

    JobEntry* job = new JobEntry(topJobId, cmd->pid, status, cmd->original_cmd_line);
    jobsList.push_back(job);
    topJobId++;
}

void JobsList::removeFinishedJobs() {
//    list<JobEntry*>::iterator itr = jobsList.end();
//    itr--;
//    list<JobEntry*>::iterator end = jobsList.begin();
//    //end--;

    for (list<JobEntry*>::iterator itr = jobsList.begin(); itr != jobsList.end(); itr++) {
        JobEntry* job = (*itr);
        int status;
        pid_t result = waitpid(job->pid, &status, WNOHANG);
        if (result == 0) {
            // Child still alive
        } else if (result == -1) {
            // Error
            // usually means finished
            itr = jobsList.erase(itr);
            itr--;
        } else {
            itr = jobsList.erase(itr);
            itr--;
            // Child exited
        }
    }

    if (jobsList.size() == 0)
        topJobId = 1;
    else
    {
        int num = 0;
        list<JobEntry*>::iterator itr = jobsList.end();
        itr--;
        topJobId = (*itr)->jobId+1;
    }
}

void JobsList::printJobsList() {
    removeFinishedJobs();
    for (list<JobEntry*>::iterator itr = jobsList.begin(); itr != jobsList.end(); itr++) {
        JobEntry* job = (*itr);
        double elapsed = difftime(time(nullptr),job->startTime);

        std::cout << "[" << job->jobId << "] " << job->cmd << " : " << job->pid << " " << elapsed << " secs";
        if (job->status == STOPPED)
            std::cout << " (stopped)";
        std::cout << "\n";
    }
}

void JobsList::killAllJobs()
{
    for (list<JobEntry*>::iterator itr = jobsList.begin(); itr != jobsList.end(); itr++) {
        JobEntry* job = (*itr);
        std::cout << job->pid << ": " << job->cmd << "\n";
        if (kill(job->pid, SIGKILL) == -1)
            perror("smash error: kill failed");

    }
}

JobsCommand::JobsCommand(const char *cmd_line, JobsList *jobs) : BuiltInCommand(cmd_line) {

}

void JobsCommand::execute() {
    SmallShell::getInstance().jobsList.printJobsList();
}

ExternalCommand::ExternalCommand(const char *cmd_line) : Command(cmd_line,0) {

}

void ExternalCommand::execute() {

    bool isComplex = false;
    for (int i = 0; i < original_cmd_line.length(); i++)
    {
        if (original_cmd_line[i] == '*' || original_cmd_line[i] == '?')
        {
            isComplex = true;
        }
    }


    int p = fork();
    if (p == -1)
        perror("smash error: fork failed");
    if (p == 0) {
        //child
        setpgrp();

        if (isComplex)
        {
            string s = "bash";
            string c = "-c";
            char* arr[] = {const_cast<char *>(s.c_str()), const_cast<char *>(c.c_str()), const_cast<char *>(cmd_line), NULL};
            execvp("bash", arr);
        }
        else
            execvp(args[0],args);
    } else {

        this->pid = p; //important?
        if (isBackground)
        {
            SmallShell::getInstance().jobsList.addJob(this,true);
        }
        else
        {
            SmallShell::getInstance().jobsList.addJob(this, false);
            waitpid(p, NULL, 0); //need to check no &
        }
    }
}

QuitCommand::QuitCommand(const char *cmd_line, JobsList *jobs) : BuiltInCommand(cmd_line) {

}

void QuitCommand::execute() {

    if (args[1] && strcmp(args[1],"kill") == 0)
    {
        JobsList* jobsList = &SmallShell::getInstance().jobsList;
        jobsList->removeFinishedJobs();
        std::cout << "smash: sending SIGKILL signal to " << jobsList->jobsList.size() << " jobs:\n";
        jobsList->killAllJobs();
    }
    exit(0);


}

ForegroundCommand::ForegroundCommand(const char *cmd_line, JobsList *jobs) : BuiltInCommand(cmd_line) {

}

void ForegroundCommand::execute() {
    if (args[2]) {
        std::cerr << "smash error: fg: invalid arguments\n";
        return;
    }
    SmallShell::getInstance().jobsList.removeFinishedJobs();
    if (args[1])
    {
        std::string s = args[1];
        if (std::all_of(s.begin(), s.end(), ::isdigit))
        {
            int id = stoi(s);
            bool done = false;
            JobsList* jobsList = &SmallShell::getInstance().jobsList;
            for (list<JobsList::JobEntry*>::iterator itr = jobsList->jobsList.begin(); itr != jobsList->jobsList.end(); itr++)
            {
                if ((*itr)->jobId == id)
                {
                    done = true;
                    (*itr)->status = FOREGROUND;
                    std::cout << (*itr)->cmd << " : " << (*itr)->pid<< "\n";
                    if (kill((*itr)->pid,SIGCONT) == -1)
                        perror("smash error: kill failed");
                    waitpid((*itr)->pid,nullptr,0);
                }
            }
            if (!done)
            {
                std::cerr << "smash error: fg: job-id " << id << " does not exist\n";
            }
        }
        else
        {
            std::cerr << "smash error: fg: invalid arguments\n";
            return;
        }
    }
    else
    {
        if (SmallShell::getInstance().jobsList.jobsList.empty()) {
            std::cerr << "smash error: fg: jobs list is empty\n";
            return;
        }

        JobsList* jobsList = &SmallShell::getInstance().jobsList;
        list<JobsList::JobEntry*>::iterator itr = jobsList->jobsList.end();
        itr--;

        (*itr)->status = FOREGROUND;
        std::cout << (*itr)->cmd << " : " << (*itr)->pid << "\n";
        if (kill((*itr)->pid,SIGCONT) == -1)
            perror("smash error: kill failed");
        waitpid((*itr)->pid,nullptr,0);
    }
}

BackgroundCommand::BackgroundCommand(const char *cmd_line, JobsList *jobs) : BuiltInCommand(cmd_line) {

}

///TODO check if working with ctrl-z/c whatever

void BackgroundCommand::execute() {

    if (args[2]) {
        std::cerr << "smash error: bg: invalid arguments\n";
        return;
    }
    SmallShell::getInstance().jobsList.removeFinishedJobs();
    if (args[1])
    {
        std::string s = args[1];
        if (std::all_of(s.begin(), s.end(), ::isdigit))
        {
            int id = stoi(s);
            bool done = false;
            JobsList* jobsList = &SmallShell::getInstance().jobsList;
            for (list<JobsList::JobEntry*>::iterator itr = jobsList->jobsList.begin(); itr != jobsList->jobsList.end(); itr++)
            {
                if ((*itr)->jobId == id)
                {
                    if ((*itr)->status != STOPPED)
                    {
                        std::cerr << "smash error: bg: job-id " << id << " is already running in the background\n";
                        return;
                    }
                    done = true;
                    (*itr)->status = BACKGROUND; //was STOPPED
                    std::cout << (*itr)->cmd << " : " << (*itr)->pid<< "\n";
                    if (kill((*itr)->pid,SIGCONT) == -1)
                        perror("smash error: kill failed");
                }
            }
            if (!done)
            {
                std::cerr << "smash error: bg: job-id " << id << " does not exist\n";
                return;
            }
        }
        else
        {
            std::cerr << "smash error: bg: invalid arguments\n";
            return;
        }
    }
    else
    {
        JobsList* jobsList = &SmallShell::getInstance().jobsList;
        int max = -1;

        for (list<JobsList::JobEntry*>::iterator itr = jobsList->jobsList.begin(); itr != jobsList->jobsList.end(); itr++)
                if ((*itr)->status == STOPPED)
                    max = (*itr)->jobId;

        if (max == -1)
        {
            std::cerr << "smash error: there is no jobs to resume\n"; //there ARE no jobs to resume
            return;
        }

        for (list<JobsList::JobEntry*>::iterator itr = jobsList->jobsList.begin(); itr != jobsList->jobsList.end(); itr++)
            if ((*itr)->jobId == max)
            {
                (*itr)->status = BACKGROUND;
                std::cout << (*itr)->cmd << " : " << (*itr)->pid<< "\n";
                if (kill((*itr)->pid,SIGCONT) == -1)
                    perror("smash error: kill failed");
            }
    }
}

KillCommand::KillCommand(const char *cmd_line, JobsList *jobs) : BuiltInCommand(cmd_line) {

}

void KillCommand::execute() {
    if(args[3]) {
        std::cerr << "smash error: kill: invalid arguments\n";
        return;
    }

    SmallShell::getInstance().jobsList.removeFinishedJobs();
    if (args[2])
    {
        std::string signum_s = args[1];
        if(signum_s[0] != '-')
        {
            std::cerr << "smash error: kill: invalid arguments\n";
            return;
        }
        signum_s.erase(0,1); // remove the '-' from the string

        std::string id_s = args[2];
        if (std::all_of(signum_s.begin(), signum_s.end(), ::isdigit) && std::all_of(id_s.begin(), id_s.end(), ::isdigit))
        {
            int id = stoi(id_s);
            int signum = stoi(signum_s);
            bool done = false;
            JobsList* jobsList = &SmallShell::getInstance().jobsList;
            for (list<JobsList::JobEntry*>::iterator itr = jobsList->jobsList.begin(); itr != jobsList->jobsList.end(); itr++)
            {
                if ((*itr)->jobId == id)
                {
                    done = true;
                    std::cout << "signal number " << signum << " was sent to pid " << (*itr)->pid << "\n";
                    if (kill((*itr)->pid,signum) == -1)
                        perror("smash error: kill failed");
                }
            }
            if (!done)
            {
                std::cerr << "smash error: kill: job-id " << id << " does not exist\n";
                return;
            }
        }
        else
        {
            std::cerr << "smash error: bg: invalid arguments\n";
            return;
        }
    }
    else
    {
        std::cerr << "smash error: kill: invalid arguments\n";
    }
}
