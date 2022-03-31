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
*  File name: dec_server
*  Description: This program decrypts ciphertext it is given, using the passed-in ciphertext and key. Almost the same syntax and usage of enc_server.c file.
* Main source: server.c program given to us in assignment and module codes
*******************/


// Error function used for reporting issues
void error(const char* msg) {
    perror(msg);
    exit(0);
}


//Source: https://www.holbertonschool.com/coding-resource-atoi-in-c
int makeInteger(char c) {

    if (c == ' ') {
        return 26;
    }
    else {
        return (c - 'A');
    }
  
}

//Source: https://www.holbertonschool.com/coding-resource-atoi-in-c
char makeChar(int i) {

    if (i == 26) {
        return ' ';
    }
    else {
        return (i + 'A');
    }
}

// Set up the address struct for the server socket
void setupAddressStruct(struct sockaddr_in* address,
    int portNumber) {

    // Clear out the address struct
    memset((char*)address, '\0', sizeof(*address));

    // The address should be network capable
    address->sin_family = AF_INET;
    // Store the port number
    address->sin_port = htons(portNumber);
    // Allow a client at any address to connect to this server
    address->sin_addr.s_addr = INADDR_ANY;
}

int main(int argc, char* argv[])
{

    int connectionSocket, portNumber, charsRead, n, status;
    socklen_t clientLength;
    char buffer[200000];
    char keyArr[200000];
    struct sockaddr_in serverAddress, clientAddress;
    socklen_t sizeOfClientInfo = sizeof(clientAddress);
    pid_t pid;

    //Checks usage and arguments
    if (argc != 2) {

        fprintf(stderr, "Usage: %s port\n", argv[0]);
        exit(1);
    }

    //Creates the socket that will listen for connections 
    int listenSocket = socket(AF_INET, SOCK_STREAM, 0);
    if (listenSocket < 0) {
        error("ERROR opening socket");
    }

    //Sets up the address struct for the server socket 
    setupAddressStruct(&serverAddress, atoi(argv[1]));

    //Associate the socket to the port 
    if (bind(listenSocket,
        (struct sockaddr*)&serverAddress,
        sizeof(serverAddress)) < 0) {
        error("ERROR on binding");
        exit(1);

    }

    //Starts listening for connections. Allow up to 5 connections to queue up. 
    listen(listenSocket, 5);

    char* ptr = buffer;
    char* keyStart;
    int numBytes = sizeof(buffer);
    int numNewLines = 0;
    int x = 1;
    int i;

    //Accept a connection request which creates a connection socket 
    while (x) {
        connectionSocket = accept(listenSocket,
            (struct sockaddr*)&clientAddress,
            &sizeOfClientInfo);
        if (connectionSocket < 0) {
            perror("ERROR on accept");
            exit(1);
        }

        //create a new process
        pid = fork();

        //if pid is less than 0, error
        if (pid < 0) {
            error("SERVER: ERROR forking process");
        }

        //if pid value is 0
        if (pid == 0) {
            //read client's message from the socket 
            recv(connectionSocket, buffer, sizeof(buffer) - 1, 0);
            char mess[] = "dec_client";
            //verifying message that was read
            //comparing buffer to message
            if (strcmp(buffer, "dec_client") != 0) {
                char response[] = "no";
                //send message back to the client 
                send(connectionSocket, response, sizeof(response), 0);
                exit(2);
            }
            else {
                //send message back to the client 
                send(connectionSocket, mess, sizeof(mess), 0);

            }
            //get message from the client and display it
            memset(buffer, '\0', sizeof(buffer));

            while (x) {
                //read the client's message from the socket 
                charsRead = recv(connectionSocket, ptr, numBytes, 0);
                //error if charsRead is less than 0
                if (charsRead < 0) {
                    error("ERROR reading from socket");
                    exit(1);
                }
                //if charsRead is 0, break bc there is no more reading to do
                if (charsRead == 0) {
                    break;
                }
                //looking for a new line in the buffer
                for (i = 0; i < charsRead; i++) {
                    if (ptr[i] == '\n') {
                        //last char keygen outputs should be a newline 
                        numNewLines++;
                        if (numNewLines == 1) {
                            keyStart = ptr + 1 + i;
                        }
                    }
                }
                //the end, break
                if (numNewLines == 2) {
                    break;
                }
                //keeping track of characters read and the number of bytes that are left
                ptr += charsRead;
                numBytes -= charsRead;
            }

            //creating message array
            char messageArr[200000];
            memset(messageArr, '\0', sizeof(messageArr));
            //copying bytes of buffer into messageArr
            strncpy(messageArr, buffer, keyStart - buffer);

            //
            int counter;
            char character;
            //as long as there is no new line
            for (counter = 0; messageArr[counter] != '\n'; counter++) {
                //calls makeInteger() to convert from char to int 
                character = makeInteger(messageArr[counter]) - makeInteger(keyStart[counter]);
                if (character < 0) {
                    //if character is less than 0, add 27
                    character += 27;
                }
                //calls makeChar to convert int to char 
                messageArr[counter] = makeChar(character);
                
            }
            //the last keygen outputs should be a new line
            messageArr[counter] = '\0';

            //sending the new encrypted message
            send(connectionSocket, messageArr, sizeof(messageArr), 0);

        }
        //close the connection socket for this client
        close(connectionSocket);

        //parent process to wait for the children to finish 
        while (pid > 0) {
            pid = waitpid(-1, &status, WNOHANG);;
        }
    }
    //close the listening socket 
    close(listenSocket);
    return 0;
}


