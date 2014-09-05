/*
 * The main executable for the Chat Application Server
 */

/* Application Includes */
#include "Server.h"

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

int main(int argc, char* argv[]){
	int port = PORT_DEFAULT;
	if (argc<2){
		std::cout<<"WARNING: No port specified, selecting default port: "<<PORT_DEFAULT<<"\n";
	}else{
		port = ATOI(argv[1]);
	}

	startServer(port);

	return 0;
}
