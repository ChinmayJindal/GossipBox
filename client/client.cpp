#include "client.h"

int setupConnection(in_port_t serverPort, char* serverIP){
	
	// setup server address
	struct sockaddr_in serv_addr;
	memset(&serv_addr, '0', sizeof(serv_addr));	
	serv_addr.sin_family = AF_INET;				// AF_INET corresponds to IPv4
	serv_addr.sin_port = htons(serverPort);		// htons converts port no to network byte order which is big endian
	// convert readable IP into struct sockaddr_in packageable form
	// pton means printable to network
	int res = inet_pton(AF_INET, serverIP, &serv_addr.sin_addr.s_addr);
	if(res<=0){
		if(res==0)
			printf("Invalid IP address !!\n");
		else
			printf("inet_pton() failed\n");
	
		return -1;
	}

	// create a socket
	int sd = socket(AF_INET, SOCK_STREAM, 0);
	if (sd < 0){
		printf("Error Opening Socket !!\n");
		return -1;
	}

	// connect to server
	if (connect(sd, (struct sockaddr *)&serv_addr ,sizeof(serv_addr)) <0){
		printf("connection to server failed !!\n");
		return -1;
	}
	
	return sd;
}


void splitString(char* str, char delimiter, char** result){
	while(*str!='\0'){ 					// replacing white spaces with null
		while(*str==delimiter)
			*str++ = '\0';

		*result++ = str;				// argv getting arguement and moving forward
		while(*str!='\0' && *str!=delimiter) 		// traversing a word till a white space is met
			str++;

		while(*str==delimiter)
			*str++ = '\0';
	}
	*result='\0';				// adding null character at the end of last arguement
}


void sendMessage(int sockfd, char* data){
	ssize_t dataLen = strlen(data);
	ssize_t sentLen = send(sockfd, data, dataLen, 0);
	if (sentLen < 0){
		printf("Name sending failed !!\n");
		exit(-1);
	}
	else if(sentLen != dataLen){
		printf("send(): sent unexpected number of bytes\n");
		exit(-1);
	}
}


void receiveMessage(int sockfd, char*buffer, int bufSize){
	ssize_t recvLen = recv(sockfd, buffer, bufSize-1, 0);
	if (recvLen < 0){
		printf("Receive Failed !!\n");
		exit(-1);
	}
}
