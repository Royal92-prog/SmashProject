#include <unistd.h>
#include <string.h>
#include <iostream>
#include <vector>
#include <sstream>
#include <sys/wait.h>
#include <iomanip>
#include "Commands.h"
#include <stdlib.h>
#include <stdio.h>
#include <fcntl.h>
#include <fstream>

using namespace std;
pid_t  foreground_pid =-1;
bool is_running=true;
std::vector <JobsList::JobEntry>* job= nullptr;
JobsList::JobEntry current_job;
std::vector <VT> timeoutProc;//new std::vector<VT>;
VT curr;
const std::string WHITESPACE = " \n\r\t\f\v";

#if 0
#define FUNC_ENTRY()  \
  cerr << __PRETTY_FUNCTION__ << " --> " << endl;

#define FUNC_EXIT()  \
  cerr << __PRETTY_FUNCTION__ << " <-- " << endl;
#else
#define FUNC_ENTRY()
#define FUNC_EXIT()
#endif

#define DEBUG_PRINT cerr << "DEBUG: "

#define EXEC(path, arg) \
  execvp((path), (arg));
SmallShell* externalSmash = nullptr;
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

static bool isInternalCommand(char* argv1)
{
    return     strcmp(argv1,"ls")==0|| strcmp(argv1,"chprompt")==0//
                || strcmp(argv1,"showpid")==0 || strcmp(argv1,"pwd")==0
                || strcmp(argv1,"cd")==0||strcmp(argv1,"jobs")==0||
                strcmp(argv1,"kill")==0|| strcmp(argv1,"bg")==0
                || strcmp(argv1,"fg")==0|| strcmp(argv1,"quit")==0;
    //SmallShell& cmd = SmallShell::getInstance();
    //cmd.executeCommand(argv1[0]);

}
// TODO: Add your implementation for classes in Commands.h
/** internal commands implementation**/
static void Remove_spaces_From_Front(string *s){
    while ((*s).front()== ' '){
        *s=s->substr(1,s->length());
    }
}

static void Remove_spaces_From_Back(string *s) {
    while ((*s).back() == ' ') {
        *s= s->substr(0, s->length() - 1);
    }
}



/*command No.1*/
void ChangeChprompt::execute() {
    char* argv[30]={nullptr};
    _parseCommandLine(cmd_line,argv);
    /*
   ________________ string str=cmd_line;
    Remove_spaces_From_Front(&str);
    std::string new_str= str.substr(8,str.length());
    Remove_spaces_From_Front(&new_str);
    Remove_spaces_From_Back(&new_str);___________________*/
    if(!argv[1])
    {
        *s="smash" ;
        return;;
    }
    else  *s=argv[1];

    for (int i = 0; i <30 ; ++i) {
        if(argv[i])  delete  argv[i];
        else break;
    }

}

/*command No.2*/

void lsPrompt::execute()
{
    struct dirent ** dir;
    int n=0,i=0;
    n=scandir(".", &dir, nullptr,alphasort);
    if (n < 0) {
        perror("smash error: chdir failed");
    }
    //else {
    for (i = 0; i < n; i++)
    {
        cout<<dir[i]->d_name<<"\n";
        delete dir[i];
    }
    delete dir;
}


/*command No.3*/
void ShowPidCommand::execute()
{
    cout<<"smash pid is "<<getpid()<<"\n";
}

/*command No.5*/

void ChangeDirCommand::execute()
{

    char* argv[30]={nullptr};
    _parseCommandLine(cmd_line,argv);
    if(argv[2]) {
        cout << "smash error: cd: too many arguments\n";
        return;
    }
    auto it=this->directory->cend();
    it--;
    auto it2=it;
    it2--;
   // auto it3=it2;
    if(*argv[1]=='-')
    {
        if(this->directory->size()==1)
        { cout<<"smash error: cd: OLDPWD not set\n";
        }
        else{
            char x[100]={0};
            strcpy(x,it2->data());
            if(chdir(x)==-1) {
                cout<<"error 1st  "<<it->data()<<"\n";
                perror("smash error: chdir failed");
                return;
            }
            string temp2=it2->data();
            this->directory->erase(it2);
            this->directory->push_back(temp2);
        }
        return;
    }

    if(chdir(argv[1])==-1)
    {

        perror("smash error: chdir failed");
        return;
    }
    this->directory->push_back(argv[1]);
}

