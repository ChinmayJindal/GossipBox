#include "Server.h"

/* Global variable definitions, declared in "Server.h" */
int SERVER_SOCKET;

int ERROR;
std::string error_message;

SocketTable socketTable;
NameTable nameTable;

int setupSocket(int port){
	int sd = socket(AF_INET, SOCK_STREAM, 0);				/* Socket Descriptor */
	if(sd<0){
		ERROR = E_DESCRIPTOR;
		error_message = "Error creating socket.";
		return -1;
	}

	/* Standard Parameters */
	struct sockaddr_in servAddr;
	memset(&servAddr, 0, sizeof(servAddr));
	
	servAddr.sin_family = AF_INET;
	servAddr.sin_addr.s_addr = htonl(INADDR_ANY);
	if(port==-1){
		port = PORT_DEFAULT;
	}
	servAddr.sin_port = htons(port);

	/* Bind to local Port */
	int b = bind(sd, (struct sockaddr *) &servAddr, sizeof(servAddr));
	if(b<0){
		ERROR = E_BIND;
		error_message = "Error binding socket.";
		return -1;
	}

	/* Start Listening */
	int l = listen(sd, MAX_PENDING);
	if(l<0){
		ERROR = E_LISTEN;
		error_message = "Error setting listener.";
		return -1;
	}

	ERROR = SOCKET_OK;
	error_message = "";
	
	return sd;
}

/* Main server routine */
void startServer(int port){
	int socketDescriptor = setupSocket(port);
	if(socketDescriptor==-1)						/* Return on failure, flag already set. */
		return;

	/* Set timeout values */
	struct timeval timeOut;
	timeOut.tv_sec = 0;
	timeOut.tv_usec = 0;
	
	fd_set master;									/* Master socket descriptor set */
	fd_set read_sds;								/* Temporary socket descriptor set */
	FD_ZERO(&master);
	FD_ZERO(&read_sds);

	int maxsd;										/* Maximum descriptor number */
	char buffer[MAX_BUFFER];						/* For incoming data storage */

	/* Add current socket to master set */
	FD_SET(socketDescriptor, &master);
	maxsd = socketDescriptor;

	SERVER_SOCKET = socketDescriptor;
	std::cout<<"Server is ready for operations...\n";
	/* Main server loop */
	while(true){
		memcpy(&read_sds, &master, sizeof(fd_set));	/* Make a temporary copy */

		/* NULL for writefd and exceptfd (Not needed) */
		int s = select(maxsd+1, &read_sds, NULL, NULL, &timeOut);
		if(s<0){
			ERROR = E_SELECT;
			error_message = "Error using select.";
		}

		/* Search for data to read */
		for(int sock=0;sock<=maxsd;++sock){
			if(FD_ISSET(sock, &read_sds)){									/* Pending read status found */
				if(sock == socketDescriptor){								/* If match on socket descriptor of server */
					int newSd = acceptConnection(socketDescriptor);
					if(newSd>-1){
						FD_SET(newSd, &master);
						maxsd = std::max(maxsd, newSd);						/* Update maximum descriptor */
					}
				}
				else{														/* Handle data from client */
					int nbytes = receiveData(sock, buffer);
					if(nbytes==-1){
						socketTable.erase(socketTable.find(sock));
						close(sock);
						FD_CLR(sock, &master);								/* Update the master socket descriptor set */
					}else{
						processRequest(sock, buffer);
					}
				}
			}
		}
	}
}

