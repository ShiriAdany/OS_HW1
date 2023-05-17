#include <iostream>
#include <signal.h>
#include <sys/wait.h>
#include "signals.h"
#include "Commands.h"
#include <unistd.h>
#include <cmath>
#include <sys/time.h>



using namespace std;

void ctrlZHandler(int sig_num) {
    std::cout << "smash: got ctrl-Z\n";
    JobsList* jobsList = &SmallShell::getInstance().jobsList;
    for (list<JobsList::JobEntry*>::iterator itr = jobsList->jobsList.begin(); itr != jobsList->jobsList.end(); itr++)
    {
        if ((*itr)->status == FOREGROUND)
        {
            JobsList::JobEntry* job = (*itr);
            int status;
            pid_t result = waitpid(job->pid, &status, WNOHANG);
            if (result == 0) {
                // Child still alive
                (*itr)->status = STOPPED;
                (*itr)->startTime = time(nullptr);

                if (kill((*itr)->pid, sig_num) == -1)
                    perror("smash error: kill failed");
                std::cout << "smash: process " << ((*itr)->pid) << " was stopped\n";
            } else if (result == -1) {
                // Error
                // usually means finished
            } else {
                // Child exited
            }
        }
    }
}

void ctrlCHandler(int sig_num) {
    std::cout << "smash: got ctrl-C\n";
    JobsList* jobsList = &SmallShell::getInstance().jobsList;
    for (list<JobsList::JobEntry*>::iterator itr = jobsList->jobsList.begin(); itr != jobsList->jobsList.end(); itr++)
    {
        if ((*itr)->status == FOREGROUND)
        {
            JobsList::JobEntry* job = (*itr);
            int status;
            pid_t result = waitpid(job->pid, &status, WNOHANG);
            if (result == 0) {
                // Child still alive
                if (kill((*itr)->pid, sig_num) == -1)
                    perror("smash error: kill failed");
                std::cout << "smash: process " << ((*itr)->pid) << " was killed\n";
            } else if (result == -1) {
                // Error
                // usually means finished
            } else {
                // Child exited
            }
        }
    }
    // If there is no process running in the foreground, then the smash should ignore them.
}

void alarmHandler(int sig_num) {
    list<TimedProcess *> &timedProcesses = SmallShell::getInstance().timedProcesses;
    double min_time_remaining = INT32_MAX;
    std::cout << "smash: got an alarm" << endl;
    for (list<TimedProcess*>::iterator itr = timedProcesses.begin(); itr != timedProcesses.end();) {
        double time_remaining = (double)((*itr)->duration - difftime(time(nullptr), (*itr)->startTime));
        int status;
        if (time_remaining <= 0) {
            int result = waitpid((*itr)->pid, &status, WNOHANG);
            if (result == 0) {    //is still alive

                if (kill((*itr)->pid, SIGKILL) == -1)
                    perror("smash error: kill failed");

                std::cout << "smash: " << (*itr)->cmd_line << " timed out!" << endl;
            }

            itr = timedProcesses.erase(itr);
            itr--;
        }
        if (time_remaining < min_time_remaining && time_remaining > 0)
            min_time_remaining = time_remaining;

        ++itr;

    }
    min_time_remaining = timedProcesses.empty() ? 0 : min_time_remaining;
    if (min_time_remaining != INT32_MAX && min_time_remaining > 0) {
        alarm(min_time_remaining);
    }

    cout << flush;
}
