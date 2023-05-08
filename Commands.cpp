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
#include <fcntl.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <dirent.h>



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
    pid = getpid(); //todo: add if fails?


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
  string cmd_s = _trim(string(cmd_line));
  string firstWord = cmd_s.substr(0, cmd_s.find_first_of(" \n"));

  for (int i = 0; i < cmd_s.length(); i++)
  {
      if (cmd_s[i] == '>')
      {
          //>
          //>>
          return new RedirectionCommand(cmd_line);
      }
      if (cmd_s[i] == '|')
      {
          // |
          // |&
          return new PipeCommand(cmd_line);
      }
  }
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
  else if (firstWord.compare("setcore") == 0)
  {
      return new SetcoreCommand(cmd_line);
  }
  else if (firstWord.compare("getfiletype") == 0)
  {
      return new GetFileTypeCommand(cmd_line);
  }
  else if (firstWord.compare("chmod") == 0)
  {
      return new ChmodCommand(cmd_line);
  }
  else if (firstWord.compare("timeout") == 0)
  {
      return new TimeoutCommand(cmd_line);
  }
  else
  {
      return new ExternalCommand(cmd_line);
  }
}

void SmallShell::executeCommand(const char *cmd_line, bool isTimed, int duration) {

    Command* cmd = CreateCommand(cmd_line);
    cmd->isTimed = isTimed;
    cmd->duration = duration;
    cmd->execute();
    //add support for & !!
  // TODO: Add your implementation here
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

Command::Command(const char *cmd_line, int pid) : cmd_line(cmd_line), original_cmd_line(cmd_line), pid(pid), isExternal(false) {
    char* s = const_cast<char *>(cmd_line);
    isBackground = false;
    //isTimed = false;
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
    std::cout << "smash pid is " << SmallShell::getInstance().pid << "\n";
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

    if (!args[1])
        return;
    string newPath = args[1];
    std::string* previousPath = &SmallShell::getInstance().previousPath;
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

        if (chdir(previousPath->c_str()) == 0) //Success
        {
            std::string temp = *previousPath;
            *previousPath = SmallShell::getInstance().currentPath;
            SmallShell::getInstance().currentPath = temp;
        }
        else {
            perror("smash error: chdir failed");
        }
    }
    else
    {
        if (newPath[0] != '/')
            newPath = SmallShell::getInstance().currentPath + "/" + newPath;
        if (chdir(newPath.c_str()) == 0) //Success
        {
            std::string temp = SmallShell::getInstance().currentPath;
            SmallShell::getInstance().currentPath = newPath;
            *previousPath = temp;
        }
        else {
            perror("smash error: chdir failed");
        }

    }

//    if (chdir(SmallShell::getInstance().currentPath.c_str()) == 0) //Success
//    {
//
//    }
//    else {
//        perror("smash error: cd failed");
//    }
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

        list<TimedProcess*>* timedProcesses = &(SmallShell::getInstance().timedProcesses);
        std::cout << "[" << job->jobId << "] ";
        for (list<TimedProcess*>::iterator itr = timedProcesses->begin(); itr != timedProcesses->end(); itr++)
        {
            if((*itr)->pid == job->pid)
            {
                std::cout << "timeout " << (*itr)->duration << " ";
            }
        }

             std::cout << job->cmd << " : " << job->pid << " " << elapsed << " secs";
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

JobsList::JobEntry *JobsList::getLastJob() {
    list<JobsList::JobEntry*>::iterator itr = jobsList.end();
    itr--;
    return *itr;
}

JobsCommand::JobsCommand(const char *cmd_line, JobsList *jobs) : BuiltInCommand(cmd_line) {

}

void JobsCommand::execute() {
    SmallShell::getInstance().jobsList.printJobsList();
}

ExternalCommand::ExternalCommand(const char *cmd_line) : Command(cmd_line,0) {
    isExternal = true;
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
            char* arr[] = {const_cast<char *>(s.c_str()), const_cast<char *>(c.c_str()), const_cast<char *>(cmd_line.c_str()), NULL};
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
        }
        if (isTimed)
        {
            TimedProcess* timedProcess = new TimedProcess();
            //timedProcess->cmd_line = SmallShell::getInstance().jobsList.getLastJob()->cmd;
            timedProcess->cmd_line = original_cmd_line;
            timedProcess->startTime = SmallShell::getInstance().jobsList.getLastJob()->startTime;
            timedProcess->duration = duration;
            timedProcess->pid = SmallShell::getInstance().jobsList.getLastJob()->pid;
            SmallShell::getInstance().timedProcesses.push_back(timedProcess);

            //std::cout << cmd_line << "\n\n\n";
        }
        if (!isBackground)
            waitpid(p, NULL, WUNTRACED); //need to check no &
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
        if (std::all_of(s.begin(), s.end(), ::isdigit) || (s[0] == '-' && std::all_of(s.begin()+1, s.end(), ::isdigit)))
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
                    //(*itr)->startTime = time(nullptr); //TODO make sure it is supposed to be started over
                    std::cout << (*itr)->cmd << " : " << (*itr)->pid<< "\n";
                    if (kill((*itr)->pid,SIGCONT) == -1)
                        perror("smash error: kill failed");
                    waitpid((*itr)->pid,nullptr,WUNTRACED);
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
        waitpid((*itr)->pid,nullptr,WUNTRACED);
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
        if (std::all_of(s.begin(), s.end(), ::isdigit) || (s[0] == '-' && std::all_of(s.begin() + 1, s.end(), ::isdigit)))
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
                    //(*itr)->startTime = time(nullptr); //TODO make sure it is supposed to be started over
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
            std::cerr << "smash error: bg: there is no stopped jobs to resume\n"; //there ARE no jobs to resume
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
        if (std::all_of(signum_s.begin(), signum_s.end(), ::isdigit) &&
            (std::all_of(id_s.begin(), id_s.end(), ::isdigit) ||
            (id_s[0] == '-' && std::all_of(id_s.begin()+1, id_s.end(), ::isdigit))) )
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

                    if (kill((*itr)->pid,signum) == -1)
                        perror("smash error: kill failed");
                    else
                        std::cout << "signal number " << signum << " was sent to pid " << (*itr)->pid << "\n";
                    if (signum == 20)
                        (*itr)->status = STOPPED;
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
            std::cerr << "smash error: kill: invalid arguments\n";
            return;
        }
    }
    else
    {
        std::cerr << "smash error: kill: invalid arguments\n";
    }
}


