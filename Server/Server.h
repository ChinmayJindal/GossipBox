/* Server includes */
#ifndef _SERVER_H_
#define _SERVER_H_

#include <unistd.h>
#include <string.h>
#include <algorithm>
#include <iostream>
#include <vector>
#include <map>
#include <utility>

#include <sys/socket.h>
#include <sys/types.h>
#include <arpa/inet.h>
#include <netinet/in.h>
/* Codes */
#define SOCKET_OK 200					/* All OK */

#define E_DESCRIPTOR 500				/* Error setting Socket Descriptor */
#define E_BIND 501						/* Error binding Socket */
#define E_LISTEN 502					/* Error listening on Socket */
#define E_SELECT 503					/* Error using select() (not synchronous I/O) */
#define E_ACCEPT 504					/* Error accepting connection */
#define E_RECV 505						/* Error in receive (recv) */
#define E_HUNG 506						/* Error connection closed */
#define E_IPADDR 507					/* Error retreiving IP address */

#define TYPE_QUERY 100
#define TYPE_SEND 101

/* Required parameters */
#define PORT_DEFAULT 1893				/* Select default port */
#define MAX_PENDING 10					/* Maximum pending Requests */
#define MAX_BUFFER 1000					/* Maximum buffer size for data */
#define DELIM ":"						/* Delimiter for queries from client */

extern int ERROR;						/* Error flags set here */
extern std::string error_message;		/* Error message set here */

/* Tables to manage client info */
typedef std::pair<std::string,int> iportPair;					/* This contains IP address, Port pair */

extern std::map<int, iportPair> socketTable;					/* Table with key as socket descriptor */
extern std::map<std::string, iportPair> nameTable;				/* Table with key as username of client */

/* 
 * Returns socket descriptor on success, else -1 and sets error flag	
 * If Port number -1, switch to default
 */
int setupSocket(int);	

/*
 * Start the server's loop, using non-blocking interface
 */				
void startServer(int);	

/*
 *	Accept connections, update hashmap
 */				
int acceptConnection(int);

/*
 * Receive data, set error flags (if any)
 */		
int receiveData(int, char*);

/*
 * Process request,
 */
void processRequest(char*);

/*
 * Split data, based on delimiters, upto a limit
 */
void splitCharStream(char*, const char*, int, std::vector<std::string>*);
void trim(std::string&, const char);


#endif		//_SERVER_H_