int acceptConnection(int serverSocket){
	struct sockaddr_in remoteAddr;											/* IP address and Port info */
	socklen_t remoteAddrLen = sizeof(struct sockaddr_in);
	memset(&remoteAddr, 0, remoteAddrLen);

	int newSd = accept(serverSocket, (struct sockaddr*)&remoteAddr, &remoteAddrLen);
	
	if(newSd<0){
		ERROR = E_ACCEPT;
		error_message = "Failed connection accept.";
		return -1;
	}

	char remoteIP[INET_ADDRSTRLEN];
	if(inet_ntop(AF_INET, &remoteAddr.sin_addr.s_addr, remoteIP, sizeof(remoteIP)) == NULL){
		ERROR = E_IPADDR;
		error_message = "Error retrieving IP address of client.";
		return -1;
	}
	int remotePort = ntohs(remoteAddr.sin_port);

	/* Add to the Socket Table for future reference */
	addToSocketTable(newSd, std::string(remoteIP), remotePort);

	return newSd;
}

int receiveData(int socketDescriptor,char* buffer){
	//memset(buffer, '\0', MAX_BUFFER);

	int nbytes = recv(socketDescriptor, buffer, sizeof(buffer), 0);
	if(nbytes<=0){
		if(nbytes==0){
			ERROR = E_HUNG;
			error_message = "Connection hung up.";
		}
		else{
			ERROR = E_RECV;
			error_message = "Error receiving data.";
		}
		return -1;
	}
	ERROR = SOCKET_OK;
	error_message = "";
	return nbytes;
}

int sendMessage(std::string toUser, std::string fromUser, std::string message){
	iportPair info = getFromNameTable(toUser);
	if(info.second==-1){
		ERROR = E_OFFLINE;
		error_message = toUser + ":Not online.";
		return -1;
	}
	
	int sd = socket(AF_INET, SOCK_STREAM, 0);
	if(sd<0){
		ERROR = E_DESCRIPTOR;
		error_message = toUser+":Error creating socket.";
		return -1;
	}
	struct sockaddr_in hostAddr;
	memset(&hostAddr, 0, sizeof(hostAddr));
	hostAddr.sin_family = AF_INET;
	int ipadd = inet_pton(AF_INET, info.first.c_str(), &hostAddr.sin_addr.s_addr);
	if(ipadd<0){
		ERROR = E_IPADDR;
		error_message = toUser+":Unable to translate IP address.";
		close(sd);
		return -1;
	}
	hostAddr.sin_port = htons(info.second);

	int conn = connect(sd, (struct sockaddr *)&hostAddr, sizeof(hostAddr));
	if(conn<0){
		ERROR = E_CONN;
		error_message = toUser+":Unable to translate IP address.";
		close(sd);
		return -1;
	}

	message = "RECV:"+fromUser+":"+message;
	int sbytes = send(sd, message.c_str(), message.length(), 0);
	if(sbytes<0){
		ERROR = E_SEND;
		error_message = toUser+":Error sending message.";
		close(sd);
		return -1;
	}
	else if(sbytes!=(int)message.length()){
		ERROR = E_PART;
		error_message = toUser+":Partial message sent.";
		close(sd);
		return -1;
	}

	close(sd);
	return (int)sbytes;
}

/*
 * Parse incoming data
 * Types of messages and their responses are:
 * 					QUERY: <from>
 *						Response --> ONLINE: <user1> : <user2> : <user3> : ....
 *					SEND: <from> : <to> : <message>
 *						Response --> SENT: <to> : <statusMessage>
 *					RECV: <from> : <message>
 */
