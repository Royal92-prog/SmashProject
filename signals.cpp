#include <iostream>
#include <signal.h>
#include "signals.h"
#include "Commands.h"
#include <sys/wait.h>
using namespace std;

void ctrlZHandler(int sig_num) {

    cout<<"smash: got ctrl-Z\n";
    unsigned int i=0;
    if(foreground_pid>0)
    {
        time_t time_left;
        time(&time_left);
        current_job.counter+=difftime(time_left,current_job.time_elapsed);
        current_job.running="(stopped)";
        auto it=job->cbegin();
        auto it2=it;
        while(it!=job->cend())
        {
            if(current_job.jobid<job->cbegin()->jobid){
                job->insert(job->cbegin(),current_job);
            break;}
            it2=it;
            it2++;
            if(i+1==job->size() && it->jobid<current_job.jobid){
                job->push_back(current_job);
            break;}
            if(it!=job->cbegin()){
                it2=it;
            it2--;
                if(it2->jobid<current_job.jobid && it->jobid>current_job.jobid){
                    job->insert(it,current_job);
                    break;}}
            it++;
            i++;
        }
        auto iter=timeoutProc.cbegin();
        while(iter!=timeoutProc.cend())
        {
            if(iter->pid==foreground_pid)
            {
                iter->stopped=true;
            }}

        if(job->size()==0)job->push_back(current_job);
       kill(foreground_pid,SIGSTOP);
        cout<<"smash: process "<<foreground_pid<<" was stopped\n";
        foreground_pid=-1;
    }
}

void ctrlCHandler(int sig_num)
{
    cout<<"smash: got ctrl-C\n";
   if(foreground_pid>0){
       kill(foreground_pid,SIGINT);
     waitpid(foreground_pid,nullptr,WUNTRACED);
   cout<<"smash: process "<<foreground_pid<<" was killed\n";
  auto it=timeoutProc.cbegin();
  while(it != timeoutProc.cend())
  {
      if(it->pid==foreground_pid){
          timeoutProc.erase(it);
          break;
      }
  }
       foreground_pid=-1;
   }
}

void alarmHandler(int sig_num)
{
    int min=0,index=-1,i=0,swapped=0;
    auto it =timeoutProc.cbegin();
    time_t timeNow;
    while(it!=timeoutProc.cend())
    {
        time(&timeNow);
        if(it->duration-difftime(timeNow,it->timestamp)<0||
         (!it->stopped && it->total-difftime(timeNow,it->timestamp)<0))

        {
            timeoutProc.erase(it);
            continue;
        }
        if(it->duration-difftime(timeNow,it->timestamp)<=min ||swapped==0 )
        {
            swapped=1;
            min=it->duration-difftime(timeNow,it->timestamp);
            index=i;
        }
    i++;
    it++;}

    i=0;
    it=timeoutProc.cbegin();
    cout << "smash: got an alarm\n";
    if(index>-1) {
        while (i < index) {
            i++;
            it++;
        }
        auto it2=externalSmash->jobs->jobs_vec->cbegin();
        while(it2!=externalSmash->jobs->jobs_vec->cend())
        {

            if(it2->process_id==it->pid)
            {
                externalSmash->jobs->jobs_vec->erase(it2);
               // flag=true;
                break;
            }
            it2++;
        }
        kill(it->pid, SIGINT);
        int res=waitpid(it->pid,nullptr,WUNTRACED);
        if(res!=0) {
            time(&timeNow);
              cout << "smash: ";
              cout << it->command;
              cout << " timed out!\n";
              timeoutProc.erase(it);
              foreground_pid=-1;
        }}
    time(&timeNow);
    alarm(timeoutProc.cbegin()->duration-difftime(timeNow,timeoutProc.cbegin()->timestamp));
}

