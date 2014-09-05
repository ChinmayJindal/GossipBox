#include "Client.h"


int main(int argc, char **argv){

	// checking the validity of args
	if (argc < 2 || argc > 3){
		cout << "<Server Address> <Server Port>";
		return -1;
	}

	char* serverIP = argv[1];
	// set user given port no or default as 1893
	in_port_t serverPort = (argc==3) ? atoi(argv[2]) : 1893;

	// getting nickname from user
	cout << "Enter your name: ";
	getline(std::cin, myname);

	// get the socket descriptor for dealing with the connection
	socketDescriptor = setupConnection(serverPort, serverIP);
	if (socketDescriptor==-1)
		return -1;

	sendMessage(socketDescriptor, ("QUERY:"+myname).c_str());		// send username to server in format QUERY:username

	receiveMessage(socketDescriptor, buffer, MAX_BUF_SIZE);			// Receive list of online clients from server
	
	splitCharStream(strdup(buffer), DELIM, -1, &usersList);			// Display list of users
	displayOnlineUsers(&usersList);

	// giving instructions to user
	cout << "To send a message to other users, enter the message in this format => username:message" << endl;
	cout << "To query online users, enter who." << endl;
	cout << "Enjoy Chatting." << endl;
	
	// starting two threads
	pthread_t threads[2];
	int rc1 = pthread_create(&threads[0], NULL, Sender, (void *)1);
	int rc2 = pthread_create(&threads[1], NULL, Receiver, (void *)2);
	
	if(rc1 || rc2)
		cout << "Error: Unable to create thread";
	
	close(sd);
	pthread_exit(NULL);

	return 0;
}
