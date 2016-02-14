#include <vector>
#include <stdio.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <stdlib.h>
#include <unistd.h>
#include <string>
#include <iostream>
#include <fcntl.h>
#include <sys/stat.h> 
#include <string.h>
#include <sys/dir.h>
#include <sys/param.h>
#include <errno.h>
#include <iostream>
#include <fstream>

using namespace std;

string server;
int port=-1;
int fg_proc=-1,bg_proc=-1;		// for making groups
bool signal_passed=0;			

bool file_exists(string fileName)					// checks if file exists
{
    ifstream infile(fileName.c_str(),ios::in);
    return infile.good();
}

void sig_handler(int signo){						// signal handler for main shell proc
	//cout<<"SH "<<endl;
	if (signo == SIGINT && fg_proc!=-1)
		kill(-fg_proc,SIGINT);
	
}

void sig_handler_child(int signo){					// signal handler for background client reaper proc
	///cout<<"SH1 "<<bg_proc<<endl;
	if (signo == SIGINT && bg_proc!=-1)
		kill(-bg_proc,SIGINT);
	signal_passed=1;
	while(waitpid((pid_t)-1,0,WNOHANG)){};
	exit(0);
}

vector<int> bgprocs;

void TheReaper(){									// reaps zombie background reaper procs
	///cout<<bgprocs.size()<<endl;
	for(int i=0;i<bgprocs.size();i++){
		if(waitpid(bgprocs[i],0,WNOHANG)){
			bgprocs.erase(bgprocs.begin()+i);
			i=-1;
		}
	}
	if(bgprocs.size()==0) bg_proc=-1;
}
		

vector<string> tokenize(string line){				// modified tokenizer(handles spaces with backslash)
	vector<string> tokens;
	string temp="";
	for(int i =0; i < line.size(); i++){
		if (line[i]== ' ' || line[i] == '\n' || line[i] == '\t'){
				tokens.push_back(temp);
				temp="";
		}
		else {
			if(line[i]=='\\' && line[i+1]==' '){
				temp+=" ";
				i++;
			}
			else{
				temp+=line[i];
			}
		}
	}
	tokens.push_back(temp);
	return tokens;
}

//cd
int handle_cd(vector<string> cmd_tokens){			
    if(cmd_tokens.size()!=2){
        return -2;
    }
    //if from root
    if(cmd_tokens[1][0]=='/'){
        const char *directory = cmd_tokens[1].c_str();
        int ret = chdir(directory);
        return ret;
    }
    
    if(cmd_tokens[1][0]=='~'){
        cout<<"Please specify directory from root or cwd"<<endl;
        return -1;
    }

    //if from cwd
    char cwd[1024];
    getcwd(cwd, sizeof(cwd));
    string cwd_str(cwd);
    cmd_tokens[1]=cwd_str+'/'+cmd_tokens[1];
    const char *directory = cmd_tokens[1].c_str();
    int ret = chdir(directory);
    return ret;
}

//ls cat echo grep
int handle_ls_cat_echo_grep(vector<string> cmd_tokens){
    pid_t my_pid, parent_pid, child_pid;
    int status;
    if((child_pid = fork()) < 0 ){
        perror("fork failure");
        exit(1);
    }
    if(child_pid == 0){
        char* argv_v[cmd_tokens.size()+1];
        for(int i=0;i<cmd_tokens.size();i++){
            argv_v[i]=strdup(cmd_tokens[i].c_str());;
        }
        argv_v[cmd_tokens.size()]=NULL;
        if(cmd_tokens[0]=="ls") execv("/bin/ls",argv_v);
        if(cmd_tokens[0]=="cat") execv("/bin/cat",argv_v);
        if(cmd_tokens[0]=="echo") execv("/bin/echo",argv_v);
        if(cmd_tokens[0]=="grep") execv("/bin/grep",argv_v);
        _exit(1);
    }
    else{
        wait(&status);
    }
    return 0;
}



