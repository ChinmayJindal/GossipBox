#include "Server.h"

/* Global variable definitions, declared in "Server.h" */
int SERVER_SOCKET;

int ERROR;
std::string error_message;

Connections conn;

int setupSocket(int port){
	int sd = socket(AF_INET, SOCK_STREAM, 0);				/* Socket Descriptor */
	if(sd<0){
		ERROR = E_DESCRIPTOR;
		error_message = "Error creating socket.";

		#ifdef __DEBUG__
		std::cout<<error_message<<std::endl;
		#endif

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

	/* Reuse the socket flag */
	setsockopt(sd, SOL_SOCKET, SO_REUSEADDR, (int*)1, sizeof(int));
	/* Bind to local Port */
	int b = bind(sd, (struct sockaddr *) &servAddr, sizeof(servAddr));
	if(b<0){
		ERROR = E_BIND;
		error_message = "Error binding socket.";

		#ifdef __DEBUG__
		std::cout<<error_message<<std::endl;
		#endif

		return -1;
	}

	/* Start Listening */
	int l = listen(sd, MAX_PENDING);
	if(l<0){
		ERROR = E_LISTEN;
		error_message = "Error setting listener.";

		#ifdef __DEBUG__
		std::cout<<error_message<<std::endl;
		#endif

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

			#ifdef __DEBUG__
			std::cout<<error_message<<std::endl;
			#endif

			continue;
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
						removeFromConnectionTable(sock);
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
		error_message = "Failed new connection accept.";		

		#ifdef __DEBUG__
		std::cout<<error_message<<std::endl;
		#endif

		return -1;
	}
	
	#ifdef __INFO__

	char remoteIP[INET_ADDRSTRLEN];
	if(inet_ntop(AF_INET, &remoteAddr.sin_addr.s_addr, remoteIP, sizeof(remoteIP)) == NULL){
		ERROR = E_IPADDR;
		error_message = "Error retrieving IP address of client.";
		return -1;
	}
	int remotePort = ntohs(remoteAddr.sin_port);
	std::cout<<"Connection accepted from: \""<<remoteIP<<"\" : "<<remotePort<<std::endl;

	#endif

	return newSd;
}

int receiveData(int socketDescriptor,char* buffer){
	memset(buffer, '\0', MAX_BUFFER);
	int nbytes = recv(socketDescriptor, buffer, MAX_BUFFER, 0);
	fcntl(socketDescriptor, F_SETFL, O_NONBLOCK);
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
	/*
	while(nbytes>0){
		std::cout<<buffer<<std::endl;
		nbytes = recv(socketDescriptor, buffer, MAX_BUFFER, 0);
		std::cout<<nbytes<<std::endl;
	}
	*/

	#ifdef __DEBUG__
	std::cout<<"In receiveData: "<< buffer <<std::endl;
	#endif

	return nbytes;
}

int sendMessage(std::string toUser, std::string fromUser, std::string message){
	bool onn = isOnline(toUser);
	if(!onn){
		ERROR = E_OFFLINE;
		error_message = "offline";

		#ifdef __DEBUG__
		std::cout<<error_message<<std::endl;
		#endif

		return -1;
	}
	int targetSocket = getFromConnectionTable(toUser);

	message = "RECV:"+fromUser+":"+message;
	int sbytes = send(targetSocket, message.c_str(), message.length(), 0);
	if(sbytes<0){
		ERROR = E_SEND;
		error_message = "e_send";

		#ifdef __DEBUG__
		std::cout<<error_message<<std::endl;
		#endif

		return -1;
	}

	return sbytes;
}

/*
 * Parse incoming data
 * Types of messages and their responses are:
 *					NEW: <from>
 *						Response --> NEW: <status>
 * 					QUERY: <from>
 *						Response --> ONLINE: <user1> : <user2> : <user3> : ....
 *					SEND: <from> : <to> : <message>
 *						Response --> SENT: <to> : <statusMessage>
 *					
 *					/toclient/ RECV: <from> : <message>
 */
void processRequest(int fromSocket, char* stream){
	std::vector<std::string> tokens;

	char* dump = strdup(stream);												/* Create a duplicate stream */
	splitCharStream(dump, DELIM, 1, &tokens);									/* Retrieve the kind of message */
	delete dump;

	std::string reqType = tokens[0];

	int TYPE_FLAG = -1, delimCount=-1;
	if(reqType=="NEW"){
		TYPE_FLAG = TYPE_NEW;
		delimCount = 0;
	}
	else if(reqType=="QUERY"){
		TYPE_FLAG = TYPE_QUERY;
		delimCount = 0;
	}
	else if(reqType=="SEND"){
		TYPE_FLAG = TYPE_SEND;
		delimCount = 2;
	}

	if(TYPE_FLAG==-1)
		return;
	
	#ifdef __DEBUG__
	std::cout<<TYPE_FLAG<<" --- ";
	#endif

	splitCharStream(strdup(tokens[1].c_str()), DELIM, delimCount, &tokens);		/* Based on kind retrieve other param list */
	
	std::string fromUser = tokens[0];
	std::string toUser, message;

	if(TYPE_FLAG == TYPE_NEW){
		#ifdef __INFO__
		std::cout<<"\""<<fromUser<<"\" new registration."<<std::endl;
		#endif

		int oldSock = getFromConnectionTable(fromUser);
		std::string reply = "duplicate";
		if(oldSock==-1){
			addToConnectionTable(fromUser, fromSocket);								/* Add user socket pair to table */
			reply = "registered";
		}
		#ifdef __INFO__
		std::cout<<"\""<<fromUser<<"\" reg. status: "<<reply<<"."<<std::endl;
		#endif
		int s = send(fromSocket, reply.c_str(), reply.length(), 0);
		if(s<0){
			ERROR = E_SEND;
			error_message = "Unable to send query reply.";

			#ifdef __DEBUG__
			std::cout<<error_message<<std::endl;
			#endif
		}	
	}
	else if(TYPE_FLAG == TYPE_QUERY){
		#ifdef __INFO__
		std::cout<<"\""<<fromUser<<"\" queried."<<std::endl;
		#endif

		std::string onlineList = getOnlineList(fromUser);

		int s = send(fromSocket, onlineList.c_str(), onlineList.length(), 0);
		if(s<0){
			ERROR = E_SEND;
			error_message = "Unable to send query reply.";

			#ifdef __DEBUG__
			std::cout<<error_message<<std::endl;
			#endif
		}
	}
	else if(TYPE_FLAG == TYPE_SEND){
		toUser = tokens[1];
		message = tokens[2];

		#ifdef __INFO__
		std::cout<<"\""<<fromUser<<"\" sending message to \""<<toUser<<"\"."<<std::endl;
		#endif

		/* Send message to desired target username */
		int status = sendMessage(toUser, fromUser, message);
				
		std::string reply = "SENT: "+toUser+" : ";
		if(status==-1){
			reply += error_message;

			#ifdef __DEBUG__
			std::cout<<"\""<<fromUser<<"\" could not send a message to \""<<toUser<<"\". "<<std::endl;
			#endif
		}
		else{
			reply += "success";

			#ifdef __DEBUG__
			std::cout<<"\""<<fromUser<<"\" sent a message to \""<<toUser<<"\"."<<std::endl;
			#endif
		}
		/* Report back to the sender */
		int s = send(fromSocket, reply.c_str(), reply.length(), 0);
		if(s<0){
			ERROR = E_SEND;
			error_message = "Unable to send message status.";

			#ifdef __DEBUG__
			std::cout<<error_message<<std::endl;
			#endif
		}
	}
}

/*
 * Split routine, 'count' defines the number of delimiters to check (if -1 then no limit)
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
		if(subToken.length()>0)
			(*result).push_back(subToken);								/* Push into results */

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

void addToConnectionTable(std::string username, int socketDescriptor){
	std::pair<Connections::iterator,bool> retVal = conn.insert(std::make_pair(username, socketDescriptor));
	/* Update if key already exists to new socket Descriptor */
	if(!retVal.second){
		retVal.first->second = socketDescriptor;
	}
}

void removeFromConnectionTable(int socketDescriptor){
	/* Remove the user with the closed socket */
	Connections::iterator it = conn.begin();
	while(it!=conn.end()){
		if(it->second == socketDescriptor){
			
			#ifdef __INFO__
			std::cout<<"\""<<it->first<<"\" went offline."<<std::endl;
			#endif

			conn.erase(it);
			break;
		}
		++it;
	}
}

/* Get socket descriptor of the username */
int getFromConnectionTable(std::string username){
	Connections::iterator it = conn.find(username);
	if(it == conn.end())
		return -1;
	return it->second;
}

/* 
 * Returns list of online nicknames as
 * ONLINE: <user1> : <user2> : ...
 */
std::string getOnlineList(std::string curUser){
	std::string response = "ONLINE: ";
	Connections::iterator it = conn.begin();

	while(it!=conn.end()){
		if(it->first!=curUser){
			bool online = isOnline(it->first);
			if(online){
				response += it->first;
				response += ":";
			}
		}
		++it;
	}

	return response;
}

bool isOnline(std::string user){
	int userSocket = getFromConnectionTable(user);
	if(userSocket==-1)
		return false;
	return true;
}