#include "Server.h"

/* Global variable definitions, declared in "Server.h" */
int ERROR;
//string error_message;

int setupSocket(int port){
	int sd = socket(AF_INET, SOCK_STREAM, 0);				/* Socket Descriptor */
	if(sd<0){
		ERROR = E_DESCRIPTOR;
		//error_message = "Error creating socket.";
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
		//error_message = "Error binding socket.";
		return -1;
	}

	/* Start Listening */
	int l = listen(sd, MAX_PENDING);
	if(l<0){
		ERROR = E_LISTEN;
		//error_message = "Error setting listener.";
		return -1;
	}

	ERROR = SOCKET_OK;
	//error_message = "";
	
	return sd;
}

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
	struct sockaddr_in remoteAddr;					/* For connected client parameters */
	int maxsd;										/* Maximum descriptor number */
	int buffer[MAX_BUFFER];							/* Buffer for incoming data storage */

	/* Add current socket to master set */
	FD_SET(socketDescriptor, &master);
	maxsd = socketDescriptor;

	std::cout<<"Server is ready for operations.\n";
	/* Main server loop */
	while(true){
		memcpy(&read_sds, &master, sizeof(fd_set));	/* Make a temporary copy */

		/* NULL for writefd and exceptfd (Not needed) */
		int s = select(maxsd+1, &read_sds, NULL, NULL, &timeOut);
		if(s<0){
			ERROR = E_SELECT;
			//error_message = "Error using select.";
		}

		/* Search for data to read */
		for(int sock=0;sock<=maxsd;++sock){
			if(FD_ISSET(sock, &read_sds)){			/* Pending read status found */
				if(sock == socketDescriptor){		/* If match on socket descriptor of server */
					socklen_t remoteAddrLen = sizeof(remoteAddr);
					int newSd = accept(socketDescriptor, (struct sockaddr*)&remoteAddr, &remoteAddrLen);
					if(newSd>-1){
						FD_SET(newSd, &master);
						maxsd = std::max(maxsd, newSd);	/* Update maximum descriptor */
					}
				}
			}
			else{									/* Handle data from client */
				int nbytes = recv(sock, buffer, sizeof(buffer), 0);
				if(nbytes<=0){
					if(nbytes==0){
						ERROR = E_CONN_CLOSED;
						//error_message = "Connection hung up.";
					}
					else{
						ERROR = E_RECV;
						//error_message = "Error receiving data.";
					}
					close(sock);
					FD_CLR(sock, &master);
				}
				else{
					//Send data to clients here
				}
			}
		}
	}
}