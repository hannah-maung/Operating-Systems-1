#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <fcntl.h>


/******************
*  Author: Hannah Maung
*  Date: 6-1-21
*  Homework 5
*  File name: enc_client
*  Description: This program is the encryption server and will run in the background as a daemon.
* Main source: client.c program given to us in assignment and module codes
*******************/

// Error function used for reporting issues
void error(const char* msg) {
    perror(msg);
    exit(0);
}


//Source:https://www.geeksforgeeks.org/fgetc-fputc-c/
//finds specified length
int getLength(const char* filename) {

    int ch, count;
    count = 0;
    //opens file 
    FILE* file = fopen(filename, "r");

    //while true 
    while (1) {
        //gets data from file 
        ch = fgetc(file);

        //if end of line or new line, break
        if (ch == EOF || ch == '\n')
            break;
        //checks if string has capital letters or space 
        if (!isupper(ch) && ch != ' ') {
            error("File contains bad characters!\n");
        }
        //increases count variable
        ++count;
    }
    //closes file
    fclose(file);
    return count;
}


//Source: sample client.c file 
//Source: https://man7.org/linux/man-pages/man2/write.2.html
void sendData(char* filename, int socketFD, int length) {

    //initializing variables
    char* bufferPosition;
    int charsRead, charsWritten;
    //open the file , set the read only 
    int file = open(filename, 'r');
    char buffer[100000];
    memset(buffer, '\0', sizeof(buffer));
    
    //while length is greater than 0
    while (length > 0) { 
        //read file
        charsRead = read(file, buffer, sizeof(buffer));
        //done reading 
        if (charsRead == 0) { 
            break;
        }
        //cannot read file
        if (charsRead < 0) {
            error("Client: ERROR reading file");
            exit(1);
        }
        //updating length 
        length -= charsRead;
    }
   
   //whiel charsRead is greater than 0
    while (charsRead > 0) {
        bufferPosition = buffer;
        //writes to count bytes from the buffer 
        charsWritten = write(socketFD, bufferPosition, charsRead);
        //cannot write to socket 
        if (charsWritten < 0) {
            error("Client: ERROR writing to socket");
            exit(1);
        }
        //updating charsRead and the position of buffer 
        charsRead -= charsWritten;
        bufferPosition += charsWritten;
    }
    
}



int main(int argc, char* argv[]) {
    
    //initializing variables 
    int socketFD, portNumber, charsRead, charsWritten, yes;
    struct sockaddr_in serverAddress;
    struct hostent* hostInfo = gethostbyname("localhost");
    const char hostname[] = "localhost";
    //sets port num to argv[3], uses atoi to convert string argument to an int
    portNumber = atoi(argv[3]);


    char buffer[100000];
    memset(buffer, '\0', sizeof(buffer));

    //checking usage and arguments 
    if (argc != 4) {
        fprintf(stderr, "USAGE: %s hostname port\n", argv[0]);
        exit(0);
    }

    // Create a socket
    socketFD = socket(AF_INET, SOCK_STREAM, 0);
    if (socketFD < 0) {
        error("CLIENT: ERROR opening socket\n");
    }


    //checking if host is null
    if (hostInfo == NULL) {
        fprintf(stderr, "No host\n");
        exit(0);
    }

    yes = 1;
    //Source: https://beej.us/guide/bgnet/html/#setsockoptman and ED Discussion posts.
    setsockopt(socketFD, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int)); //allow reuse of port


    //Setting up the server address struct
    bzero((char*)&serverAddress, sizeof(serverAddress));
    serverAddress.sin_family = AF_INET;
    bcopy((char*)hostInfo->h_addr, (char*)&serverAddress.sin_addr.s_addr, hostInfo->h_length);
    serverAddress.sin_port = htons(portNumber);

    //Connect to the server 
    if (connect(socketFD, (struct sockaddr*)&serverAddress, sizeof(serverAddress)) < 0) {
        error("CLIENT: ERROR connecting");
    }

    //create variable for file and key length using argv arguments, calls getLength to get the length
    long fileLength = getLength(argv[1]);
    long keyLength = getLength(argv[2]);
    //Checking if file length is greater than the key length 
    if (fileLength > keyLength) {
        fprintf(stderr, "Key is too short\n");
        exit(1);
    }


    char mess[] = "enc_client";
    //send message to server, write to the server
    send(socketFD, mess, sizeof(mess),0);
    //read data from the socket
    recv(socketFD, buffer, sizeof(buffer), 0);
    //comparing buffer to message 
    if (strcmp(buffer, "enc_client") != 0) {
        fprintf(stderr, "cannot contact enc_server on this port\n");
        exit(2);
    }

    //clear out buffer array
    memset(buffer, '\0', sizeof(buffer));
    //sending file data
    sendData(argv[1], socketFD, fileLength);
    sendData(argv[2], socketFD, keyLength);

    //Get return message from server, clear out the buffer again for reuse
    memset(buffer, '\0', sizeof(buffer));
    //Reading data from the socket 
    charsRead = recv(socketFD, buffer, sizeof(buffer) - 1,0);
    if (charsRead < 0) {
        error("CLIENT: ERROR reading from socket");
    }

    printf("%s\n", buffer);

    //close the socket
    close(socketFD);
    return 0;
}