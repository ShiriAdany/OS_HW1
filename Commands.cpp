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

SmallShell::SmallShell() {
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

BuiltInCommand::BuiltInCommand(const char *cmd_line) : Command(cmd_line) {

}

Command::Command(const char *cmd_line) {

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
    std::stack<string>* cdHistory = &SmallShell::getInstance().cdHistory;
    bool pushed = false;
    if (args[2] != 0)
    {
        std::cerr << "smash error: cd: too many arguments\n";
        return;
    }
    if (strcmp(args[1],"-") == 0)
    {
        if (cdHistory->empty())
        {
            std::cerr << "smash error: cd: OLDPWD not set\n";
            return;
        }
        newPath = cdHistory->top();
        cdHistory->pop();
    }
    else
    { //should this be here??
        char cwd[PATH_MAX];
        if (getcwd(cwd, sizeof(cwd)) != NULL) {
            std::cout << cwd << "\n\n";
            cdHistory->push(cwd);
            pushed = true;
        } else {
            perror("smash error: getcwd failed");
        }
    }
    if (chdir(newPath.c_str()) == 0) //Success
    {

    }
    else {
        perror("smash error: cd failed");
        if (pushed)
            cdHistory->pop();
    }
}
