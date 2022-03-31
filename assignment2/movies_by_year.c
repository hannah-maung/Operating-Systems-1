#define _GNU_SOURCE
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include <dirent.h>
#include <time.h>


/*************************
 * Author: Hannah Maung
 * Date: 4/20/21
 * Program: assignment 2 Files and Directories 
 * Decription: this program reads directory entries, finds a file in the current directory based on the user input, reads and parses the data, creates a directory, and creates new files in the newly created directyr and writes processed data into those files. 
***********************/


//Used my code from assignment 1:
//struct for movie function 
struct movie {

	char *title;
	int year;
	char *language;
	float rating;
	struct movie *next;

};

////parse current line which is space delimited and create a movie struct with the data in this line
//////float and int variables do not need to allocate memory, only char variables 
struct movie *createMovie(char *currLine){

	
	struct movie *currMovie = malloc(sizeof(struct movie));

	char *saveptr;
	
	//The first token is the title
	char *token = strtok_r(currLine, ",", &saveptr);
	currMovie->title = calloc(strlen(token) +1, sizeof(char));
	strcpy(currMovie->title, token);
	//The second token is the year
	token = strtok_r(NULL, ",", &saveptr);
	currMovie->year = atoi(token);
	//The third token is the language
	token = strtok_r(NULL, ",", &saveptr);
	currMovie->language = calloc(strlen(token) +1, sizeof(char));
	strcpy(currMovie->language, token);
	//The last token is the rating 
	token = strtok_r(NULL, " ", &saveptr);
	currMovie->rating = atof(token);
	//sets the next node to NULL in the newly created movie entry
	currMovie->next = NULL;
	return currMovie;
}
//returns a linked list of movies by parsing data from each line of the file 
struct movie *processFile(char *filePath) {
	//opens the file for reading only 
	FILE *movieFile = fopen(filePath, "r");
	char *currLine = NULL;
	size_t len = 0;
	size_t nread;
	char *token;

	int count;
	//declares head and tail of the linked list
	struct movie *head = NULL;
	struct movie *tail = NULL;
	//reads the file line by line 
	while ((nread = getline(&currLine, &len, movieFile)) != -1) {
		//gets a new movie node corresponding to the current line 
		struct movie *newNode = createMovie(currLine);
		//if this is the first node in the linked list then..
		if (head == NULL) {
			head = newNode;
			tail = newNode;
	
		}
		//not the first node, adds this node to the list and advances the tail
		else{
			tail->next = newNode;
			tail = newNode;
		}
		count++;
	}

	free(currLine);
	fclose(movieFile);
	return head;
}


//makes directory
void make_directory() {

	//creating variabels 
        srand(time(0));
        char onidname[10000] = "maungh.movies.";
        char str[10000];
	//generates random num between 0 and 99999
        int lower = 0, upper = 99999;
        int random = (rand() % (upper - lower +1)) + lower;
	//formats data to a string
        sprintf(str,"%d", random);
	//concatenates strings 
        char *dirname = strcat(onidname,str);
	//makes directory with read,write and execute permissions
        mkdir(dirname,0750);

	//prints out directory name 
        printf("Created directory with name maungh.movies.%d\n",random);
	//changes into the directory
        chdir(dirname);
}

//finds the movies released in each year in the chosen file 
void find_movies_by_year(struct movie *list) {

	//calls make directory which opens the directory and makes it first 
	make_directory();
	
	int y = 0;
	int year = 1000;
	//for loop going for years
        for (year; year <= 2050; year++) {
                struct movie *this = list;
		//as long as the list is not empty
                while (this != NULL) {
			//checks if the year in the list is = to the year count
                        if (this->year == year) {
				//if it is then..
				if(this->year == y) {
					//combining the year of the movie and the.txt 
					char str[50];
					char text[50] = ".txt";
					sprintf(str,"%d",year);
					//combining strings
					char* newFilePath = strcat(str,text);
					//opens the file and writes the movie titles of that year into it 
					FILE *f;
					f = fopen(newFilePath,"a");
					fprintf(f,"%s\n",this->title);
					//closes file
					fclose(f);
					//moves onto the next in the list
					this = this->next;
				}//if it is not equal then...
				else{
					//combining the year of the movie and the .txt string
					char str[50];
					char text[50] = ".txt";
					//combining the strings 
					sprintf(str,"%d",year);
					char* newFilePath = strcat(str,text);
					//opens the file and sets the permissions of these files to read and write to the file, while group can only read the file 
					int file = open(newFilePath, O_RDWR, 0750);	
					FILE *f;
					//writes the movie titles of that year into the file 
					f = fopen(newFilePath, "a");
					fprintf(f, "%s\n", this->title);
					//closes the file 
					fclose(f);
					y = year;
					//moves onto the next in the list
					this = this->next;
				}
				
			}
                        else{
				//moves on to the next in the list
           	             this = this->next;
                        }

                }

		
        }

	//exits current directory	
	chdir("..");
}


