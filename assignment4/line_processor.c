#define  _GNU_SOURCE
#include <stdio.h>
#include <sys/stat.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <dirent.h> 
#include <pthread.h>
#include <sys/types.h>
#include <stdbool.h>

/*********************
* Name: Hannah Maung
* Date: 5/17/21
* Homework 4
* Program description: program that creates 4 threads to process input from standard input as follows:
* Thread 1 is the Input thread which reads in lines of characters
* Thread 2 in the Line Separator Thread that replaces every line separator in the input by a space
* Thread 3 is the Plus Sign Thread that replaces every pair of plus signs with a carrot symbol
* Thread 4 is the Output Thread that processes the data to standard output as lines of 80 characters
********************/

//Sources: https://www.geeksforgeeks.org/memset-c-example/
//Sources: all code in modules, I have specified the exact code I got it from in each function 

//Size of the buffers.
#define SIZE 10

//Max amount of characters an input line can have, including the line separator.
#define LINE 1000
//The max amount of characters that the program can output (with a line separator after each line).
#define MAX_CHAR 80

//Buffer 1, shared resource between input thread and line separator thread
char* buffer_1[SIZE];
//number of items in the buffer
int count_1 = 0;
//index where the input thread will put the next item 
int prod_idx_1 = 0;
//indes where the line separator thread will pick up the nexdt time 
int con_idx_1 = 0;
//initialize the mutex for buffer 1
pthread_mutex_t mutex_1 = PTHREAD_MUTEX_INITIALIZER;
//initialize the condition variables for buffer 1
pthread_cond_t empty_1 = PTHREAD_COND_INITIALIZER; 
pthread_cond_t full_1 = PTHREAD_COND_INITIALIZER;


//Buffer 2, shared resource between line separator thread and plus sign thread
char* buffer_2[SIZE];
//number of items in the buffer
int count_2 = 0;
//index where the line separator thread will put the next item 
int prod_idx_2 = 0;
//index where the plus sign thread will pick up the next item 
int con_idx_2 = 0;
//initialize the mutex for buffer 2
pthread_mutex_t mutex_2 = PTHREAD_MUTEX_INITIALIZER;
//initialize the condition variables for buffer 2
pthread_cond_t empty_2 = PTHREAD_COND_INITIALIZER;
pthread_cond_t full_2 = PTHREAD_COND_INITIALIZER; 

//Buffer 3, shared resource between plus sign thread and output thread 
char* buffer_3[SIZE];
//number of items in the buffer
int count_3 = 0;
//index where the plus sign thread will put the next item 
int prod_idx_3 = 0;
//index where the output thread will pick up the next item 
int con_idx_3 = 0;
//intialize the mutex for buffer 3
pthread_mutex_t mutex_3 = PTHREAD_MUTEX_INITIALIZER;
//initialize the condition variables for buffer 3
pthread_cond_t empty_3 = PTHREAD_COND_INITIALIZER;
pthread_cond_t full_3 = PTHREAD_COND_INITIALIZER;


//initialize the mutex for input output mutex (4th buffer I made)
pthread_mutex_t inout = PTHREAD_MUTEX_INITIALIZER;
//initialize the condition variables for inout mutex 
pthread_cond_t empty = PTHREAD_COND_INITIALIZER;
pthread_cond_t full = PTHREAD_COND_INITIALIZER;

//global variables 
int counter = 0;
int exit_program = 0;

//Thread 1
//Gets the input from the user, does not perform any error checking 
void *get_input(void* argument)  {

	//Sources:
	//Most of this code is inspired from the 6.5 prod_cons_pipeline.c  and 6.3 granular_sync.c in Module 6 summaries 


	int x = 1;
	//while x
	while(x) {

		//input array
		char in[LINE];
		char* end;

		//Lock the mutex before changing if the buffer has data
		pthread_mutex_lock(&inout);

		//Buffer is full. Wait for the producer to signal that the buffer has space
		while(counter == 1) {

			pthread_cond_wait(&empty,&inout);
		}
		
		//Reads the input of user
		end = fgets(in,LINE,stdin);
		
		//Signal to consumer that the buffer is no longer empty.
		pthread_cond_signal(&full);

		//unlocks the mutex
		pthread_mutex_unlock(&inout);

		//increases counter
		counter++;

		//end loop if end is null, meaning end of the line 
		if(end == NULL) {
			x = 0;
			exit_program = 1;
		}
		

		//Check if line has STOP in it 
		int stop = strcmp(in,"STOP\n");
		if(stop == 0) {
			x = 0;
			exit_program = 1;
		}
		//Lock the mutex before putting the item in the buffer.
		pthread_mutex_lock(&mutex_1);
		
		//While the buffer is full.
		while(count_1 == 1) {
			//Wait for consumer. 
			pthread_cond_wait(&empty_1,&mutex_1);
	
		}

		buffer_1[prod_idx_1] = strdup(in);
		//Increment the index where the next item will be put.
		prod_idx_1 = (prod_idx_1 + 1) % SIZE;
		count_1++;
		
		//Signal to the consumer that the buffer is no longer empty. 
		pthread_cond_signal(&full_1);


		//Unlock the mutex.
		pthread_mutex_unlock(&mutex_1);		

	}

}

