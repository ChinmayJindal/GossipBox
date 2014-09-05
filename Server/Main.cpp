/*
 * The main executable for the Chat Application Server
 */

/* Application Includes */
#include "Server.h"

/* System includes */
#include <stdlib.h>
#include <signal.h>

/* Forward declarations */
int ATOI(char*);										/* Convert char array into integer */
void signal_callback_handler(int);						/* Handle interrupt signals */

int main(int argc, char* argv[]){
	signal(SIGINT, signal_callback_handler);
	signal(SIGTERM, signal_callback_handler);
	signal(SIGTSTP, signal_callback_handler);

	int port = PORT_DEFAULT;
	if (argc<2){
		std::cout<<"WARNING: No port specified, selecting default port: "<<PORT_DEFAULT<<"\n";
	}else{
		port = ATOI(argv[1]);
		if(port==-1){
			std::cout<<"WARNING: No port specified, selecting default port: "<<PORT_DEFAULT<<"\n";
			port = PORT_DEFAULT;
		}
	}

	startServer(port);

	return 0;
}

int ATOI(char *num){
	int n = strlen(num);
	int res=0,place=1,i=n-1;
	while(i>=0){
		if(num[i]>='0' && num[i]<='9'){
			res += int(num[i]-'0')*place;
			--i;
			place*=10;
		}else{
			return -1;
		}
	}
	return res;
}

void signal_callback_handler(int signum){
	std::cout<<"\nEnding services..\n";
	close(SERVER_SOCKET);
	exit(0);
}