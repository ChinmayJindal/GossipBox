#ifndef _SERVER_H_
#define _SERVER_H_

#include <iostream>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <vector>
#include <pthread.h>

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#define MAX_USERNAME_LENGTH 50
#define MAX_BUF_SIZE 1000
#define MAX_USERS 100

#define DELIM ":"


extern string myname;
extern int socketDescriptor;
extern std::vector<std::string> usersList;

extern char buffer[MAX_BUF_SIZE];
extern std::vector<std::string> bufferTokens;

extern std::string cmdInput;
extern std::vector<std::string> cmdTokens;


int setupConnection(in_port_t serverPort, char* serverIP);


void *Sender(void*);
void *Receiver(void*);

void sendMessage(int sockfd, char* data);
void receiveMessage(int sockfd, char*buffer, int bufSize);

void splitCharStream(char* stream, const char* delim, int count, std::vector<std::string>* result);
void trim(std::string& s, const char tchar);

void displayOnlineUsers(std::vector<std::string> &usersList);

#endif