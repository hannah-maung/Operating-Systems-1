/************************
 * Name: Hannah Maung
 * Date: 4/29/21
 * Program: Assignment 3: smallsh Portfolio Assignment
 * Description: Writing smallsh in my own shell in C. 
**************************/

#include <sys/types.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <string.h>
#include <signal.h>
#include <sys/wait.h>

//Declaring constants 
#define MAXLENGTH 2048 
#define MAXINPUT 512

//Global Variables 
int allowBackground = 1;

//This function prompts the user and gets the command from the user then parses 
void get_input(char * commands[], char input_f[], char output_f[], int pid, int *background ) {
	
	//array for inputs, sets max of 512 arguments
	char in[MAXLENGTH];

	//prompts input and stores it 
	printf(": ");
	fflush(stdout);
	fgets(in, MAXLENGTH, stdin);

	//Removes the new line 
	int i, j;
	
	/*
	int found = 0;
	for (i=0; !found && i < MAXLENGTH; i++) {
		if (in[i] == '\n') {
			in[i] = '0';
			found = 1;
		}
	}*/

	in[strcspn(in, "\n")] = 0;

	//checking if the command is blank 
	if (!strcmp(in, "")) {
		commands[0] = strdup("");
		return;
	}

	//creating a token to parse the input string 
	char *token = strtok(in, " ");

	for(i = 0; token; i++) {
		//checking for & if command is to be executed in the background
		if (!strcmp(token, "&")) {
			*background = 1;
		}
		//checking for > character, comparing the two strings 
		else if (!strcmp(token, ">")) {
			token = strtok(NULL, " ");
			strcpy(output_f, token);
		}
		//checking for < character, comparing the two strings 
		else if (!strcmp(token, "<")) {
			token = strtok(NULL, " ");
			strcpy(input_f, token);
		}
	
		//Else it is a part of the command and do the following 
		else {
			commands[i] = strdup(token);
			//expansion of variable $$, string is converted to pid numbers
			for (j=0; commands[i][j]; j++) {
				if (commands[i][j] == '$' &&
					commands[i][j+1] == '$') {
					commands[i][j] = '\0';
					snprintf(commands[i], MAXINPUT, "%s%d", commands[i], pid);
				}
			}
		}
		//moves onto the next
		token = strtok(NULL, " ");
	}
}


void do_status(int stat);
//Sources in lecture material: 4_fork_example.c
//Exploration: Signal Handling API
void execute_command(char * commands[], char input_f[], char output_f[], struct sigaction SIGINT_action, int *background, int* stat) {

	//fork a new process
	pid_t spawnpid = -5;
	spawnpid = fork();

	//If fork is sucessful, the valud of spawnpid will be 0 in the child,
	//the child's pid in the parent
	int result;
	int sourceFD;
	int targetFD;

	
	switch (spawnpid) {
		//when fork() fails and the creation of child process fails, we do this:
		case -1:
			perror("fork() failed\n");
			exit(1);
			break;
		//when spawnpid is 0  
		case 0:
			//Fill out the SIGINT_action struct
			SIGINT_action.sa_handler = SIG_DFL;

			//Installs our signal handler
			sigaction(SIGINT,&SIGINT_action, NULL);
		
			//Source: Exploration Processes and I/O
			if(strcmp(input_f,"")){
				//opens file, sets to read only 
				sourceFD = open(input_f,O_RDONLY);
				if (sourceFD == -1) {
					perror("Error, cannot open the input file");
					exit(1);
				}
				//using dup2 to point FD
				result = dup2(sourceFD,0);
				if(result == -1) {
					perror("Error, cannot assign input file \n");
					exit(2);
				}
				//whenever this process or one of its child processes calls exec, the file descriptor will be closed in that process
				fcntl(sourceFD, F_SETFD, FD_CLOEXEC);
			}
			//same thing as above, but for the output file 
			if(strcmp(output_f,"")){
				//opens file
				targetFD = open(output_f, O_WRONLY | O_CREAT | O_TRUNC, 0644);
				if(targetFD == -1) {
					perror("output open");
					exit(1);
				}
				//using dup2 to point FD
				result = dup2(targetFD,1);
				if(result == -1) {
					perror("target dup2");
					exit(2);
				}
				//whenever this process or one of its child processes calls exec, the file descriptor will be closed in that process

				fcntl(targetFD, F_SETFD, FD_CLOEXEC);
			}

			//Execute 
			if(execvp(commands[0],(char* const*)commands)) {
				printf("%s: no such file or directory\n",commands[0]);
				fflush(stdout);
				exit(2);

			}
			break;
		//From exploration modules: process API-Monitoring Child Processes
		default:
			//if allowBackground, then execute in the background 
			if(*background && allowBackground) {
				//parent process, wait for the child to terminate
				pid_t p = waitpid(spawnpid,stat,WNOHANG);
				printf("background pid is %d\n",spawnpid);
				fflush(stdout);
			}
			else{
				pid_t p = waitpid(spawnpid, stat, 0);
			}
			//WNOHAND specified, if the child has not terminated, waitpid will return value 0
		while((spawnpid = waitpid(-1,stat, WNOHANG)) > 0) {
			//prints out the child has been terminated 
			printf("child %d terminated\n",spawnpid);
			do_status(*stat);
			fflush(stdout);	
		}			

	}

}