//https://replit.com/@cs344/35statexamplec 
//Used sample module code as inspiration for this function 
void find_largest_file(char *dir) {
 

	//opens current directory
	DIR* currDir = opendir(".");

	//creating variables
	struct dirent *aDir;
	struct stat dirStat;
	char *name;
	char *p1;
	char *p2;

	int i = 0;
	//goes through all the entries 
        while((aDir = readdir(currDir)) != NULL) {
		//get meta-data for the current entry
		stat(aDir->d_name, &dirStat);  
	        if(S_ISREG(dirStat.st_mode)) {
			//gets data for the current entry
                	stat(aDir->d_name, &dirStat);
				//checking if the file name has .csv and movies_, if it passes then choose that tile
				p1 = strstr(aDir->d_name, ".csv");
				p2 = strstr(aDir->d_name, "movies_");
				if(p1 && p2) {
				//checks which file is the largest
        	        	if(dirStat.st_size > i) {
                	   		i = (dirStat.st_size);
                   			name = (aDir->d_name);
					}		
		
                		}

           		}	
		}
	//prints out the file name 
       printf("Now processing the chosen file named %s\n", name);

	//calls data to parse, calls find_movies_by_year to write into that file
	struct movie *list = processFile(name);
	find_movies_by_year(list);
	//closes current directory
        closedir(currDir);
}


void find_smallest_file(char *dir){

	//opens the current directory
        DIR* currDir = opendir(".");
	//creating variables
        struct dirent *aDir;
        struct stat dirStat;
        char *name;
	char *p1;
	char *p2;	

	int i = 10000;
	//goes through all of the entries 
        while((aDir = readdir(currDir)) != NULL) {
		//get meta-data for the current entry
        	stat(aDir->d_name, &dirStat);
                if(S_ISREG(dirStat.st_mode)) {
			//get meta-data for the current entry
			stat(aDir->d_name, &dirStat);
			//checking if the current file name has .csv and movies_, if yes then choose that file 
			p1 = strstr(aDir->d_name, ".csv");	
			p2 = strstr(aDir->d_name, "movies_");
			if(p1&&p2) {
				//finding the smallest file in that directory
				if(dirStat.st_size <= i) {
					i = (dirStat.st_size);
                	      	        name = (aDir->d_name);
				
				}
			
			}				
		}

	}	
	//prints out the file name 
        printf("Now processing the chosen file name %s\n",name);

	//parses the data, calls find_movies_by_year to write into that file 
	struct movie *list = processFile(name);
	find_movies_by_year(list);
	//choses current directory
	closedir(currDir);
}


//for prompt number 3
void enter_file_name() {

	//asks user to input their file name 
	char filename[100];
	printf("Enter the complete file name: ");
	scanf("%s",filename);

	char *p1;
	char *p2;

	//checking for name in file name
	p1 = strstr(filename, ".csv");
	p2 = strstr(filename, "movies_");
	srand(time(0));
	
	//checking if the file name the user inputted has .csv and movies_
	if(p1 && p2) {
	
		//making new directory named onid_movies.randomnumber
		char str[50];
		int lower = 0, upper = 99999;

		//generates random number between 0 and 99999
		int random = (rand() % (upper - lower +1)) + lower;
		sprintf(str,"%d",random);
		printf("%d",random);
		char *dirname = strcat(filename,str);
		//creates the directory
		mkdir(dirname,0750);
		printf("Created directory with name %s", dirname);
		
	}

	else{
		//if the user did not input a correct file name then it prints out the error message
		printf("The file %s was not found. Try again\n",filename);
	}

}



int main(void) {



	//switch statement for each case 
	int x = 0;
	while(!x) {

		//asks the user what they want to do first and saves it into a variable 
		int input1;
		int input2;
		printf("\n\n1. Select file to process\n");
		printf("2. Exit the program\n\n");
		printf("Enter a choice 1 or 2: ");
		scanf("%d",&input1);

		//exits the program if the user enters a 2
		if (input1 == 2) {
			exit(0);
		}
		//prints out error message if user enters something other than a 1 or 2
		else if ((input1 != 1) && (input1 != 2)) {

			printf("Incorrect choice, try again.");
		}
		else {
		//prompts	
		printf("\n\nWhich file do you want to process?\n");
		printf("\nEnter 1 to pick the largest file\n");
		printf("Enter 2 to pick the smallest file\n");
		printf("Enter 3 to specify the name of a file\n\n");
		printf("Enter a choice from 1 to 3: ");
		scanf("%d",&input2);
		
		switch(input2) {

			//finds largest file in directory 
			case 1:
				find_largest_file(".");
				break;
			//finds smallest file in directory
			case 2:
				find_smallest_file(".");
				break;

			case 3:
				//calls enter name function
				enter_file_name();
				break;
			//prints error message if user enters something other than the 1-3 options
			default:
				printf("Incorrect choice, try again.");
				break;
			}	


		}

	}


}
