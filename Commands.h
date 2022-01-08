#ifndef SMASH_COMMAND_H_
#define SMASH_COMMAND_H_
#include <vector>
#include <string.h>
#include <dirent.h>
#include <unistd.h>
#include <string.h>
#include <time.h>
#include <iostream>
#include <list>
using std::string;
using  std::cout;

#define COMMAND_ARGS_MAX_LENGTH (200)
#define COMMAND_MAX_ARGS (20)
#define HISTORY_MAX_RECORDS (50)



 extern pid_t foreground_pid;
 extern bool is_running;

class Command {
protected:
   const char* cmd_line;
public:
    Command(const char* cmd_line): cmd_line(cmd_line){};
    virtual ~Command()= default;
    virtual void execute() = 0;
    //virtual void prepare();
    //virtual void cleanup();
    // TODO: Add your extra methods if needed
};

class BuiltInCommand : public Command {
public:
    BuiltInCommand(const char* cmd_line):Command(cmd_line){};
    virtual ~BuiltInCommand() = default;
};


class JobsList {
public:
    class JobEntry {

    public:

        mutable unsigned int jobid;
        mutable double counter;
        mutable double total_time;
        mutable string command;
        mutable int process_id;
        mutable  string running;
        mutable  time_t time_elapsed;
        JobEntry(int total_time=0,string command=std::string(),unsigned int jobid=0 ):jobid(jobid),counter(0),total_time(total_time),command(command)
                ,process_id(getpid()),running("\0"){
            time(&time_elapsed);

        };
        ~JobEntry()= default;
    };
     std::vector<JobEntry> *jobs_vec;
public:

    JobsList():jobs_vec(new std::vector<JobEntry>){};
    ~JobsList()
    {delete jobs_vec;}
    void addJob(Command* cmd, bool isStopped = false);
    void printJobsList();
    void killAllJobs();
    void removeFinishedJobs();
    JobEntry * getJobById(int jobId);
    void removeJobById(int jobId);
    JobEntry * getLastJob(int* lastJobId);
    JobEntry *getLastStoppedJob(int *jobId);

};
//extern JobsList::JobEntry myjob;

 extern std::vector <JobsList::JobEntry>* job;
  extern JobsList::JobEntry current_job;

/*command No.1*/

class ChangeChprompt: public BuiltInCommand{

    string* s;
public:
    //ChangeChprompt(string cmd_line);
    ChangeChprompt(const char* cmd_line,string* s):BuiltInCommand(cmd_line),s(s){};
    virtual ~ChangeChprompt() {}
    void execute() override;

};

/*command No.2*/

class lsPrompt: public BuiltInCommand{

public:

    lsPrompt(const char* cmd_line):BuiltInCommand(cmd_line){};
    virtual ~lsPrompt() {}
    void execute() override;

};

/*command No.3*/
class ShowPidCommand : public BuiltInCommand {
public:
    ShowPidCommand(const char* cmd_line):BuiltInCommand(cmd_line){};
    virtual ~ShowPidCommand() {}
    void execute() override;

};

/*command No.4 - pwd command */

class GetCurrDirCommand : public BuiltInCommand {
public:
    GetCurrDirCommand(const char* cmd_line):BuiltInCommand(cmd_line){};

    virtual ~GetCurrDirCommand() {}
    void execute() override
    {
       if(get_current_dir_name()) std::cout<<get_current_dir_name()<<"\n";
       else perror("smash error: get_current_dir_name failed");
    }
};

/*command No.5*/

class ChangeDirCommand : public BuiltInCommand {
    mutable std::vector<string>* directory;

public:
    ChangeDirCommand(const char* cmd_line,std::vector<string>* directory):
            BuiltInCommand(cmd_line),directory(directory){};
    virtual ~ChangeDirCommand() {}
    void execute() override;

};

/*command No.6*/
class JobsCommand : public BuiltInCommand {
    JobsList* jobs;
public:
    JobsCommand(const char* cmd_line, JobsList* jobs):
    BuiltInCommand(cmd_line),jobs(jobs){};
    virtual ~JobsCommand() {}
    void execute() override
    {
        auto it=jobs->jobs_vec->cbegin();
        while (it!=jobs->jobs_vec->cend())
        {
            time_t time_now;
            time(&time_now);
            if(it->running!="(stopped)" &&
            it->total_time<difftime(time_now,it->time_elapsed)+it->counter)
            {
                jobs->jobs_vec->erase(it);
                continue;
            }
            if(it->running==""){
            cout<<"["<<it->jobid<<"] "<<it->command<<" : "<<it->process_id<<" "
            <<difftime(time(&time_now),it->time_elapsed)<<" secs "<<it->running<<"\n";}
            else{
                cout<<"["<<it->jobid<<"] "<<it->command<<" : "<<it->process_id<<" "
                    <<it->counter<<" secs "<<it->running<<"\n";
            }
            it++;
        }
    };
};