int main(int argc,char** argv)
{
	signal(SIGINT, sig_handler);
	string line;     
	while (1) {           
	
		cout<<"Hello>";
		TheReaper();     
		getline(cin,line);           
		///cout<<line<<endl;
		vector<string> tokens=tokenize(line);
		
		//server
		if(tokens[0]=="server"){
			if(tokens.size()==3){
				server=tokens[1];
				port=atoi(tokens[2].c_str());
				///cout<<server<<" "<<port<<endl;
			}
			else cout<<"ERROR : Correct usage --> server <server-ip> <port>"<<endl;
		}
		
		//getfl
		else if(tokens[0]=="getfl" && tokens.size()==2){
			if(tokens.size()==2){
				if(port!=-1){
					int pid=fork();
					if(pid<0)	cout<<"ERROR : Unable to create a new process"<<endl;
					else if(pid==0){
						execl("./get-one-file-sig","get-one-file-sig",tokens[1].c_str(),server.c_str(),to_string(port).c_str(),"display",(char*)0);
					}
					else{
						if(fg_proc==-1) fg_proc=pid;
						setpgid(pid,fg_proc);
						waitpid(pid,0,0);
					}
				}
				else{
					cout<<"ERROR : Server/Port not specified. Usage --> server <server-ip> <port>"<<endl;
				}
			}
			else cout<<"ERROR : Correct usage --> getfl <filename>"<<endl;
		
		}
		
		//getfl with output redirection
		else if(tokens[0]=="getfl" && tokens[2]==">"){
			if(tokens.size()==4){
				if(port!=-1){
					int pid=fork();
					if(pid<0)	cout<<"ERROR : Unable to create a new process"<<endl;
					else if(pid==0){
						FILE* f=fopen(tokens[3].c_str(),"w");
						int fd=fileno(f);
						if(fd<0) {cout<<"Unable to open file"<<endl; break;}
						dup2(fd,1);
						execl("./get-one-file-sig","get-one-file-sig",tokens[1].c_str(),server.c_str(),to_string(port).c_str(),"display",(char*)0);
					}
					else{
						if(fg_proc==-1) fg_proc=pid;
						setpgid(pid,fg_proc);
						waitpid(pid,0,0);
					}
				}
				else{
					cout<<"ERROR : Server/Port not specified. Usage --> server <server-ip> <port>"<<endl;
				}
			}
			else cout<<"ERROR : Correct usage --> getfl <filename>  '>'  <out_filename> "<<endl;
		}
		
		
		//PIPE
		else if(tokens[0]=="getfl" && tokens[2]=="|"){
            if(tokens.size()>=4){
                if(port!=-1){
					int pipefd[2];
                    pipe(pipefd);

                    int ip_pid;
                    int op_pid;
                    
                    if((ip_pid=fork())==0){
                        dup2(pipefd[1],1);
                        close(pipefd[0]);
                        close(pipefd[1]);
                        execl("./get-one-file-sig","get-one-file-sig",tokens[1].c_str(),server.c_str(),to_string(port).c_str(),"display",(char*)0);
                    }
                    if((op_pid=fork())==0){
                        dup2(pipefd[0],0);
                        close(pipefd[0]);
                        close(pipefd[1]);
                        
                        char* argv_v[tokens.size()-2];
						for(int i=3;i<tokens.size();i++){
							argv_v[i-3]=strdup(tokens[i].c_str());
						}
						
						argv_v[tokens.size()-3]=NULL;
						if(file_exists("/bin/"+tokens[3]))
							execv(("/bin/"+tokens[3]).c_str(),argv_v);
						else{
							cout<<"Invalid bin command after pipe"<<endl;
							exit(0);
						}
                    }
                    else {
                        close(pipefd[0]);
                        close(pipefd[1]);
                        waitpid(ip_pid,0,0);
                        ///cout<<"LHS done!"<<endl;
                        waitpid(op_pid,0,0);
                        ///cout<<"RHS done!"<<endl;
                    }
                }
                else cout<<"ERROR : Server/Port not specified. Usage --> server <server-ip> <port>"<<endl;
            }
            else cout<<"ERROR : Correct usage --> getfl <filename>  '|'  <cmd> "<<endl;
        }
		
		
		
		//getsq
		else if(tokens[0]=="getsq"){
			if(tokens.size()>1){
				if(port!=-1){
					for(int i=1;i<tokens.size();i++){
						int pid=fork();
						if(pid<0)	cout<<"ERROR : Unable to create a new process"<<endl;
						else if(pid==0){
							execl("./get-one-file-sig","get-one-file-sig",tokens[i].c_str(),server.c_str(),to_string(port).c_str(),"nodisplay",(char*)0);
						}
						else{
							if(fg_proc==-1) fg_proc=pid;
							setpgid(pid,fg_proc);
							waitpid(pid,0,0);
						}
		
					}
				}
			
				else{
					cout<<"ERROR : Server/Port not specified. Usage --> server <server-ip> <port>"<<endl;
				}
			}
			else cout<<"ERROR : There must be atleast 1 file "<<endl;
		}
		
		
		//getpl
		else if(tokens[0]=="getpl"){
			if(tokens.size()>1){
				if(port!=-1){
					vector<int> v_pid;
					for(int i=1;i<tokens.size();i++){
							int pid=fork();
							if(pid<0)	cout<<"ERROR : Unable to create a new process"<<endl;
							else if(pid==0){
								execl("./get-one-file-sig","get-one-file-sig",tokens[i].c_str(),server.c_str(),to_string(port).c_str(),"nodisplay",(char*)0);
							}
							else{
								v_pid.push_back(pid);
								if(fg_proc==-1) fg_proc=pid;
								setpgid(pid,fg_proc);
								///if(i!=tokens.size()-1){	while(waitpid((pid_t)-1,0,WNOHANG)){};	}
								///else waitpid(pid,0,0);
							}
			
						}
					for(int i=0;i<v_pid.size();i++)	waitpid(v_pid[i],0,0);
					}
			
				else{
					cout<<"ERROR : Server/Port not specified. Usage --> server <server-ip> <port>"<<endl;
				}
			}
			else cout<<"ERROR : There must be atleast 1 file "<<endl;
		}
		
		
		//getbg
		else if(tokens[0]=="getbg"){
			if(tokens.size()==2){
				if(port!=-1){
					int pid=fork();
					if(pid<0)	cout<<"ERROR : Unable to create a new process"<<endl;
					else if(pid==0){
						
						
						signal(SIGINT, sig_handler_child);
							
						///while(waitpid((pid_t)-1,0,WNOHANG)){};
						int pid1=fork();
						if(pid1<0)	cout<<"ERROR : Unable to create a new process"<<endl;
						else if(pid1==0){
							
							execl("./get-one-file-sig","get-one-file-sig",tokens[1].c_str(),server.c_str(),to_string(port).c_str(),"nodisplay",(char*)0);
						}
						else{
							if(bg_proc==-1) bg_proc=pid1;
							setpgid(pid1,bg_proc);
							signal(SIGINT, sig_handler_child);
							waitpid(pid1,0,0);
							if(!signal_passed)	cout<<"File "<<tokens[1]<<" downloaded successfully!\n";
							exit(0);
						}
					}
					else{
						bgprocs.push_back(pid);
						if(bg_proc==-1) bg_proc=pid;
						setpgid(pid,bg_proc);
					}
					
				}
				else{
					cout<<"ERROR : Server/Port not specified. Usage --> server <server-ip> <port>"<<endl;
				}
			}
			else cout<<"ERROR : Correct usage --> getfl <filename>  '>'  <out_filename> "<<endl;
		}
		
		
		//cd
        else if(tokens[0]=="cd"){
            int ret=handle_cd(tokens);
            if(ret==-2)	fprintf(stderr, "Error invalid command line arguments\n");
            if(ret==-1)	fprintf(stderr, "Error Directory not available\n");
        }
        
        //ls cat echo grep
        else if(tokens[0]=="ls"||tokens[0]=="cat"||tokens[0]=="echo"||tokens[0]=="grep"){
            int ret=handle_ls_cat_echo_grep(tokens);
        }
        
		//exit
		else if(tokens[0]=="exit"){
			///cout<<bg_proc<<endl;
			if(bg_proc!=-1)	kill(-bg_proc,SIGINT);
			while(waitpid((pid_t)-1,0,WNOHANG)>0){}
			exit(1);
		}
		
		//Others
		else{
			if(line!="")	cout<<line<<": command not found"<<endl;
			///cout<<"|"<<tokens[0]<<"|command not found"<<endl;
		}
		
		
		
	}
     

}

                