/*command No.7*/
void KillCommand::execute()
{
    char* argv[30]={nullptr};
    string str=cmd_line;
    Remove_spaces_From_Front(&str);
    std::string new_str= str.substr(4,str.length());
    Remove_spaces_From_Front(&new_str);
    Remove_spaces_From_Back(&new_str);

    _parseCommandLine(cmd_line,argv);
    if(argv[3]||!argv[2]) {
        cout<<"smash error: kill: invalid arguments\n";
        return;}


    for (int j = 1; j <3 ; ++j) {
        char* temp=argv[j];
        if(j==1){
            if(*temp!='-' )
            {
            cout<<"smash error: kill: invalid arguments\n";
                return;
            }temp++;}
        if(*temp=='-' && j==2){
            cout<<"smash error: kill: job-id "<<argv[2]<<" does not exist\n";
            return;}
        while(*temp!='\0'){ if(*temp != '0' && *temp != '1' && *temp != '2' &&
        *temp != '3' && *temp != '4' &&
                            *temp != '5' && *temp != '6' && *temp != '7' && *temp != '8'
                            && *temp != '9') {

            cout<<"smash error: kill: invalid arguments\n";
            return;
        }
            temp++;}
    }
    new_str=new_str.substr(1,new_str.length());

    string signum,jobid;
    auto iter= new_str.cbegin();
    while(*iter !=' '){
        signum.push_back(*iter);
        iter++;
    }

    int b=atoi(signum.data()); // checking signal numbers

    while(iter != new_str.cend()){
        jobid.push_back(*iter);
        iter++;
    }
    auto it=jobs->jobs_vec->cbegin();
    while(iter != new_str.cend()){
        jobid.push_back(*iter);
        iter++;
    }


    int i=0;
    int a=atoi(argv[2]);
    if(a<=0){cout<<"smash error: kill: job-id "<<a<<" does not exist\n";
        return;}
    while(it!=jobs->jobs_vec->cend()){
        int b=it->jobid;
        if(b==a)break;
        it++;
        i++;
    }
     int c=jobs->jobs_vec->size();
     if(i==c){
         cout<<"smash error: kill: job-id "<<a<<" does not exist\n";
         return;
     }
    i=0;
    if( b<=0 || b>31 )//b>32
    {
        cout<<"smash error: kill: invalid arguments\n";
        return;
    }
    if(kill(it->process_id,b)==-1)
    {
        perror("smash error: kill failed");
    }
    cout<<"signal number "<<b<<" was sent to pid "<<it->process_id<<"\n";
    if(b==9){
        jobs->jobs_vec->erase(it);
    job=jobs->jobs_vec;}

}