/*command No.7*/
class KillCommand : public BuiltInCommand {
    JobsList* jobs;
public://, JobsList* jobs
    KillCommand(const char* cmd_line,JobsList* jobs):BuiltInCommand(cmd_line),
                                                     jobs(jobs){};

    virtual ~KillCommand() {}
    void execute() override;
};
/*command No.8*/
class ForegroundCommand : public BuiltInCommand {
    JobsList* jobs;
public:
    ForegroundCommand(const char* cmd_line, JobsList* jobs):BuiltInCommand(cmd_line),jobs(jobs){};
    virtual ~ForegroundCommand() {}
    void execute() override;
};

/*command No.9*/

class BackgroundCommand : public BuiltInCommand {
    JobsList *jobs;
public:
    BackgroundCommand(const char *cmd_line, JobsList *jobs) : BuiltInCommand(cmd_line), jobs(jobs) {};

    virtual ~BackgroundCommand()= default;

    void execute() override;
};
/*command No.10*/
class JobsList;

class QuitCommand : public BuiltInCommand {
    JobsList* jobs;
    virtual ~QuitCommand() {}
    void execute() override;

public:
    QuitCommand(const char* cmd_line, JobsList* jobs):BuiltInCommand(cmd_line)
    ,jobs(jobs){}
};

/*Bonus Command*/
class JobsList;

class cpCommand : public BuiltInCommand {
    JobsList* jobs;
    virtual ~cpCommand() {}
    void execute() override;

public:
    cpCommand(const char* cmd_line):BuiltInCommand(cmd_line){}
};





/** External Command**/


class ExternalCommand : public Command {

public:
    ExternalCommand(const char* cmd_line):Command(cmd_line){};
    virtual ~ExternalCommand() {}
    void execute() override;
};
 /** Redirection Command**/

class RedirectionCommand : public Command {

public:
    explicit RedirectionCommand(const char* cmd_line):Command(cmd_line){};
    virtual ~RedirectionCommand() {}
    void execute() override;
    //void prepare() override;
    //void cleanup() override;
};



class PipeCommand : public Command {

public:
    PipeCommand(const char* cmd_line):Command(cmd_line){};
    virtual ~PipeCommand() {}
    void execute() override;
};



class VT{
public:
    int pid;
    time_t timestamp;
    double duration;
     char* command;
     int total;
    mutable bool stopped;
   VT(int pid=-1,
 double duration=0,time_t time1=0):pid(pid),timestamp(time1),
 duration(duration),command(nullptr),total(0),stopped(false){};
   void insertCh(const char* ch)
   {
       this->command= new char [100];
       for (int i = 0; i <100 ; ++i) {
           this->command[i]=0;
       }
       if(ch) strcpy(this->command,ch);
   }
   ~VT()
   {
         /*   if(command) {
                for (int i = 0; i < 30; ++i) {
                    if (command[i])delete command[i];
                    command[i]= nullptr;
                }
       delete command;
       command= nullptr;
            }*/}
};



class Timeout: public Command
{
public:
    //SmallShell* smash;smash(smash),
    //std::vector<vecTimeout> vecTimeout;SmallShell* smash ,


    Timeout(const char* cmd_line):Command(cmd_line){};
    virtual ~Timeout(){};
    void execute() override;


};

  extern std::vector <VT> timeoutProc;
extern VT curr;
class SmallShell {
private:
    std::string s;
    std::vector<string> directory;
    int curr_pid;
    //int maxJob;
    SmallShell():s("smash"),
    curr_pid(-1),jobs(new JobsList){
       directory.push_back(get_current_dir_name());
        job=jobs->jobs_vec;
       // cout<<"Hi pid is: "<<getpid()<<"\n";
    };
public:
  //  std::vector<VT> vecTimeout;
    JobsList* jobs;
    Command *CreateCommand(const char* cmd_line);
    SmallShell(SmallShell const&)      = delete; // disable copy ctor
    void operator=(SmallShell const&)  = delete; // disable = operator
    static SmallShell& getInstance() // make SmallShell singleton
    {
        static SmallShell instance; // Guaranteed to be destroyed.
        // Instantiated on first use.
        return instance;
    }

    ~SmallShell()
    {
     //   delete jobs;
    }
    void executeCommand(const char* cmd_line);
    //friend ChangeChprompt;
    void get_chprompt(){
        std::cout<< this->s<<"> ";
    }
   friend ExternalCommand;
};
extern  SmallShell* externalSmash;
#endif //SMASH_COMMAND_H_
