#include "Client.h"

using namespace std;

string myname;					// nickName of client
int socketDescriptor;
std::vector<std::string> usersList;		// list of online users

char buffer[MAX_BUF_SIZE];				// buffer for receiving data from server
std::vector<std::string> bufferTokens;	// buffer string split and stored in vector 

std::string cmdInput;					// input from user (command line)
std::vector<std::string> cmdTokens;		// cmd input split and stored in vector


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
			cout << "Invalid IP address !!" << endl;
		else
			cout << "inet_pton() failed !!" << endl;
	
		return -1;
	}

	// create a socket
	int sd = socket(AF_INET, SOCK_STREAM, 0);
	if (sd < 0){
		cout << "Error opening socket !!" << endl;
		return -1;
	}

	// connect to server
	if (connect(sd, (struct sockaddr *)&serv_addr ,sizeof(serv_addr)) <0){
		cout << "connection to server failed !!" << endl;
		return -1;
	}
	
	return sd;
}

void *Sender(void *threadargs){
	while(1)
	{
		getline(std::cin, cmdInput);
		trim(cmdInput, ' ');
		if (cmdInput=="who")			// if user demands a list of online clients
			sendMessage(socketDescriptor, ("QUERY:"+myname).c_str());
		else if(cmdInput=="exit")
			exit(0);
		else{
			splitCharStream(strdup(cmdInput.c_str()), DELIM, 1, &cmdTokens);
			if (cmdTokens.size()==2)
				sendMessage(socketDescriptor, ("SEND:"+cmdTokens[0]+":"+cmdTokens[1]).c_str());
			else
				std::cout << "Invalid Input!! Please Re-Enter." << std::endl;
		}
	}
	pthread_exit(NULL);
}

void *Receiver(void *threadargs){
	while(1){

		receiveMessage(socketDescriptor, buffer, MAX_BUF_SIZE);			// Receive list of online clients from server
		if (strlen(buffer) > 0){	// if something received from server
			splitCharStream(strdup(buffer), DELIM, 2, &bufferTokens);
			if (bufferTokens[0]=="RECV")								// another client sent the message to client
				std::cout << bufferTokens[1] << "said: " << bufferTokens[2] << std::endl;
			else if(bufferTokens[0]=="ONLINE"){							// list of online clients received from server
				splitCharStream(strlen(buffer), DELIM, -1, &usersList);
				displayOnlineUsers(usersList);
			}
			else if(bufferTokens[0]=="SENT")							// request status received from server
				std::cout << "Message sending to " << bufferTokens[1] << ": " << bufferTokens[2] << std::endl;
		}
	}
	pthread_exit(NULL);
}


void sendMessage(int sockfd, char* data){
	ssize_t dataLen = strlen(data);
	ssize_t sentLen = send(sockfd, data, dataLen, 0);
	if (sentLen < 0){
		cout << "Name sending failed !!" << endl;
		exit(-1);
	}
	else if(sentLen != dataLen){
		cout << "send(): sent unexpected number of bytes" << endl;
		exit(-1);
	}
}


void receiveMessage(int sockfd, char*buffer, int bufSize){
	ssize_t recvLen = recv(sockfd, buffer, bufSize-1, 0);
	if (recvLen < 0){
		cout << "Receive Failed !!" << endl;
		exit(-1);
	}
}

/*
 * Split routine, 'count' defines the number of delimiters to check
 */
void splitCharStream(char* stream, const char* delim, int count, std::vector<std::string>* result){
	(*result).clear();													/* Clear the result vector */

	char* token;
	std::string subToken;

	/* Flag to check if limits applicable */
	bool limitDelim = (count==-1)? false : true;

	token = strtok(stream, delim);
	while(token!=NULL){
		if(limitDelim)													/* If limit, then update count */
			--count;

		subToken = std::string(token);
		trim(subToken, ' ');											/* Trim leading and trailing spaces */
		(*result).push_back(subToken);									/* Push into results */

		if(limitDelim && count<=0)										/* Get all of next of count exhausted */
			token = strtok(NULL, "");
		else															/* Keep applying default delimiter */
			token = strtok(NULL, delim);
	}
}

/* 
 * Trim leading and trailing unwanted char (tchar)
 */
void trim(std::string& s, const char tchar){
	int i=0;
	while(s[i]==tchar)
		++i;
	s = s.substr(i, std::string::npos);
	i=s.length()-1;
	while(s[i]==tchar)
		--i;
	s = s.substr(0, i+1);
}


void displayOnlineUsers(std::vector<std::string> &usersList){
	std::cout << "Following users are available now:\n";
	for (unsigned int i=1; i<usersList.size(); ++i)
		std::cout << usersList[i] << std::endl;
}
