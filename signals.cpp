#include <iostream>
#include <signal.h>
#include <sys/wait.h>
#include "signals.h"
#include "Commands.h"
#include <unistd.h>


using namespace std;

void ctrlZHandler(int sig_num) {
    // TODO: Add your implementation
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
                (*itr)->startTime = time(nullptr); //TODO make sure it is supposed to be started over

                if (kill((*itr)->pid, sig_num) == -1)
                    perror("smash error: kill failed");
                std::cout << "smash: process " << ((*itr)->pid) << " was stopped\n";
            } else if (result == -1) {
                // Error
                // usually means finished
            } else {
                // Child exited
            }

//            (*itr)->status = STOPPED;
//            (*itr)->startTime = time(nullptr); //TODO make sure it is supposed to be started over
//
//            if (kill((*itr)->pid, sig_num) == -1)
//                perror("smash error: kill failed");
//            std::cout << "smash: process " << ((*itr)->pid) << " was stopped\n";
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
//            if (kill((*itr)->pid, sig_num) == -1)
//                perror("smash error: kill failed");
//            std::cout << "smash: process " << ((*itr)->pid) << " was killed\n";
        }
    }
    // If there is no process running in the foreground, then the smash should ignore them.
}

void alarmHandler(int sig_num) {
    // TODO: Add your implementation
    std::cout << "smash: got an alarm\n"; //should be printed anyway ??
    list<TimedProcess*>* timedProcesses = &(SmallShell::getInstance().timedProcesses);

    double min_time_last = 0;

    for (list<TimedProcess*>::iterator itr = timedProcesses->begin(); itr != timedProcesses->end(); itr++)
    {
        double current_itr_diff = ((*itr)->duration - difftime(time(nullptr), (*itr)->startTime));
        double first_diff = difftime(time(nullptr), timedProcesses->front()->startTime);
        min_time_last = (timedProcesses->front()->duration - first_diff);


        if ((*itr)->duration <= difftime(time(nullptr), (*itr)->startTime)) //maybe need to use >= and check for the minimum
        {
            //std::cout << "exists \n";
            if (waitpid((*itr)->pid, nullptr, WNOHANG) == 0) //still alive
            {
                std::cout << "smash: timeout " << (*itr)->duration << " " << (*itr)->cmd_line << " timed out!\n";
                if (kill((*itr)->pid, SIGKILL) == -1)
                    perror("smash error: kill failed");
                itr = timedProcesses->erase(itr);
                itr--;
            }
            //else
            //    std::cout << "died already \n";
        }
        else{
            if (current_itr_diff < min_time_last)
                min_time_last = current_itr_diff;
        }

    }
    alarm(min_time_last);
}