//Thread 1 and Thread 2
void* input_lineseparator(void* argument) {

	//Source: Modules and example program given in the assignment.

	//creating arrays for line separator and the new line.
	char space_separator[LINE];
	char line_separator[LINE];


	//while x 
	int x = 1;
	while (x) {

		//Lock the mutex before becking if the buffer has data. 
		pthread_mutex_lock(&mutex_1);
		
		//Buffer is empty, wait for the producer to signal that the buffer has data. 
		while (count_1 == 0) {
			pthread_cond_wait(&full_1, &mutex_1);
		}


		//checking if line has STOP
		int stop = strcmp(buffer_1[con_idx_1], "STOP\n");
		if (stop == 0) {
			x = 0;
		}


		//copying space_separator to buffer 1
		strcpy(space_separator, buffer_1[con_idx_1]);

		//Increments index from which the item will be picked up,
		//rolling over to the start of the buffer if currently at the end of the buffer
		con_idx_1 = (con_idx_1 + 1) % SIZE;
		count_1--;

		//Signals to consumer that buffer is no longer empty
		pthread_cond_signal(&empty_1);

		//Unlock the mutex
		pthread_mutex_unlock(&mutex_1);



		int i;
		for (i = 0; i < LINE; ++i) {
			//checks if there is a new line (line separator), replaces it by a space 
			if (space_separator[i] == '\n') {
				space_separator[i] = ' ';
			}
		}

		//Lock mutex 2
		pthread_mutex_lock(&mutex_2);

		//Buffer is full. Wait for the consumer to signal that the buffer has space.
		while (count_2 == 1) {
			pthread_cond_wait(&empty_2, &mutex_2);
		}

		//Puts the space separator line into buffer 2
		buffer_2[prod_idx_2] = strdup(space_separator);

		//Increment the index
		prod_idx_2 = (prod_idx_2 + 1) % SIZE;
		count_2++;

		//Signal to the consumer that the buffer is no longer empty.
		pthread_cond_signal(&full_2);

		//Unlock the mutex
		pthread_mutex_unlock(&mutex_2);

		
	}

}


//Goes through string line, checks if there is a pair of plus',replaces with carrot if
char* plussign(const char* arr, const char* plus_arr, const char* carrot_arr) {
	
	//Making string for length
	//initializing variables
	int carrot_length = strlen(carrot_arr);
	int plus_length = strlen(plus_arr);
	int count;

	
	//Source: https://www.geeksforgeeks.org/c-program-replace-word-text-another-given-word/

	//Checking the number of times ++ occurs
	int i;
	for (i = 0; arr[i] != '\0'; i++) {
		//finding subset
		if (strstr(&arr[i], plus_arr) == &arr[i]) {
			//increasing count variable 
			count++;
			i += plus_length - 1;
		}
	}
	
	//making string for length 
	char* new = (char*)malloc(i + count * (carrot_length - plus_length) + 1);
	i = 0;
	while (*arr) {
		// finding the substring 
		if (strstr(arr, plus_arr) == arr) {
			strcpy(&new[i], carrot_arr);
			i += carrot_length;
			arr += plus_length;
		}
		else
			new[i++] = *arr++;
	}

	new[i] = '\0';
	return new;
}