//Source: Modules Exploration Signal Handling API
//Our signal handler for SIGINT
void handle_SIGINT(int signo) {


	//if allowBackground variable is 0 then do the following 
	if (allowBackground == 0) {
		//print out the following message
		char* str = "Exiting foreground-only mode\n";
		//writes instead of printf
		write(1,str,29);
		fflush(stdout);
		//sets allowbackground variable to 1 
		allowBackground = 1;

	}
	//if allowBackground variable is 1
	else {
		//print out the following message	
		char *str = "Entering foreground-only mode (& is now ignored)\n";
		write(1,str,49);
		fflush(stdout);
		allowBackground = 0;
	}
	
}

//checks the background processes
void do_status(int stat) {

	int exit;
	int signal;

	//checks if child process terminated
	if(WIFEXITED(stat)) {
		//prints out exit value
		exit = WEXITSTATUS(stat);
		printf("Exit value %d\n",exit);

	}

	else{ 
		//signal number
		signal = WTERMSIG(stat);
		//prints out signal number 
		printf("Terminated by signal %d\n",signal);
	}
}

//for build in command cd, changes into and out of directories 
void do_cd(char * commands[]) {	

	if (commands[1]) {
		if(chdir(commands[1]) == -1) {
			printf("Cannot find directory\n");
			fflush(stdout);
		}
	}

	else{
		//if user does not specify specific directory, change into home directory
		chdir(getenv("HOME"));

	}
}


int main() {


	//declares variables 
	char* commands[512];
	char input_f[MAXINPUT] = "";
	char output_f[MAXINPUT] = "";
	int pid = getpid();
	int background = 0;
	int stat;
	int go = 1;

	//initialize
	int i;
	for (i = 0; i < 512; i++) {
		commands[i] = NULL;

	}

	//creating handler for contron c and control z
	struct sigaction SIGINT_action = { 0 };		
	struct sigaction SIGTSTP_action = { 0 };	

	SIGINT_action.sa_handler = SIG_IGN;
	sigfillset(&SIGINT_action.sa_mask);
	sigaction(SIGINT, &SIGINT_action, NULL);

	SIGTSTP_action.sa_handler = handle_SIGINT;	
	sigfillset(&SIGTSTP_action.sa_mask);
	sigaction(SIGTSTP, &SIGTSTP_action, NULL);	

	do{	
		//calls function get_input to prompt user for command 
		get_input(commands,input_f, output_f, pid, &background);

		//if line begins with # or blank line, do nothing
		if(strncmp(commands[0],"#",1) == 0 || strcmp(commands[0]," ") == 0) {

	        }
		//if user enters cd, call do_cd to change into and out of directories
       		else if(strcmp(commands[0],"cd") == 0) {
			do_cd(commands);
       	 	}
		//if user inputs exit, exit the program 
        	else if(strcmp(commands[0],"exit") == 0) {
			go = 0;

        	}	
		//if user enters status, call do_status() function 
        	else if(strcmp(commands[0],"status") == 0) {

                	do_status(stat);
        	}

		//if user enters nothing that is above, call execute command 
        	else{

                	execute_command(commands,input_f,output_f,SIGINT_action, &background, &stat);
		}
 		//resets variables 
		for(i=0;commands[i];i++) {

			commands[i] = NULL;
		}
		background = 0;
		input_f[0] = 0;
		output_f[0] = 0;



	}while(go);

}


