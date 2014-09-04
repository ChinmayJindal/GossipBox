#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#define MAX_USERNAME_LENGTH 50
#define MAX_BUF_SIZE 1000
#define MAX_USERS 100

int setupConnection(in_port_t serverPort, char* serverIP);

void sendMessage(int sockfd, char* data);
void receiveMessage(int sockfd, char*buffer, int bufSize);

void splitString(char* str, char delimiter, char** result);
