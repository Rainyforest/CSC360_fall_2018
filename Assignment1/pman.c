/*------------------------
 * University of Victoria
 * CSC360  Assignment1
 * Jue Fu
 * V00863998
 -----------------------*/

/* ---------- Commands ----------
 *bg->fork(), exec*()
 *bglist
 *bgkill
 *bgstop
 *bgstart
 *pstat
-------------------------------*/
#include <stdio.h>
#include <unistd.h>                 // fork(), exec*()
#include <stdlib.h>                 // malloc()
#include <sys/types.h>              // pid_t
#include <string.h>                 // string concatenation
#include <sys/wait.h>               // waitpid()
#include <readline/readline.h>      // readline
#include <regex.h>                  // regular expression to read status file
#include "linkedlist.h"             // linkedlist


#define LINE_LEN 500                // max char number of a line
#define MAX_ARG_NUM 50              // max argument number of a line
#define MAX_ARG_LEN 50              // max char number of an argument
#define MAX_FILE_LEN 1000           // max length of a file
p_node* plist= NULL;                // initialize process list
int arg_count= 0;                   // argument counter for input line

void bg(char** inputList){
  char* name = inputList[1];
  pid_t pid=fork();        // The ID of child process
  char path[2+MAX_ARG_LEN] = "./";
  strcat(path,name);

  /* Child process */
	if(pid == 0){
    int i=0;
    char *argv_execvp[arg_count];
    argv_execvp[0] = path;
    argv_execvp[arg_count-1]=NULL;
    for(i=1;i<arg_count-1;i++){
      argv_execvp[i] = inputList[i+1];

    }
    // int j=0;
    // for(j=0;j<arg_count;j++){
    //   printf("%d:(%s)\n",j,argv_execvp[j]);
    // } /* For testing argv_execvp[] */
		if (execvp(path, argv_execvp) < 0)
			perror("Error on execvp");
  }

  /* Parent process */
	else if(pid > 0){  		//parent process
		printf("PID: %d New process created.\n",pid);
    p_node* p=newNode(name,pid,1);
    plist =append(p,plist);
	}
	else{  // Fail to create new process
		perror("\nFail to create a new process.\n");
	}
	//setbuf(stdin, NULL);  // clear the buffer. You could try to remove this line to see the result
}
void bglist(){
  if(plist == NULL)perror("No process existing.\n");
  p_node* temp = plist;
  int counter=0;
  char exe_path[LINE_LEN];
  while(temp!=NULL){
    getcwd( exe_path, sizeof( exe_path ) );
    printf("%d: \t", temp->pid);
    printf("%s",  exe_path);
    printf("/%s \t", temp->name);
    printf("%s\n",((temp->active>0)?"RUNNING":"STOPPED"));
    temp=temp->next;
    counter++;
  }
  printf("Total background jobs: %d\n",counter);
}
void bgkill(pid_t pid){
  p_node* p;
  if((p = findById(pid,plist))!=NULL){
    int feedback = kill(pid, SIGTERM);
    if(feedback==0){
      plist = deleteById(pid,plist);
      sleep(1);
    }
      else printf("%s\n","Fail to kill process.");
  }else printf("Pid invalid, try again.");
}
void bgstop(pid_t pid){
  p_node* p;
  if((p = findById(pid,plist))!=NULL){
    int feedback = kill(pid, SIGSTOP);
    if(feedback==0){
      p->active = 0;
      sleep(1);
    }
      else printf("%s\n","Fail to stop process.");
  }else printf("Pid invalid, try again.");
}
void bgstart(pid_t pid){
  p_node* p;
  if((p = findById(pid,plist))!=NULL){
    int feedback = kill(pid, SIGCONT);
    if(feedback==0){
      p->active = 1;
      sleep(1);
    }
      else printf("%s\n","Fail to resume process.");
  }else printf("Pid invalid, try again.");
}
void pstat(pid_t pid){
  regex_t reg_obj;
  char comm[LINE_LEN]="";
  char state[LINE_LEN]="";
  char voluntary_ctxt_switches[LINE_LEN]="";
  char nonvoluntary_ctxt_switches[LINE_LEN]="";
  char u_time[LINE_LEN];
  char s_time[LINE_LEN];
  char rss[LINE_LEN];
  p_node* p;
  if((p = findById(pid,plist))!=NULL){
    char status_path[LINE_LEN];
    char stat_path[LINE_LEN];

    sprintf(status_path, "/proc/%d/status", pid);
    sprintf(stat_path,"/proc/%d/stat",pid);

    /* Read the /proc/status file */
    FILE *f_status=fopen(status_path,"r");
    char line[LINE_LEN];
    while (fgets(line, sizeof(line), f_status)) {

      /* Use reg exp to match lines while traversing the file */
      regcomp (&reg_obj, "^Name:.*$",REG_NEWLINE);
      if(!regexec(&reg_obj,line,0,NULL,0)){
        strncpy(comm, line, LINE_LEN);
      }
      regfree(&reg_obj);
      regcomp (&reg_obj, "^State:.*$",REG_NEWLINE);
      if(!regexec(&reg_obj,line,0,NULL,0)){
        strncpy(state, line, LINE_LEN);
      }
      regfree(&reg_obj);
      regcomp (&reg_obj, "^voluntary_ctxt_switches:.*$",REG_NEWLINE);
      if(!regexec(&reg_obj,line,0,NULL,0)){
        strncpy(voluntary_ctxt_switches, line, LINE_LEN);
      }
      regfree(&reg_obj);
      regcomp (&reg_obj, "^nonvoluntary_ctxt_switches:.*$",REG_NEWLINE);
      if(!regexec(&reg_obj,line,0,NULL,0)){
        strncpy(nonvoluntary_ctxt_switches, line, LINE_LEN);
      }
    }
    fclose(f_status);
    FILE *f_stat=fopen(stat_path,"r");
    char stream[MAX_FILE_LEN];

    /* Read the /proc/stat file */
    if(fgets (stream, MAX_FILE_LEN, f_stat)!=NULL) {
      int token_counter=1;
      char* token = strtok(stream," ");
      while(token!=NULL){
        if(token_counter==14)strncpy(u_time,token,10);
        if(token_counter==15)strncpy(s_time,token,10);
        if(token_counter==24)strncpy(rss,token,10);
    		token = strtok(NULL, " ");
        token_counter++;
    	}
    }

    char* p;
    double utime = strtoul(u_time, &p, 10) / sysconf(_SC_CLK_TCK);
    double stime = strtoul(s_time, &p, 10) / sysconf(_SC_CLK_TCK);
    printf("%s", comm);
		printf("%s", state);
		printf("utime:\t%lf\n", utime);
		printf("stime:\t%lf\n", stime);
		printf("rss:\t%s\n", rss);
		printf("%s", voluntary_ctxt_switches);
    printf("%s", nonvoluntary_ctxt_switches);
    fclose(f_stat);
  }else printf("Error: Process %d does not exist.",pid);

}

