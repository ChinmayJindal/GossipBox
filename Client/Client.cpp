#include "Client.h"

std::string myname;					// nickName of client
int socketDescriptor;
std::vector<std::string> usersList;		// list of online users

char buffer[MAX_BUF_SIZE];				// buffer for receiving data from server
std::vector<std::string> bufferTokens;	// buffer string split and stored in vector 

std::string cmdInput;					// input from user (command line)
std::vector<std::string> cmdTokens;		// cmd input split and stored in vector


// returns socket descriptor otherwise -1
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
			std::cout << "Invalid IP address !!" << std::endl;
		else
			std::cout << "inet_pton() failed !!" << std::endl;
	
		return -1;
	}

	// create a socket
	int sd = socket(AF_INET, SOCK_STREAM, 0);
	if (sd < 0){
		std::cout << "Error opening socket !!" << std::endl;
		return -1;
	}

	// connect to server
	if (connect(sd, (struct sockaddr *)&serv_addr ,sizeof(serv_addr)) <0){
		std::cout << "connection to server failed !!" << std::endl;
		return -1;
	}
	
	return sd;
}

// thread which sends user requests to server in a special packaged format
void *Sender(void *threadargs){
	while(1)
	{
		std::cout << ">>>  " << std::flush;
		getline(std::cin, cmdInput);
		trim(cmdInput, ' ');
		if (cmdInput==WHO)			// if user demands a list of online clients
			sendMessage(socketDescriptor, ("QUERY:"+myname).c_str());
		else if(cmdInput==EXIT){
			std::cout << "You are going offline..!!\n";
			close(socketDescriptor);
			exit(0);
		}
		else if(cmdInput==HELP)
			giveInstructions();
		else{
			splitCharStream(strdup(cmdInput.c_str()), DELIM, 1, &cmdTokens);
			if (cmdTokens.size()==2){
				if(cmdTokens[0]=="broadcast")
					sendMessage(socketDescriptor, ("BCAST:"+myname+":"+cmdTokens[1]).c_str());
				else if(cmdTokens[0]=="cname")
					sendMessage(socketDescriptor, ("CHNAME:"+myname+":"+cmdTokens[1]).c_str());
				else
					sendMessage(socketDescriptor, ("SEND:"+myname+":"+cmdTokens[0]+":"+cmdTokens[1]).c_str());
			}
			else
				std::cout << "Invalid Input!! Please Re-Enter." << std::endl;
		}
	}
	pthread_exit(NULL);
}

// thread which receives and processes data
void *Receiver(void *threadargs){
	while(1){
		memset(buffer, 0, MAX_BUF_SIZE);
		while(receiveMessage(socketDescriptor, buffer)<=0);			// Receive list of online clients from server
		if (strlen(buffer) > 0){									// if something received from server
			std::cout << "\n**********************************************************************" << std::endl;
			splitCharStream(strdup(buffer), DELIM, 2, &bufferTokens);
			if (bufferTokens[0]=="RECV")								// another client sent the message to client
				std::cout << "\t\"" <<bufferTokens[1] << "\" says : " << bufferTokens[2] << std::endl;
			else if(bufferTokens[0]=="ONLINE"){							// list of online clients received from server
				splitCharStream(strdup(buffer), DELIM, -1, &usersList);
				displayOnlineUsers(usersList);
			}
			else if(bufferTokens[0]=="CHNAME"){							// name change request's response
				if(bufferTokens[2] == "success"){
					std::cout << "\tName successfully changed to " << bufferTokens[1] << std::endl;
					myname=bufferTokens[1];
				}
				else if(bufferTokens[2] == "duplicate")
					std::cout << "\tName change failed(\'" << bufferTokens[1] << "\' name already in use)" << std::endl;
				else if(bufferTokens[2] == "fail")
					std::cout << "\tName changing to " << bufferTokens[1] << "failed" << std::endl;
			}
			else if(bufferTokens[0]=="SENT"){							// request status received from server
				if(bufferTokens[2] == SUCCESS)
					std::cout << "\tMessage to "<<bufferTokens[1] << " successfully sent" << std::endl;
				else if(bufferTokens[2] == OFFLINE)
					std::cout << "\tMessage to "<<bufferTokens[1] << " not sent (User Offline)" << std::endl;
				else if(bufferTokens[2] == SEND_ERR)
					std::cout << "\tMessage to "<<bufferTokens[1] << " failed" << std::endl;
				else if (bufferTokens[2] == BSUCCESS)
					std::cout << "\tBroadcast Successful" << std::endl;
			}
			std::cout << "***********************************************************************" << std::endl;
		}
		std::cout << "\n>>>  " << std::flush;
	}
	pthread_exit(NULL);
}

int sendMessage(int sockfd, const char* data){
	// final data stream to be sent
	int sentLen = send(sockfd, data, strlen(data) ,0);
	if (sentLen < 0){
		std::cout << "Sending to server failed !!\n";
		return -1;
	}
	else if(sentLen != strlen(data)){
		std::cout << "send(): sent unexpected number of bytes\n";
		return 0;
	}

	#ifdef __DEBUG__
	std::cout << sentLen << " characters Successfully sent\n";
	#endif
	return sentLen;
}

int receiveMessage(int sockfd, char* buffer){
	int recvLen = recv(sockfd, buffer, MAX_BUF_SIZE-1, 0);
	if (recvLen < 0){
		return -1;
	}
	return recvLen;
}

// Split routine, 'count' defines the number of delimiters to check
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
		if (subToken.length() >0)
			(*result).push_back(subToken);								/* Push into results */

		if(limitDelim && count<=0)										/* Get all of next of count exhausted */
			token = strtok(NULL, "");
		else															/* Keep applying default delimiter */
			token = strtok(NULL, delim);
	}
}

// Trim leading and trailing unwanted char (tchar)
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

// display list of all the online users
void displayOnlineUsers(std::vector<std::string> &usersList){
	unsigned int numUsers = usersList.size()-1;
	if (numUsers==0)
		std::cout << "\tNo one is online..!!" << std::endl;
	else{
		std::cout << "\tCurrently " << numUsers << " user" << ((numUsers==1) ? " is":"s are") << " available:"<< std::endl;
		for (unsigned int i=1; i<=numUsers; ++i)
			std::cout <<  "\t" <<usersList[i] << std::endl;
	}
}

// prints the instruction set for the user
void giveInstructions(){
	std::cout << "\n************************Instructions*******************************"<<std::endl;
	std::cout << "\tTo send message, use this format => username:message" << std::endl;
	std::cout << "\tTo broadcast message, use this format => broadcast:message" << std::endl;
	std::cout << "\tTo query online users, enter " << WHO << std::endl;
	std::cout << "\tTo end the connection, enter " << EXIT << std::endl;
	std::cout << "\tTo change name, use this format => cname:new_name" << std::endl;
	std::cout << "\tEnjoy Chatting." << std::endl;	
	std::cout << "*********************************************************************\n"<<std::endl;	
}
