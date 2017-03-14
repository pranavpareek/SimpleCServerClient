/*
	Author: Pranav Pareek
	Email: me@pranavpareek.com
*/

#include <stdio.h>
#include <sys/socket.h>
#include <sys/types.h> 
#include <netinet/in.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <netdb.h>
#include <arpa/inet.h>

int sock;

void error(char *msg)
{
	perror(msg);
	exit(1);
}

void interrupt() {
	printf("Quitting...\n");
	close(sock);
	exit(0);
}

void isValidIpAddress(char *hostname, char *ipAddress)
{
	struct sockaddr_in server;
	struct hostent *hostbyname;
	struct in_addr **addrList;
	int i;

	int result = inet_pton(AF_INET, hostname, &(server.sin_addr));
	if(result == 0)	//not a valid ip, lets try converting hostname to valid ip
	{
		if ((hostbyname=gethostbyname(hostname))==NULL)
		{
			printf("Error in gethostbyname\n");
			exit(1);
		}
		addrList = (struct in_addr **) hostbyname->h_addr_list;
		for(i = 0; addrList[i] != NULL; i++) 
		{
			strcpy(ipAddress, inet_ntoa(*addrList[i]));
			break;
		}
	}
	else if(result == 1) //valid ip
	{
		strcpy(ipAddress, hostname);
	}
	else
	{
		error("Error in hostname/IP address\n");
	}
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
	char buf[256], count[64];
	char ipAddress[256];
	struct sockaddr_in server;
	struct hostent *hostbyname;
	int n, counter = 0;

	if (argc<3) 
	{
		printf("Please provide the hostname/ip and port number. Example: client <localhost/ip> 8080\n");
		return 0;
	}

	if(!isValidPort(argv[2]))
	{
		printf("Please enter a valid port number between 1-65535\n");
		exit(0);
	}
	
	//check if argument is valid IP address, if its hostname then convert it to IP address
	isValidIpAddress(argv[1], ipAddress);
	
	//clearing the internet address family structure
	memset((char *) &server, 0, sizeof(server));
	server.sin_port = htons(atoi(argv[2]));
	server.sin_family = AF_INET;
	server.sin_addr.s_addr = inet_addr(ipAddress);

	signal(SIGINT, interrupt);
	
	//creating the socket
	if((sock = socket(AF_INET, SOCK_STREAM, 0))<0)
	{
		error("Error creating socket\n");
	}

	//connect to server
	if (connect(sock,(struct sockaddr *) &server,sizeof(server))<0)
	{
		error("Error connecting to server\n");
	}

	while(1)
	{
		counter++;
		sprintf(count, "%d", counter);
		//write to socket
		n = write(sock,count,strlen(count));
		if(n<0)
		{
			error("Error writing request to client\n");
		}

		//read from socket
		memset(buf, 0, sizeof(buf));
		if(read(sock,buf,255)<0)
		{
			error("Error reading response from client\n");
		}

		//print server response
		printf("Server response: %s\n", buf);
		sleep(1);
	}
	
	return 0;
}
