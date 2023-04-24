#include <iostream>
#include <signal.h>
#include "signals.h"
#include "Commands.h"

using namespace std;

void ctrlZHandler(int sig_num) {
	// TODO: Add your implementation
//    JobsList* jobsList = &SmallShell::getInstance().jobsList;
//    for (list<JobsList::JobEntry*>::iterator itr = jobsList->jobsList.begin(); itr != jobsList->jobsList.end(); itr++)
//    {
//        if ((*itr)->status == FOREGROUND)
//        {
//            std::cout << "smash: got ctrl-Z\n";
//            //(*itr)->status = STOPPED;
//            if (kill((*itr)->pid, sig_num) == -1)
//                perror("smash error: kill failed");
//            JobsList* jobsList = &SmallShell::getInstance().jobsList;
//            jobsList->addJob();
//            std::cout << "smash: process " << ((*itr)->pid) << " was stopped\n";
//
//        }
//    }
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
}