//Thread 2 and 3
void* line_separator_plussign(void* argument) {

	//creating arrays for thread 2 and thread 3
	char plus_line[LINE];
	char carrot[LINE];

	//while x 
	int x = 1;
	while (x) {
		
		//Lock the buffer
		pthread_mutex_lock(&mutex_2);

		//Buffer is empty. Wait for the producer to signal that the buffer has data.
		while (count_2 == 0) {
			pthread_cond_wait(&full_2, &mutex_2);
		}

		//copying plus_line to buffer_2[con_idx_2]
		strcpy(plus_line, buffer_2[con_idx_2]);

		//Increment the index from which the item will be picked up.
		con_idx_2 = (con_idx_2 + 1) % SIZE;
		count_2--;

		//Signals to consumer that buffer is no longer empty
		pthread_cond_signal(&empty_2);

		//Unlock the mutex
		pthread_mutex_unlock(&mutex_2);

		//Checking if there is a "STOP" 
		int stop = strcmp(plus_line, "STOP\n");
		if (stop == 0) {
			x = 0;
		}

		//creating arrays for plus symbols and carrots to put in as arguments to call plussign() function 
		char p[] = "++";
		char c[] = "^";

		//copying carrot to plussign(plus_line), calling plussign() function 
		strcpy(carrot, plussign(plus_line,p,c));

		//Locks the mutex before checking if buffer has data
		pthread_mutex_lock(&mutex_3);

		//Buffer is full. Wait for the consumer to signal that the buffer has space.
		while (count_3 == 1) {
			pthread_cond_wait(&empty_3, &mutex_3);
		}

		//carrot line into buffer 3
		buffer_3[prod_idx_3] = strdup(carrot);

		//Increment the index where the item will be put next.
		prod_idx_3 = (prod_idx_3 + 1) % SIZE;
		count_3++;
		
		//Signals to the consumer that the buffer is no longer empty
		pthread_cond_signal(&full_3);

		//Unlocks the mutex
		pthread_mutex_unlock(&mutex_3);

	}
}


//exit program function 
void exit_function() {

	//if global variable exit_program
	if (exit_program) {
		//exit the program 
		exit(0);
	}

}

void* output_thread(void* argument) {

	//Sources:
	//Most of this code is inspired from the 6.5 prod_cons_pipeline.c  and 6.3 granular_sync.c in Module 6 summaries 


	//Makes array for output
	char out[LINE];

	//variables for all lines and the output lines 
	int lines_all;
	int lines_o;

	int x = 1;
	//while x
	while (x) {

		pthread_mutex_lock(&mutex_3);

		//Buffer is full. Wait for the producer to signal that the buffer has space.
		while (count_3 == 0) {

			pthread_cond_wait(&full_3, &mutex_3);
		}


		//Check if line has STOP in it. 
		int stop = strcmp(buffer_3[con_idx_3], "STOP\n");
		if (stop == 0) {
			x = 0;
			
		}

		//concatenates out and buffer_3[con_idx_3]
		//the resultant string is stored in out.
		strcat(out, buffer_3[con_idx_3]);

		//Increment the index from which the item will be picked up, rolling over 
		//to the start of the buffer if currently at the end of the buffer.
		con_idx_3 = (con_idx_3 + 1) % SIZE;
		count_3--;

		//Signal to the producer that the buffer has space. 
		pthread_cond_signal(&empty_3);

		//Unlock the mutex.
		pthread_mutex_unlock(&mutex_3);



		//Locks the input output mutex, the fourth mutex that I made
		pthread_mutex_lock(&inout);

		//Buffer is full. 
		while (counter == 0) {
			pthread_cond_wait(&full, &inout);
		}
		
		//decrease counter variable
		counter--;

		int i, j;
		//setting array to 81 characters because line is 80 characters
		char arr[MAX_CHAR + 1];
		//setting num to the string length of out
		int num = strlen(out);
		//as long as num is greated than 80
		if (num >= MAX_CHAR) {

			lines_o = strlen(out) / MAX_CHAR;
			for (i = lines_all; i < lines_o; ++i) {
				//fill whole array with null termination character to mark end of the string
				memset(arr, '\0', sizeof(arr));
				//as long as j is less than 80
				for (j = 0; j < MAX_CHAR; ++j) {
					int index = i * MAX_CHAR + j;
					arr[j] = out[index];
				}
				//prints out the line 
				printf("%s\n", arr);
			}

	
			lines_all = lines_o;
			//writes any unwritten data in stream's buffer
			fflush(NULL);

		}

		//Signal to the producer that the it is no longer empty
		pthread_cond_signal(&empty);

		//Unlock the mutex 
		pthread_mutex_unlock(&inout);

		//calls exit function to exit out of the program 
		exit_function();


	}
	return NULL;
}



int main() {


	//Source: modules (Example program given to us in the assignment) 

	pthread_t input_t, line_separator_t, plus_sign_t, output_t;

	//Create the producer threads
	pthread_create(&input_t, NULL, get_input, NULL);
	pthread_create(&line_separator_t, NULL, input_lineseparator, NULL);
	pthread_create(&plus_sign_t, NULL, line_separator_plussign, NULL);
	pthread_create(&output_t, NULL, output_thread, NULL);


	//Now create the consumer thread
	pthread_join(input_t,NULL);
	pthread_join(line_separator_t, NULL);
	pthread_join(plus_sign_t, NULL);
	pthread_join(output_t, NULL);


	return EXIT_SUCCESS;

}
