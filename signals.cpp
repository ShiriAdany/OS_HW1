#include <iostream>
#include <signal.h>
#include <sys/wait.h>
#include "signals.h"
#include "Commands.h"

using namespace std;

void ctrlZHandler(int sig_num) {
	// TODO: Add your implementation
    JobsList* jobsList = &SmallShell::getInstance().jobsList;
    for (list<JobsList::JobEntry*>::iterator itr = jobsList->jobsList.begin(); itr != jobsList->jobsList.end(); itr++)
    {
        if ((*itr)->status == FOREGROUND)
        {
            std::cout << "smash: got ctrl-Z\n";
            (*itr)->status = STOPPED;

            if (kill((*itr)->pid, sig_num) == -1)
                perror("smash error: kill failed");
            std::cout << "smash: process " << ((*itr)->pid) << " was stopped\n";
        }
    }
}

void ctrlCHandler(int sig_num) {
  // TODO: Add your implementation
    std::cout << "smash: got ctrl-C\n";
    JobsList* jobsList = &SmallShell::getInstance().jobsList;
    for (list<JobsList::JobEntry*>::iterator itr = jobsList->jobsList.begin(); itr != jobsList->jobsList.end(); itr++)
    {
        if ((*itr)->status == FOREGROUND)
        {
            if (kill((*itr)->pid, sig_num) == -1)
                perror("smash error: kill failed");
            std::cout << "smash: process " << ((*itr)->pid) << " was killed\n";
        }
    }
    // If there is no process running in the foreground, then the smash should ignore them.
}

void alarmHandler(int sig_num) {
  // TODO: Add your implementation
    std::cout << "smash: got an alarm\n"; //should be printed anyway ??
    list<TimedProcess*>* timedProcesses = &(SmallShell::getInstance().timedProcesses);

    for (list<TimedProcess*>::iterator itr = timedProcesses->begin(); itr != timedProcesses->end(); itr++)
    {
        if ((*itr)->duration == difftime(time(nullptr), (*itr)->startTime)) //maybe need to use >= and check for the minimum
        {
            //std::cout << "exists \n";
            if (waitpid((*itr)->pid, nullptr, WNOHANG) == 0) //still alive
            {
                std::cout << "smash: " << (*itr)->cmd_line << " timed out!\n";
                if (kill((*itr)->pid, SIGKILL) == -1)
                    perror("smash error: kill failed");
                itr = timedProcesses->erase(itr);
                itr--;
            }
            //else
            //    std::cout << "died already \n";
        }
    }
}