/*command No.8*/
void ForegroundCommand::execute() {

    string str = cmd_line;
    char *argv[30] = {nullptr};
    _parseCommandLine(cmd_line, argv);
    int a = 101;
    int b=jobs->jobs_vec->size()-1,c=0;
    time_t time2;
    auto it = jobs->jobs_vec->cbegin();
    if (!argv[1]){
        if (jobs->jobs_vec->size() == 0) {
            cout << "smash error: fg: jobs list is empty\n";
            return;
        }
        while(c!=b) {
            c++;
            it++;
        }
        while (1)
        {

            time(&time2);
            if((it->counter+difftime(time2,it->time_elapsed))-it->total_time<0
            || it->running=="(stopped)")//
            {cout<<it->command<<" : "<<it->process_id<<"\n";
               if(it->running=="(stopped)"){
                it->running="";
                time(&it->time_elapsed);
               }
                current_job.jobid=it->jobid;
                current_job.total_time=it->total_time;
                current_job.time_elapsed=it->time_elapsed;
                current_job.running=it->running;
                current_job.process_id=it->process_id;
                current_job.command=it->command;
                current_job.counter=it->counter;
               int num=it->process_id;
                foreground_pid=num;
                jobs->jobs_vec->erase(it);
                if(kill(it->process_id,SIGCONT)==-1){
                    perror("smash error: kill failed");
                }

                  if( waitpid(num, nullptr,WUNTRACED)==-1){
                    //  perror("smash error: waitpid failed");
                  }
                foreground_pid=-1;
               return;
            }

            if(it==jobs->jobs_vec->cbegin())
            {
                foreground_pid=-1;
                cout << "smash error: fg: jobs list is empty\n";
                return;
            }
            it--;
        }}
    int i = 0;
    char* num = new char[80];
    if(argv[1])strcpy(num,argv[1]);
    if (a > 100 && argv[1]) {
        char* temp=argv[1];
        if(temp[0]=='-')temp++;
        while (*temp!=0){
            if(*temp>'9' || *temp<'0')  {
                cout << "smash error: fg: invalid arguments\n";
            return;}
            temp++;
        }
        while (num[i] == '0' || num[i] == '1' || num[i] == '2' || num[i] == '3' || num[i] == '4' ||
               num[i] == '5' || num[i] == '6' || num[i] == '7' || num[i] == '8'
               || num[i] == '9') {

            i++;
        }
        a = atoi(argv[1]) ;
        if(a<=0){
            cout<<"smash error: fg: job-id "<<a<<" does not exist\n";
            foreground_pid=-1;
            return;
        }
        int d=strlen(num);
        if ((argv[1] &&i != d)|| argv[2]) {
            foreground_pid=-1;
            cout << "smash error: fg: invalid arguments\n";
            return;
        }

         }
    i=0;
    while (i < b+1) {
    int d=it->jobid;
    if(d==a)  break;
        i++;
        it++;}
    int y=jobs->jobs_vec->size();
    if(i==y)
    {cout<<"smash error: fg: job-id "<<a<<" does not exist\n";
        foreground_pid=-1;
        return;}
    time(&time2);
    if( it->running!="(stopped)" && (it->counter +difftime(time2,it->time_elapsed))-it->total_time>0)//
    {
        cout<<"smash error: fg: job-id "<<a<<" does not exist\n";
        return;
    }
    cout<<it->command<<" : "<<it->process_id<<"\n";
    if(it->running=="(stopped)"){
    it->running="";
    time(&it->time_elapsed);}
    current_job.jobid=it->jobid;
    current_job.total_time=it->total_time;
    current_job.time_elapsed=it->time_elapsed;
    current_job.running=it->running;
    current_job.process_id=it->process_id;
    current_job.command=it->command;
    current_job.counter=it->counter;
    foreground_pid=it->process_id;
    jobs->jobs_vec->erase(it);
    if(kill(foreground_pid,SIGCONT)==-1){
       // perror("smash error: kill failed");
    }
    if(waitpid(foreground_pid, nullptr,WUNTRACED)==-1){
       // perror("smash error: waitpid failed");
    }
   /* for (int j = 0; j <30 ; ++j) {
        if(argv[i])delete argv[i];
        else break;}*/
}
/*command No.9*/
void BackgroundCommand::execute()
{
    char* argv[30]={nullptr};
    unsigned int i=0;
    _parseCommandLine(cmd_line,argv);
    if(!argv[1])
    {    if(jobs->jobs_vec->size()==0)
        {
            cout<<"smash error: bg: there is no stopped jobs to resume\n";
            return;
        }
        auto it = jobs->jobs_vec->cend();
        it--;
        while (1)
        {

            if(it->running!="")
            {//cout<<"inside here:)\n";
                cout<<it->command<<" : "<<it->process_id<<"\n";
                it->running="";
               if( kill(it->process_id,SIGCONT)==-1){
                   perror("smash error: kill failed");
               }

                for (int i = 0; i <30 ; ++i) {
                    if(argv[i])delete argv[i];
                    else break;
                }
                return;
            }
            if(it==jobs->jobs_vec->cbegin()) break;
            it--;

        }
        cout<<"smash error: bg: there is no stopped jobs to resume\n";
        for (int i = 0; i <30 ; ++i) {
            if(argv[i])delete argv[i];
            else break;}
        return;
    }
    auto it=jobs->jobs_vec->cbegin();
    if(argv[1]&& !argv[2])
    {
         int a= atoi(argv[1]);
        if(a<=0)
        {
            cout<<"smash error: bg: job-id "<<a<<" does not exist\n";
            return;
        }
        while (it!=jobs->jobs_vec->cend()){
            int c=it->jobid;
            if(c==a)break;
            it++;
            i++;
            }
        if(i==jobs->jobs_vec->size())
        {
            cout<<"smash error: bg: job-id "<<a<<" does not exist\n";
            return;
        }
        if(it->running=="\0")
        {
            cout<<"smash error: bg: job-id "<<a<<" is already running in the background\n";
            for (int i = 0; i <30 ; ++i) {
                if(argv[i])delete argv[i];
                else break;}
            return;
        }
        cout<<it->command<<" : "<<it->process_id<<"\n";
        if(kill(it->process_id,SIGCONT)==-1){
            perror("smash error: kill failed");
        }
        it->running="\0";
    }
    else
    {cout<<"smash error: bg: invalid arguments\n";}
    for (int i = 0; i <30 ; ++i) {
        if(argv[i])delete argv[i];
        else break;}
}


