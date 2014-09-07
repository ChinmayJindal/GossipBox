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
#include <fcntl.h>

extern int SERVER_SOCKET;

/* Codes */
#define SOCKET_OK 200					/* All OK */

#define E_DESCRIPTOR 500				/* Error setting Socket Descriptor */
#define E_BIND 501						/* Error binding Socket */
#define E_LISTEN 502					/* Error listening on Socket */
#define E_SELECT 503					/* Error using select() (not synchronous I/O) */
#define E_ACCEPT 504					/* Error accepting connection */
#define E_RECV 505						/* Error in receive (recv) */
#define E_HUNG 506						/* Error connection closed */
#define E_SEND 507						/* Error in sending message */
#define E_IPADDR 508					/* Error retreiving IP address */
#define E_OFFLINE 509					/* Error if client offline */
#define E_CONN 510 						/* Error if connect failed */
#define E_PART 511 						/* Error if partial message sent */

#define TYPE_QUERY 100 					/* Define Type of message, querying */
#define TYPE_SEND 101					/* Define Type of message, sending */
#define TYPE_PING 102					/* Define Type of message, ping */

/* Required parameters */
#define PORT_DEFAULT 8080				/* Select default port */
#define MAX_PENDING 10					/* Maximum pending Requests */
#define MAX_BUFFER 1000					/* Maximum buffer size for data */
#define DELIM ":"						/* Delimiter for queries from client */

extern int ERROR;						/* Error flags set here */
extern std::string error_message;		/* Error message set here */

typedef std::map<std::string, int> Connections;	/* Manage connections here*/

extern Connections conn;						/* Table of username versus open socket */

/* 
 * Returns socket descriptor on success, else -1 and sets error flag	
 * If Port number -1, switch to default
 */
int setupSocket(int);	

/* Start the server's loop, using non-blocking interface */				
void startServer(int);	

/* Accept connections, update hashmap */				
int acceptConnection(int);

/* Receive data, set error flags (if any) */		
int receiveData(int, char*);

/* Send message to the user */
int sendMessage(std::string, std::string, std::string);

/* Process request */
void processRequest(int, char*);

/* Split data, based on delimiters, upto a limit */
void splitCharStream(char*, const char*, int, std::vector<std::string>*);
void trim(std::string&, const char);

/* Function for table entries, queries */
void addToConnectionTable(std::string, int);
int getFromConnectionTable(std::string);
void removeFromConnectionTable(int);

/* Get latest online nicknames */
std::string getOnlineList(std::string);
bool isOnline(std::string);

#endif		/*_SERVER_H_*/