SetcoreCommand::SetcoreCommand(const char *cmd_line) : BuiltInCommand(cmd_line) {

}

void SetcoreCommand::execute() {
    if(args[3])
    {
        std::cerr << "smash error: setcore: invalid arguments\n";
        return;
    }

    std::string corenum_s = args[2];
    std::string id_s = args[1];
    if (std::all_of(corenum_s.begin(), corenum_s.end(), ::isdigit) && std::all_of(id_s.begin(), id_s.end(), ::isdigit))
    {
        int id = stoi(id_s);
        int corenum = stoi(corenum_s);
        int num_of_processes = sysconf(_SC_NPROCESSORS_ONLN);
        if(corenum >= num_of_processes)
        {
            std::cerr << "smash error: setcore: invalid core number\n";
            return;
        }
        else if(num_of_processes == -1)
        {
            perror("smash error: sysconf failed");
        }
        bool done = false;
        JobsList* jobsList = &SmallShell::getInstance().jobsList;
        for (list<JobsList::JobEntry*>::iterator itr = jobsList->jobsList.begin(); itr != jobsList->jobsList.end(); itr++)
        {
            if ((*itr)->jobId == id)
            {
                cpu_set_t my_set;        /* Define your cpu_set bit mask. */
                CPU_ZERO(&my_set);       /* Initialize it all to 0, i.e. no CPUs selected. */
                CPU_SET(corenum, &my_set);
                done = true;

                if (sched_setaffinity((*itr)->pid,sizeof(cpu_set_t), &my_set) == -1) {
                    perror("smash error: sched_setaffinity failed");
                }
            }
        }
        if (!done)
        {
            std::cerr << "smash error: setcore: job-id "  << id << " does not exist\n";
            return;
        }
    }
    else{
        std::cerr << "smash error: setcore: invalid arguments\n";
        return;
    }

}