/*command No.10*/
void QuitCommand::execute()
{	time_t time_now;
	time(&time_now);
    char* argv[30]={nullptr};
    _parseCommandLine(cmd_line,argv);
    auto it = jobs->jobs_vec->cbegin();
    while (it!= jobs->jobs_vec->cend()){
       double y=difftime(time_now,it->time_elapsed)+it->counter;
        if(it->total_time<y+0.4)
        {
            jobs->jobs_vec->erase(it);
            continue;
        }
        it++;
    }
    it = jobs->jobs_vec->cbegin();
    if(argv[1]){
        cout<<"smash: sending SIGKILL signal to "<<jobs->jobs_vec->size()<<" jobs:\n";
        while(jobs->jobs_vec->size()!=0 && it != jobs->jobs_vec->cend())
        {

            if(kill(it->process_id,SIGKILL)==-1){
                perror("smash error: kill failed");
                for (int i = 0; i <30 ; ++i)
                {
                    if(argv[i]) delete argv[i];
                    else break;}
                return;
            };
            cout <<it->process_id<<": "<<it->command<<"\n";
            it++;
        }}

    for (int i = 0; i <30 ; ++i)
    {
        if(argv[i]) delete argv[i];
        else break;}
    is_running= false;
}

/*Bonus Command $$$*/
void cpCommand::execute() {
    pid_t pid =fork();
    if (pid == 0) {
        char *argv[81], data[81] = {0};
        _parseCommandLine(cmd_line, argv);
        int new_fdt = open(argv[1], O_RDONLY, 0666);
        int new_fdt2 = open(argv[2], O_RDWR | O_CREAT | O_TRUNC, 0666);//creat(argv[2],0666);
        if (new_fdt == -1 || new_fdt2 == -1) {
            perror("smash error: open failed");
            return;
        }
        while (read(new_fdt, data, 1) > 0) {
            write(new_fdt2, data, 1);
        }

        cout<<"smash: "<<argv[1]<<" was copied to "<<argv[2]<<"\n";
    close(new_fdt);
    close(new_fdt2);
       exit(0);
    }
    foreground_pid =pid;
    current_job.counter=0;
    current_job.command=cmd_line;
    time(&current_job.time_elapsed);
    current_job.running="";
    int max=1;
    time_t time2;
    if(job->size()!=0){
    auto it = job->cend();
    it--;
    while(1)
    {

        time(&time2);
        if(it->running=="(stopped)" ||
           (it->running!="(stopped)" && (((it->counter+(difftime(time2,it->time_elapsed))-it->total_time)<0))))
        {

            max=it->jobid+1;
            break;
        }
        if(it==job->cbegin()) break;
        it--;
    }}
    current_job.jobid=max;
    waitpid(pid,nullptr,WUNTRACED);
    foreground_pid =-1;
    }




