#include "client.h"

int main(int argc, char **argv){

	// checking the validity of args
	if (argc < 2 || argc > 3){
		printf("<Server Address> <Server Port>");
		return -1;
	}

	char* serverIP = argv[1];
	// set user given port no or default as 1893
	in_port_t serverPort = (argc==3) ? atoi(argv[2]) : 1893;

	// getting nickname from user
	char username[MAX_USERNAME_LENGTH], *temp;
	printf("Enter your name: ");
	scanf("%[^\n]%c", username, temp);

	// get the socket descriptor for dealing with the connection
	int sd = setupConnection(serverPort, serverIP);
	if (sd==-1)
		exit(-1);

	sendMessage(sd, username);						// send username to server

	char buffer[MAX_BUF_SIZE];
	receiveMessage(sd, buffer, MAX_BUF_SIZE);		// Receive list of online clients from server
	
	char *usersList[MAX_USERS];
	splitString(buffer, ' ', usersList);

	printf("Following users are available now:\n");
	for (int i=0; usersList[i]!='\0'; ++i)
		printf("%s\n", usersList[i]);


	close(sd);
	return 0;
}