RedirectionCommand::RedirectionCommand(const char *cmd_line) : Command(cmd_line,0) {

}

void RedirectionCommand::execute() {
    //pwd > .txt
    //note that 0 is a pid ???
    std::string s = cmd_line;
    std::string start = _trim(s.substr(0,s.find('>')));
    std::string end;
    int stdout_fd = dup(1);

    if (s[s.find('>')] == s[s.find('>') + 1]) //>>
    {
        int from = s.find('>') + 2;
        int length = s.length() - from;
        end = _trim(s.substr(from,length));
        close(1);
        open(end.c_str(), O_WRONLY | O_CREAT | O_APPEND, 0666);
    }
    else //>
    {
        int from = s.find('>') + 1;
        int length = s.length() - from;
        end = _trim(s.substr(from,length));
        close(1);
        open(end.c_str(), O_WRONLY | O_CREAT, 0666);
    }
    SmallShell::getInstance().executeCommand(start.c_str());
    dup2(stdout_fd, 1);
    close(stdout_fd);
}

PipeCommand::PipeCommand(const char *cmd_line) : Command(cmd_line,0) {

}

void PipeCommand::execute() {
    std::string s = cmd_line;
    std::string start = _trim(s.substr(0,s.find('|')));
    std::string end;

    int p;
    int fd[2];
    //[ read | write ]
    if (pipe(fd) == -1)
    {
        perror("smash error: pipe failed");
    }
    //SmallShell& smash = SmallShell::getInstance();
    //bool isFirstExternal = smash.CreateCommand(start.c_str())->isExternal;
    //bool isSecondExternal = smash.CreateCommand(end.c_str())->isExternal;

    // in | - should we close stderr? no right?
    // in |& - should we close stdout? no?
    if (s[s.find('|') + 1] == '&') // |&
    {
        int from = s.find('|') + 2;
        int length = s.length() - from;
        end = _trim(s.substr(from,length));

        p=fork();
        if (p == 0) {
            // first child
            setpgrp();
            dup2(fd[1],2); //its err -> written to second child
            close(fd[0]);
            close(fd[1]);
            SmallShell::getInstance().executeCommand(start.c_str());
            exit(0);
        }
        else if(p == -1) {
            perror("smash error: fork failed");
        }

        p = fork();
        if (p == 0) {
            // second child
            setpgrp();
            dup2(fd[0],0); //its in -> received from first child
            close(fd[0]);
            close(fd[1]);
            SmallShell::getInstance().executeCommand(end.c_str());
            exit(0);
        }
        else if(p == -1) {
            perror("smash error: fork failed");
        }

    }
    else // |
    {
        int from = s.find('|') + 1;
        int length = s.length() - from;
        end = _trim(s.substr(from,length));

        p = fork();
        if (p == 0) {
            // first child
            // ls | more
            setpgrp();
            dup2(fd[1], 1); //its out -> written to second child
            close(fd[0]);
            close(fd[1]);
            SmallShell::getInstance().executeCommand(start.c_str());
            exit(0);
        } else if (p == -1) {
            perror("smash error: fork failed");
        }

        p = fork();
        if (p == 0) {
            // second child
            setpgrp();
            dup2(fd[0], 0); //its in -> recieved from first child
            close(fd[0]);
            close(fd[1]);
            SmallShell::getInstance().executeCommand(end.c_str());
            exit(0);
        } else if (p == -1) {
            perror("smash error: fork failed");
        }

    }
    close(fd[0]);
    close(fd[1]);
    wait(nullptr);
    wait(nullptr);
}