void ExternalCommand::execute() {
    char *argv[30]={nullptr};
    string cmd_s = string(cmd_line);
    char s[150]={0};
     int a=0,max=1;
    strcpy(s,cmd_line);
    string num= std::string();
    int i=0;

        while(s[i]!='0'&&s[i]!='1'&&s[i]!='2'&&s[i]!='3'&&s[i]!='4'&&s[i]!='5'&&s[i]!='6'
              &&s[i]!='7'&&s[i]!='8'
              &&s[i]!='9')i++ ;
        while(s[i]=='0'||s[i]=='1'||s[i]=='2'||s[i]=='3'||s[i]=='4'||
              s[i]=='5'||s[i]=='6'||s[i]=='7'||s[i]=='8'
              ||s[i]=='9') {num.push_back(s[i]);
            i++;}
        a=atoi(num.data());
    i=0;
    int b=externalSmash->jobs->jobs_vec->size()-1;
    if(externalSmash->jobs->jobs_vec->size()>0){
    auto it=externalSmash->jobs->jobs_vec->cbegin();
    time_t time2;
            while(i!=b) {
                i++;
                it++;
            }
while (1){
    time(&time2);
    if(it->running=="(stopped)" ||
            (it->running!="(stopped)" && (((it->counter+(difftime(time2,it->time_elapsed))-it->total_time)<0))))
    {

        max=it->jobid+1;
        break;
    }
        if(it==externalSmash->jobs->jobs_vec->cbegin())
        {
            max=1;
            break;
        }
        it--;
    }}
    JobsList::JobEntry fgjob=JobsList::JobEntry(a,cmd_line,max);
    current_job.counter=fgjob.counter;
    current_job.jobid=fgjob.jobid;
    current_job.running=fgjob.running;
    current_job.process_id=fgjob.process_id;
    current_job.total_time=fgjob.total_time;
    current_job.command=fgjob.command;
    for (int j = 0; j <30 ; ++j)argv[j]=nullptr;
    i=0;
    argv[0]= new char[81];
    strcpy(argv[0],"/bin/bash");
    argv[1]=new char[81];
    strcpy(argv[1],"-c");
    argv[2]= new char[81];
    strcpy(argv[2],cmd_line);
    if(_isBackgroundComamnd(argv[2])){
       _removeBackgroundSign(argv[2]);
    }
    pid_t pid=fork();
    if (pid==-1){
        perror("smash error: fork failed");
        for (int j = 0; j <30 ; ++j)if(argv[j])
            {
                delete argv[j];
            }
        return;}

    if(pid !=0 && !_isBackgroundComamnd(s)){
        current_job.process_id=pid;
        foreground_pid=pid;
        waitpid(pid,nullptr,WUNTRACED);
        foreground_pid=-1;
    }
    else  if(pid==0){
        if(execv("/bin/bash",argv)==-1){
            perror("smash error: execv failed");
            return;
        }}

    if(_isBackgroundComamnd(s)){
    fgjob.process_id=pid;
    externalSmash->jobs->jobs_vec->push_back(fgjob);
    }
    job=externalSmash->jobs->jobs_vec;
    for (int j = 0; j <20 ; ++j) {
     if(argv[j]) delete argv[j];
      else break;}
}
void releaseArray(char** argv)
{
    for (int i = 0; i <30 ; ++i) {
        if(argv[i])delete argv[i];
    }
}


