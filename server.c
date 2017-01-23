#include <stdio.h>
#include <sys/socket.h>
#include <sys/types.h> 
#include <netinet/in.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>

int sock, cSock;

void error(char *msg)
{
	perror(msg);
	exit(1);
}

void interrupt() {
	printf("Quitting...\n");
	close(cSock);
	close(sock);
	exit(0);
}

int isValidPort(char port[])
{
	int i = 0;

	//check for minus sign
	if (port[0] == '-')
	{
		return 0;
	}
	for (i=0; port[i]!=0; i++)
	{
		if (!isdigit(port[i]))
		{
			return 0;
		}
	}
	//check valid range of ports
	if(atoi(port)>65535 || atoi(port)<1)
	{
		return 0;
	}
	return 1;
}

int main(int argc, char *argv[])
{
	//Errors are currently printed to standard out so this process cannot be run in the background without a terminal
	int optval = 1, forkProcessID, readVar;
	socklen_t clientLen;
	char buf[256];
	struct sockaddr_in server, client; //variables for setting internet address family

	signal(SIGINT, interrupt);

	if (argc<2) 
	{
		printf("Please provide the port number to listen on as the argument. Example: server 8080\n");
		return 0;
	}
	
	if(!isValidPort(argv[1]))
	{
		printf("Please enter a valid port number between 1-65535\n");
		exit(0);
	}
	
	//clearing the internet address family structure
	memset((char *)&server, 0, sizeof(server));
	server.sin_family = AF_INET;
	server.sin_addr.s_addr = INADDR_ANY;
	server.sin_port = htons(atoi(argv[1]));
	
	//creating the socket
	if((sock = socket(AF_INET, SOCK_STREAM, 0))<0)
	{
		error("Error creating socket\n");
	}
	
	//set reusable socket option once its closed
	if(setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, &optval, sizeof(int)) == -1)
	{
		error("Error setting option to make socket reusable\n");
	}
	
	//bind the socket	
	if(bind(sock, (struct sockaddr *)&server, sizeof(server))<0)
	{
		error("Error binding socket\n");
	}

	//listen for an incoming connection to the socket
	listen(sock, 5);
	printf("Listening on port: %d\n", atoi(argv[1]));

	clientLen = sizeof(client);
	while(1)
	{
		//accept incoming connection
		if((cSock = accept(sock, (struct sockaddr *)&client, &clientLen))<0)
		{
			error("Error accepting incoming connection\n");
		}
	
		//create new process for the client
		if((forkProcessID = fork())<0)
		{
			error("Error creating new process");
		}
	
		if(forkProcessID==0)
		{
			//read incoming data to buffer while the client is connected
			while(getsockopt(sock, SOL_SOCKET, SO_ERROR, &optval, &clientLen) == 0)
			{
				memset(buf, '\0', sizeof(buf));
				readVar = read(cSock, buf, sizeof(buf));
				if(readVar<0)
				{
					close(sock);
					close(cSock);
					error("Error reading request from client");
				}
				if(readVar==0)
				{
					close(sock);
					close(cSock);
					exit(1);
				}
				if(write(cSock, buf, readVar)<0)
				{
					close(sock);
					close(cSock);
					error("Error writing response to client");
				}
			}
			close(cSock);
		}
		else
		{
			close(cSock);
		}
	}
	return 0;
}