void processRequest(int fromSocket, char* stream){
	std::vector<std::string> tokens;

	char* dump = strdup(stream);												/* Create a duplicate stream */
	splitCharStream(dump, DELIM, 1, &tokens);									/* Retrieve the kind of message */
	delete dump;

	std::string reqType = tokens[0];
	int TYPE_FLAG = (reqType=="QUERY")? TYPE_QUERY : TYPE_SEND;
	int delimCount = (TYPE_FLAG==TYPE_QUERY)? 0 : 2;

	splitCharStream(strdup(tokens[1].c_str()), DELIM, delimCount, &tokens);		/* Based on kind retrieve other param list */
	
	std::string fromUser = tokens[0];
	std::string toUser, message;
	iportPair userPair;															/* Save info from table here */

	if(TYPE_FLAG == TYPE_QUERY){
		userPair = getFromNameTable(fromUser);									/* Check if name exists in name table */
		if(userPair.second==-1){												/* Add to name table from socket info */
			userPair = getFromSocketTable(fromSocket);
			addToNameTable(fromUser, userPair.first, userPair.second);
		}

		std::string onlineList = getOnlineList(fromUser);
		int s = send(fromSocket, onlineList.c_str(), onlineList.length(), 0);
		if(s<0){
			ERROR = E_SEND;
			error_message = "Unable to send query reply.";
		}
	}
	else if(TYPE_FLAG == TYPE_SEND){
		toUser = tokens[1];
		message = tokens[2];

		/* Send message to desired target username */
		int status = sendMessage(toUser, fromUser, message);
		std::string reply = "SENT: "+toUser+" : ";
		if(status==-1){
			reply += error_message;
		}
		/* Report back to the sender */
		int s = send(fromSocket, reply.c_str(), reply.length(), 0);
		if(s<0){
			ERROR = E_SEND;
			error_message = "Unable to send message status.";
		}
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

void addToSocketTable(int socket, std::string ipAddr, int port){
	iportPair newPair = iportPair(ipAddr, port);
	std::pair<SocketTable::iterator,bool> retVal = socketTable.insert(std::pair<int,iportPair>(socket, newPair));
	/* Update if key already exists to new <IP,Port> pair */
	if(!retVal.second){
		retVal.first->second = newPair;
	}
}

void addToNameTable(std::string name, std::string ipAddr, int port){
	iportPair newPair = iportPair(ipAddr, port);
	std::pair<NameTable::iterator,bool> retVal = nameTable.insert(std::pair<std::string,iportPair>(name, newPair));
	/* Update if key already exists to new <IP,Port> pair */
	if(!retVal.second){
		retVal.first->second = newPair;
	}
}

iportPair getFromSocketTable(int socketKey){
	SocketTable::iterator it = socketTable.find(socketKey);

	if(it == socketTable.end()){
		return iportPair("none",-1);
	}
	return it->second;
}

iportPair getFromNameTable(std::string userName){
	NameTable::iterator it = nameTable.find(userName);

	if(it == nameTable.end()){
		return iportPair("none",-1);
	}
	return it->second;
}

/* Returns list of online nicknames as
 * ONLINE: <user1> : <user2> : etc.
 */
std::string getOnlineList(std::string curUser){
	std::string response = "ONLINE: ";
	NameTable::iterator it = nameTable.begin();
	while(it!=nameTable.end()){
		if(it->first!=curUser){
			bool online = isOnline(it->first);
			if(online){
				response += it->first;
				response += ":";
			}
			else{
				nameTable.erase(nameTable.find(it->first));					/* Erase from table if no more accepting */
			}
		}
		++it;
	}
	return response;
}

bool isOnline(std::string user){
	iportPair info = getFromNameTable(user);									/* Check if user exists in table */
	if(info.second==-1)															/* WARNING: Redundant check */
		return false;

	int sd = socket(AF_INET, SOCK_STREAM, 0);
	if(sd<0){
		ERROR = E_DESCRIPTOR;
		error_message = "Error creating socket.";
		return false;
	}
	struct sockaddr_in hostAddr;
	memset(&hostAddr, 0, sizeof(hostAddr));
	hostAddr.sin_family = AF_INET;
	int ipadd = inet_pton(AF_INET, info.first.c_str(), &hostAddr.sin_addr.s_addr);
	if(ipadd<0){
		ERROR = E_IPADDR;
		error_message = "Unable to translate IP address.";
		close(sd);
		return false;
	}
	hostAddr.sin_port = htons(info.second);

	int conn = connect(sd, (struct sockaddr *)&hostAddr, sizeof(hostAddr));
	if(conn<0){
		close(sd);
		return false;
	}
	close(sd);
	return true;
}