void PipeCommand:: execute() {
    char cmd2[81]={0}, cmd1[81]={0}, new_str[81];
    int curr = 0;
    bool flag= false;
    char *argv1[30]={ nullptr};
    char *argv2[30]={ nullptr};
    strcpy(new_str, cmd_line);
    while (new_str[curr] == ' ') {
        curr++;
    }
    int i = 0;
    for (int j = strlen(cmd_line) - 1; j > 0; --j) {
        if (*(cmd_line + j) != ' ') {
            new_str[j + 1] = 0;
            break;
        }
    }
    while (new_str[curr]!= '|')
    {
        cmd1[i]=new_str[curr];
        i++;
        curr++;
    }
    curr++;
    if(new_str[curr]=='&')
    {
        flag= true;
        curr++;
    }
    i=0;
    while (new_str[curr]!= 0)
    {
        cmd2[i]=new_str[curr];
        i++;
        curr++;
    }
    _parseCommandLine(cmd1,argv1);
    _parseCommandLine(cmd2,argv2);
    int fd[2];
    pipe(fd);
    int dup_num2=dup(0) ,dup_num1=dup(1);
    if(flag) dup_num1=dup(2);
    if(dup_num2==-1 ||dup_num1==-1) {
        perror("smash error: dup failed");
        releaseArray(argv1);
        releaseArray(argv2);
        return;}
    pid_t pid2=fork();
    if(pid2==-1){
        perror("smash error: fork failed");
        releaseArray(argv1);
        releaseArray(argv2);
        return;}
    if (pid2 == 0) {
        dup2(fd[0], 0);
        close(fd[0]);
        close(fd[1]);
        externalSmash->executeCommand(cmd2);
        exit(0);
    }

    else {//father + 1st process
        if(flag) dup2(fd[1], 2);
        else  dup2(fd[1], 1);
        close(fd[0]);
        close(fd[1]);
        // /home/student/hw1.1/os1-tests-d9a8f12b49078df05fb2a1fe943c260955414fe4/
        if (isInternalCommand(argv1[0])) {
            SmallShell &cmd = SmallShell::getInstance();
            cmd.executeCommand(argv1[0]);
        }
        else
        {
            int pid1=fork();
            if(pid1==-1){
                perror("smash error: fork failed");
                releaseArray(argv1);
                releaseArray(argv2);
                return;}

            if(pid1==0)
            {
                externalSmash->executeCommand(cmd1);
                exit(0);
            }
            waitpid(pid1, nullptr, WUNTRACED);//if(foreground_pid==pid1)
        }
    }
    dup2(dup_num2, 0);
    if(flag)dup2(dup_num1, 2);
    else dup2(dup_num1, 1);
    waitpid(pid2, nullptr, WUNTRACED);
}

void RedirectionCommand ::execute() {
    char out[81]={0}, cmd1[81]={0}, new_str[81];
    int curr = 0,i=0,new_fdt=0;
    bool flag=false;
    char *argv1[30]={nullptr};
    for (int j = 0; j < 30; ++j) argv1[j] = nullptr;
    strcpy(new_str, cmd_line);
    while (new_str[curr] == ' ') {
        curr++;
    }
    for (int j = strlen(cmd_line) - 1; j > 0; --j) {
        if (*(cmd_line + j) != ' ') {
            new_str[j + 1] = 0;
            break;
        }
    }

    while (new_str[curr]!= '>')
    {
        cmd1[i]=new_str[curr];
        i++;
        curr++;
    }
    curr++;
    if(new_str[curr]=='>')
    {flag= true;
        curr++;}
    while(new_str[curr]!=0 &&new_str[curr]==' ')curr++;
    i=0;
    while (new_str[curr]!= 0)
    {
        out[i]=new_str[curr];
        i++;
        curr++;
    }
    _parseCommandLine(cmd1,argv1);//   /home/student/Os,hw1/
    if(!flag) new_fdt=open(out,O_RDWR | O_CREAT | O_TRUNC,0666);
    if(new_fdt==-1){
        perror("smash error: open failed");
        return;
    }
    else new_fdt=open(out,O_RDWR | O_APPEND,0666);
    if(new_fdt==-1){
        perror("smash error: open failed");
        return;
    }
    int temp=dup(1);
    if(temp==-1){
        perror("smash error: dup failed");
        return;
    }
    if(dup2(new_fdt,1)==-1){
        perror("smash error: dup2 failed");
        return;
    }

   externalSmash->executeCommand(cmd1);

    if(close(new_fdt)==-1){
        perror("smash error: close failed");
        return;
    }
    if(dup2(temp, 1)==-1){
        perror("smash error: dup2 failed");
        return;
    }
}


