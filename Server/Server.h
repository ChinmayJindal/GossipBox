/* Server includes */
#ifndef _SERVER_H_
#define _SERVER_H_

#include <unistd.h>
#include <string.h>
#include <algorithm>
#include <iostream>

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
#define E_CONN_CLOSED 506				/* Error connection closed */

/* Required parameters */
#define PORT_DEFAULT 1893				/* Select default port */
#define MAX_PENDING 10					/* Maximum pending Requests */
#define MAX_BUFFER 1000					/* Maximum buffer size for data */

extern int ERROR;						/* Error flags set here */
//extern string error_message;			/* Error message set here */

/* Function prototypes */
int setupSocket(int);					/* Returns socket descriptor on success, else -1 and sets error flag	
										 * If Port number -1, switch to default
										 */
void startServer(int);					/*
										 * Start the server's loop, using non-blocking interface
										 */

#endif		//_SERVER_H_