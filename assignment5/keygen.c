#include <stdio.h>
#include <string.h>
#include <time.h>
#include <stdlib.h>

/******************
*  Author: Hannah Maung
*  Date: 6-1-21
*  Homework 5
*  File name: keygen
*  Description: This program creates a key file of specified length. The characters in the file generated will be any of the 27 allowed characters, generated using the standard Unix randomization methods
*  Main source: module codes
*******************/


int main(int argc, char* argv[]) {


	//initializing variables
	int length = atoi(argv[1]);
	char* str;


	//for random generator
	srandom(time(NULL));

	//checking args
	if (argc != 2) {
		printf("Error, you need at least two arguments");
		exit(0);
	}

	else {

		//converting string to numbers using atoi
		str = (char*)malloc(sizeof(char) * (length + 1));

		//array filled with 26 capital letters and a space
		static const char alphabet_char[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZ ";
		int i;
		for (i = 0; i < length; i++) {
			//using rand(), assigning each character in str a random character out of 27 (26 capital letters and the space character) 
			str[i] = alphabet_char[rand() % 27];
		}

		str[length] = '\0';
		printf("%s\n", str);

	}

}