void Timeout::execute()
{
    char **argv=new char*[30];
    char **argv2=new char*[30];
    for (int k = 0; k <30 ; ++k) { argv[k]= nullptr;
    argv2[k]= nullptr;}
    unsigned int i=0;
    _parseCommandLine(cmd_line,argv+2);
    _parseCommandLine(cmd_line,argv2);
    JobsList::JobEntry myjob;

    if(!argv2[1]|| !argv2[2]|| *(argv2[1])=='-'){
        cout<<"smash error: timeout: invalid arguments\n";
        return;
    }
    auto it = externalSmash->jobs->jobs_vec->cend();
    it--;
        int max=1;
        myjob.command=cmd_line;
        myjob.total_time=atoi(argv[3]);
        if(_isBackgroundComamnd(cmd_line)&&externalSmash->jobs->jobs_vec->size()==0){
            myjob.jobid=1;
            externalSmash->jobs->jobs_vec->push_back(myjob);
        }
        else{ time_t timenow;
            while (it!=externalSmash->jobs->jobs_vec->cbegin() && externalSmash->jobs->jobs_vec->size()>0)
            {
                time(&timenow);
                if(it->running=="(stopped)" || difftime(timenow,it->time_elapsed)+it->counter -it->total_time<0)
                {myjob.jobid=it->jobid+1;
                    max= myjob.jobid;
                  if(_isBackgroundComamnd(cmd_line)) externalSmash->jobs->jobs_vec->push_back(myjob);
                    break;
                }
                it++;}}
    char* num = new char[81];
    for (int j = 0; j < 81; j++) num[j]=0;
    if(argv2[1]) {
        strcpy(num, argv2[1]);
        while (num[i] == '0' || num[i] == '1' || num[i] == '2' || num[i] == '3' || num[i] == '4' ||
               num[i] == '5' || num[i] == '6' || num[i] == '7' || num[i] == '8'
               || num[i] == '9')
            i++;
      if (( i!= strlen(num))){delete[] num;
            return;
        }}
       delete[] num;
       int a=atoi(argv2[1]);
        if(a<=0){
           cout<<"smash error: timeout: invalid arguments\n";
           return;
       }
    //max sorts of vectors
    auto it2 =timeoutProc.cbegin();
    while (it2!=timeoutProc.cend())
    {
        time_t time_now;
        time(&time_now);
        if(it2->duration-difftime(time_now,it2->timestamp)<0 ||
                (!it2->stopped && it2->total-difftime(time_now,it2->timestamp)<0))
        {timeoutProc.erase(it2);
            continue;
        }
    it2++;}
    curr.total=atoi(argv2[3]);
    curr.duration=atoi(argv2[1]);
  int pid=fork();
   if(pid==0) {
       argv[0]= new char[81];
       strcpy(argv[0],"/bin/bash");
       argv[1]=new char[81];
       strcpy(argv[1],"-c");
       argv[2]= new char[81];
       strcpy(argv[2],cmd_line);
       _removeBackgroundSign(argv[2]);
           execv("/bin/bash", argv);
   }
   if(pid==-1){
       perror("smash error: fork failed");
       return;}
    myjob.process_id=pid;
    time(&curr.timestamp);
    curr.insertCh(cmd_line);
    std::vector<VT> tempVec;
    curr.pid=pid;
    timeoutProc.push_back(curr);
    bool checked[101]={0};
    int index=-1,c=timeoutProc.size();
    double minTime=0;
    auto it4=timeoutProc.cbegin();
    time_t time_now2;
    for (int k = 0; k <c ; k++)
    {
        it2=timeoutProc.cbegin();
        for (int j = 0; j <c ; ++j) {
            time(&time_now2);
            if(checked[j]==0&&(it2->duration-difftime(time_now2,it2->timestamp)<minTime|| index==-1))
            {
                index=j;
                minTime=it2->duration-difftime(time_now2,it2->timestamp);
            }
            it2++;
        }
        checked[index]=1;
        auto it3=timeoutProc.cbegin();
        int h=0;
        while (h<index){
            it3++;
            h++;
        }
        tempVec.push_back(*it3);
        index=-1;}
    tempVec.swap(timeoutProc);
     it4=timeoutProc.cbegin();
    time(&time_now2);
    alarm(timeoutProc.cbegin()->duration-difftime(time_now2,timeoutProc.cbegin()->timestamp));
    if(!_isBackgroundComamnd(cmd_line)) {
        current_job.process_id=pid;
        current_job.command=cmd_line;
        current_job.jobid=max;
        current_job.time_elapsed= curr.timestamp;
        current_job.total_time=atoi(argv2[3]);
        current_job.running="";
        current_job.counter=0;
       // cout<<"Line1179\n";
        foreground_pid=pid;
         waitpid(pid,nullptr,WUNTRACED);
    }
}