GetFileTypeCommand::GetFileTypeCommand(const char *cmd_line) : BuiltInCommand(cmd_line) {

}

void GetFileTypeCommand::execute() {
    struct stat statbuff;
    if (args[2])
    {
        std::cerr << "smash error: gettype: invalid arguments\n";
        return;
    }
    if (stat(args[1], &statbuff) != 0) ///syscall ? what is it? what if no such file? << need to print with perror i think
        return;
    std::cout << args[1] << "'s type is “";
    switch (statbuff.st_mode & S_IFMT) {
        case S_IFBLK:  std::cout <<"block device" ;           break;
        case S_IFCHR:  std::cout <<"character device" ;       break;
        case S_IFDIR:  std::cout <<"directory";               break;
        case S_IFIFO:  std::cout <<"FIFO";                    break;
        case S_IFLNK:  std::cout <<"symbolic link";           break;
        case S_IFREG:  std::cout <<"regular file";            break;
        case S_IFSOCK: std::cout <<"socket";                  break;
    }
    std::cout << "“ and takes up " << getSize(args[1])  << " bytes\n";
}

int GetFileTypeCommand::getSize(std::string file) {
    struct stat statbuff;
    //std::cout <<  "\n" << file << "  " << file.find('/')<< "\n";
    stat(file.c_str(), &statbuff);
    if (!S_ISDIR(statbuff.st_mode))
        return statbuff.st_size;
    int size = 0; //should we init this to 512 ? nah no way right?
    DIR *dr;
    struct dirent *en;
    dr = opendir(file.c_str()); //open all directory
    if (dr) {
        while ((en = readdir(dr)) != NULL) {
            if (strcmp(en->d_name, ".") == 0   || strcmp(en->d_name,"..") == 0) continue;
            //    std::cout << file + "/" + en->d_name << " frigging d name is " << en->d_name << "\n";
            size += getSize(file + "/" + en->d_name);
        }
        closedir(dr); //close all directory
    }
    return size;
}

ChmodCommand::ChmodCommand(const char *cmd_line) : BuiltInCommand(cmd_line) {

}

void ChmodCommand::execute() {
    if (args[3]) {
        std::cerr << "smash error: chmod: invalid arguments\n";
        return;
    }

    std::string path = args[2];
    std::string new_mode_s = args[1];
    if (std::all_of(new_mode_s.begin(), new_mode_s.end(), ::isdigit)) {
        mode_t new_mode = stoi(new_mode_s,0,8);
        if(chmod(path.c_str(), new_mode) == -1)
        {
            perror("smash error: chmod failed");
        }
    }
    else{
        std::cerr << "smash error: chmod: invalid arguments\n";
        return;
    }
}


TimeoutCommand::TimeoutCommand(const char *cmd_line) : BuiltInCommand(cmd_line) {

}

void TimeoutCommand::execute() {
    int i = 2;
    if (!args[1] || !args[2])
    {
        std::cerr << "smash error: timeout: invalid arguments\n";
        return;
    }
    char *cmd_without_timeout [20];
    _parseCommandLine(original_cmd_line.c_str(),cmd_without_timeout);

    string cmd = "";
    while (cmd_without_timeout[i])
    {
        cmd += cmd_without_timeout[i];
        cmd += " ";
        i++;
    }
//    if (isBackground)
//        cmd += "&";

//    if (!args[1] || !args[2])
//    {
//        std::cerr << "smash error: timeout: invalid arguments\n";
//        return;
//    }
    string s= args[1];
    if (!std::all_of(s.begin(), s.end(), ::isdigit)) {
        std::cerr << "smash error: timeout: invalid arguments\n";
        return;
    }
    int duration = stoi(args[1]); //todo check valid, check not 0, was told it cant be 0

    alarm(duration);

    //isTimed = true;

    SmallShell::getInstance().executeCommand(cmd.c_str(), true, duration);
    //SmallShell::getInstance().executeCommand(cmd_line, true, duration, cmd.c_str());

    //delete timedProcess;
    //todo delete on the other new too
}
