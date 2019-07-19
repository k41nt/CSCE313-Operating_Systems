#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <string.h>
#include <fcntl.h>
#include <string.h>
#include <stdbool.h>

char *cmd_array[100];

char * readCmd ( char *c ){//read the command line
	char *temp = c;
    while ((*temp = getchar()) != '\n'){//temp goes through cmd until \n
    	temp++;
    }
    *temp=0;
    return c;
}

int redirect(char * arg){// argument after cd
	if (arg == NULL){// if theres nothing after cd
		chdir(getenv("HOME"));
	}
	else{
		char* c= "-";// "-" after cd
		if(strcmp(c,arg) == 0){
			printf("Went up by one level directory\n");
			chdir("..");// This function goes up by level dir
			return 0;
			}
		else if(chdir(arg) == -1){
			printf("Error: Wrong directory path\n");
			return -1;
			}
		}
		return 0;
	}

void parse(char * command, char ** cmd_array){//parses some special characters
	while (*command != '\0' && *command != '>' && *command != '<'){
		while (*command == ' '){
			*command++ = '\0';
		}
		*cmd_array++ = command;
		while (*command != '\0' && *command != ' ' && *command != '\n'){
			command++;
	}
	*cmd_array = '\0';}
}

int exec(char* cmd, char** cmd_array, int flag, char* inf, char* outf){
	pid_t pid = fork();
	if(pid == 0){//child
		int inF = 0; //in fd is 0
		int outF = 1; // out fd is 1
		if (flag==1){ //input from file
			    if(inf == NULL) printf("Error: Couldn't read from file!\n");
				inF = open(inf, O_CREAT|O_WRONLY|O_TRUNC, S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH);
				dup2(inF , 0); //std in fd is 0
				close(inF);
			}
		else if (flag==2){ //output to file
				if(outf == NULL) printf("Error: Couldn't write to file!\n");
				printf("Wrote to file.\n");
				outF = open(outf, O_CREAT|O_WRONLY|O_TRUNC, S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH);
				dup2(outF , 1);
				close(outF);
			}
				execvp(cmd, cmd_array);
			}

	else if (pid>0) { //parent
		if(flag != 3) wait(NULL);// wait for the child then exit the loop to avoid zombie processes			
		}
	else if (pid == -1)printf("Child couldn'y be created.\n"); //error
	return 0;
}

int pipelining(char ** cmd_array){
	int fds1[2];
	int fds2[2];
	int num_cmds = 0;
	char *cmd[256];

	pid_t pid;


	int p = 0; //for pipelining iteration
	int l = 0;//length of array
	while(cmd_array[l] != NULL){ // find the needle "|"
		if(strcmp(cmd_array[l],"|") == 0){
			num_cmds++;
		}
		l++;
	}
	num_cmds++;//+1  because we count from second command
	//loop through the arguments
	int x = 0;//index of cmd_array
	int y = 0;//index of cmd
	int exit = 0;//if 1 exit out the loop below
	while(cmd_array[y] != NULL && exit != 1){
		x = 0;
		//store the cmd into cmd_array
		while(strcmp(cmd_array[y],"|") != 0){
			cmd[x] = cmd_array[y];
			y++;
			if(cmd_array[y] == NULL){
				exit = 1;
				x++;
				break;
			}
			x++;
		}
		cmd[x] = NULL;
		y++;

		if(p % 2 != 0) pipe(fds1);
		else pipe(fds2);

		pid=fork();
		if(pid == -1){
			if(p != num_cmds - 1){
				if(p % 2 != 0) close(fds1[1]);
				else close(fds2[1]);
			}
			printf("Error: Child process couldn't be made\n");
			return -1;
		}
		if(pid == 0){
			if(p == 0) dup2(fds2[1], 1);
			else if(p == num_cmds - 1){
				if(num_cmds % 2 != 0) dup2(fds1[0], 0);
				else dup2(fds2[0],0);
			}
			else{
				if (p % 2 != 0){
					dup2(fds2[0], 0);
					dup2(fds1[1], 1);
				}
				else{
					dup2(fds1[0], 0);
					dup2(fds2[1], 1);
				}
			}
			if (execvp(cmd[0],cmd)== -1) printf("Command not found!\n");
		}
		if(p == 0) close(fds2[1]);
		else if(p == num_cmds - 1){
			if(num_cmds % 2 != 0) close(fds1[0]);
			else close(fds2[0]);
		}
		else{
			if(p % 2 != 0){
				close(fds2[0]);
				close(fds1[1]);
			}
			else{
				close(fds1[0]);
				close(fds2[1]);
			}
		}
		waitpid(pid,NULL,0);//wait to run completely
		p++;
	}
	return 0;
}

void split(char *command){
	parse(command, cmd_array);
	char * inFS = NULL;
	char * outFS = NULL;
	int flg = 0;
	int l;// length of cmd_array
	for (l = 0 ; cmd_array[l] ; l++);
	bool exec_now = true;
	int i;
	for (i = 0 ; cmd_array[i] ; i++){
		if(strcmp(cmd_array[i], "cd") == 0){
			char * str = cmd_array[i+1];
			exec_now = false;
			redirect(str);
		}
	    if (cmd_array[i][0] == '<'){ //go to input file
	          if (i + 1 < l) inFS = cmd_array[i + 1];// get filename
	          flg = 1;
	          cmd_array[i] = NULL;
	          i++; //increment becase theres filename
	      }
      if (cmd_array[i][0] == '>') {//output to file
          if (i + 1 < l) outFS = cmd_array[i + 1]; //get filename
          flg = 2;
          cmd_array[i] = NULL;
          i++; //increment because there's filename
      }
			if(cmd_array[i][0] == '&'){//background process
				flg = 3;
				cmd_array[i] = NULL;
				i++;
				l--;
			}
			if(cmd_array[i][0] == '|'){//pipelining
				exec_now = false;
				pipelining(cmd_array);
				return;
			}
		}
		if(exec_now==true) {
			exec(command, cmd_array, flg, inFS, outFS);}
	}
//////////////////////////////////****MAIN****///////////////////////////////////////////////////////////////////
	int main(int argc, char ** argv) {
	char command[100]; // max length of command is 100
	printf("This is a unix shell program:\n");
	while(1){//infinity loop to keep shell running
		printf("my_shell$>> ");
		readCmd(command);
		while (command[0] == '\0'){//get back to promt if nothing was entered
			printf("my_shell$>> ");
			readCmd(command);
		}
		split(command);
		if(strcmp(cmd_array[0], "exit") == 0) {
			exit(0); // if "exit" is typed, exit shell
			printf("Shell terminated.\n");
		}
		if(strcmp(cmd_array[0], "clear") == 0) system("clear"); // clear 
	}
	return 0;
}