/**
* Creates and returns a pointer to Command class which matches the given command line (cmd_line)
*/
Command * SmallShell::CreateCommand(const char* cmd_line) {
    string cmd_s = string(cmd_line);
    if( cmd_s.find_first_of('|') != string::npos)
    {
                return new PipeCommand(cmd_line);}

    else if( cmd_s.find_first_of('>') != string::npos)
    {
        return new RedirectionCommand(cmd_line);
    }
    else if( cmd_s.find("ls") ==0)
    {
        return new lsPrompt(cmd_line);
    }

    else if (cmd_s.find("pwd") ==0) {

        return new GetCurrDirCommand(cmd_line);
    }
    else if (cmd_s.find("chprompt")==0) {

        return new ChangeChprompt(cmd_line,&(this->s));
    }
    else if (cmd_s.find("showpid")==0) {
        return new ShowPidCommand(cmd_line);
    }

    else if (cmd_s.find("cd")==0) {
        return new ChangeDirCommand(cmd_line,&this->directory);
    }
    else if (cmd_s.find("kill")==0) {

        return new KillCommand(cmd_line,this->jobs);
    }

    else if (cmd_s.find("bg")==0) {

        return new BackgroundCommand(cmd_line,this->jobs);
    }
    else if (cmd_s.find("fg")==0) {



        return new ForegroundCommand(cmd_line,this->jobs);
    }

    else if (cmd_s.find("jobs")==0) {

        return new JobsCommand(cmd_line,this->jobs);
    }
    else if (cmd_s.find("quit")==0) {
        return new QuitCommand(cmd_line,this->jobs);
    }
         else if (cmd_s.find("timeout")==0) {
             return new Timeout(cmd_line);//this
         }
    else if (cmd_s.find("cp")==0) {
        return new cpCommand(cmd_line);//this
    }



    else {

        return new ExternalCommand(cmd_line);
    }

}


void SmallShell::executeCommand(const char *cmd_line) {
    Command* cmd = CreateCommand(cmd_line);
    if(cmd) cmd->execute();
    delete cmd;
}