/* Get and parse a line from input to process list */
void readLine(char** inputList){
  arg_count=0;
  char* inputLine;
  /*https://stackoverflow.com/questions/6727171/using-readline-for-completion*/
  inputLine=readline("PMan: > ");
  char* token = strtok(inputLine," ");
  int i=0;
  while(token!=NULL){
    arg_count++;
		inputList[i++] = token;
		token = strtok(NULL, " ");
	}
  /*Reference: https://www.tutorialspoint.com/c_standard_library/c_function_strtok.htm*/
}

/* Identify the input command and execute them */
void executeFunction(char** inputList){
  char function[7];
  strcpy(function,*inputList);
  if(strncmp(function,"bg",4)==0){
    bg(inputList);
  }
  else{
    pid_t pid = atoi(inputList[1]);
    if(strncmp(function,"bglist",6)==0){
      bglist();
    }
    else if(strncmp(function,"bgkill",6)==0){
      bgkill(pid);
    }
    else if(strncmp(function,"bgstop",6)==0){
      bgstop(pid);
    }
    else if(strncmp(function,"bgstart",7)==0){
      bgstart(pid);
    }
    else if(strncmp(function,"pstat",5)==0){
      pstat(pid);
    }else{
      printf("PMan: > %s:  command not found\n",*inputList);
    }
  }
}

/* Update the status of background process */
void updateBackground(){
  int p_status;
  while(1){
    pid_t pid = waitpid(-1, &p_status, WCONTINUED | WNOHANG | WUNTRACED);

    if(pid>0){
      if (WIFSIGNALED(p_status)) {
          printf("Process %d was killed\n", pid);
          deleteById(pid,plist);
      }
      if (WIFEXITED(p_status)) {
          printf("Process %d exits\n", pid);
          deleteById(pid,plist);
      }
      if (WIFSTOPPED(p_status)) {
    			printf("Process %d was stopped.\n", pid);
    			p_node* p = findById(pid,plist);
    			p->active = 0;
    	}
    	if (WIFCONTINUED(p_status)) {
      		printf("Process %d was resumed.\n", pid);
      		p_node* p = findById(pid,plist);
      		p->active = 1;
      }
    }else break;
  }
}
int main(){
  char *inputLine[MAX_ARG_NUM];

  /* Keep reading commands and execute them */
  while(1){
      readLine(inputLine);
      executeFunction(inputLine);
      updateBackground();
      sleep(3);
  }
  return 0;
}
