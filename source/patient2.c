#include <stdio.h>
#include <stdlib.h>
#include <netdb.h>
#include <sys/socket.h>
#include <arpa/inet.h> 
#include <sys/types.h> 
#include <string.h>
#include <unistd.h>

#define LOGIN_FILE_PATH	"./input/patient2.txt"
#define INSUR_FILE_PATH	"./input/patient2insurance.txt"	
#define BUF_SIZE		128
#define SERVER_NAME		"127.0.0.1"
#define TCP_SERVER_PORT		21647

typedef struct sockaddr		sockaddr;
typedef struct sockaddr_in	sockaddr_in;
typedef struct hostent		hostent;

int CommWithHealthCenter(char* doc_name);	//communicate with health center, TCP connection
void CommWithDoc(char* doc_name, int doc_port);		//communicate with doctor, UDP connection
int ConnectHCServer();		//connect health center server
int ClientLoginAuth(int client_sock_d);		//login authentication in client side
int ClientMakeAppointment(int client_sock_d, char* doc_name, int* doc_port);	//make appointment in client side



//funciton that handles communication with Health Center Server
int CommWithHealthCenter(char *doc_name)
{

	int client_sock_d=0;	//client socket descriptor
	int doc_port=0;

	//call ConnectHCServer() and obtain client socket descriptor
	client_sock_d = ConnectHCServer();
	if(client_sock_d == -1)
	{
		return-1;
	}

	//call ClientLoginAuth() and deal with authentication
	if(ClientLoginAuth(client_sock_d) == -1)
	{
		close(client_sock_d);	
		return -1;
	}
	
	//call ClientMakeAppointment() and make appointment with Health Center Server
	if(ClientMakeAppointment(client_sock_d, doc_name, &doc_port) != 0)
	{
		close(client_sock_d);
		return -1;
	}

	close(client_sock_d);
	return doc_port;
}

//function that handles communication with doctor
void CommWithDoc(char* doc_name, int doc_port)
{
	int client_sock_d;
	sockaddr_in server_saddr, client_saddr;
	socklen_t slen=sizeof(sockaddr_in);
	FILE *fp=NULL;
	char buf[BUF_SIZE]={0};
	int plan=0;
	hostent *hent=NULL; 

	//create datagram socket
	client_sock_d = socket(AF_INET, SOCK_DGRAM, 0);
	if(client_sock_d == -1)
	{
//		perror("CommWithDoc socket()");
		return;
	}

	//fill in server socket address structure
	hent = gethostbyname(SERVER_NAME);
	server_saddr.sin_family = AF_INET;
	server_saddr.sin_addr = *(struct in_addr *)(hent->h_addr);
	server_saddr.sin_port = htons(doc_port);

	//read insurance plan of patient2 from file
	fp = fopen(INSUR_FILE_PATH, "r");
	if(fp == NULL)
	{
//		perror(INSUR_FILE_PATH);
		return;
	}
	fscanf(fp, "%[^\n]", buf);
	sscanf(buf, "%*[a-z]%d", &plan);

	//send insurance plan of patient2 to doctor
	sendto(client_sock_d, buf, strlen(buf)+1, 0, (sockaddr *)&server_saddr, slen);

	//get host info of this socket
	getsockname(client_sock_d, (sockaddr *)&client_saddr, &slen);
	printf("Phase 3: Patient 2 has a dynamic UDP port number %d and IP address %s.\n", ntohs(client_saddr.sin_port), inet_ntoa(client_saddr.sin_addr));
	
	printf("Phase 3: The cost estimation request from Patient 2 with insurance plan %d has been sent to the doctor with port number %d and IP address %s.\n", plan, ntohs(server_saddr.sin_port), inet_ntoa(server_saddr.sin_addr));

	//receive estimation cost from doctor
	memset(buf, 0, BUF_SIZE);
	recvfrom(client_sock_d, buf, BUF_SIZE, 0, (sockaddr *)&server_saddr, &slen);
	printf("Phase 3: Patient 2 receives %s $ estimation cost from doctor with port number %d and name %s.\n", buf, ntohs(server_saddr.sin_port), doc_name);

	//close socket
	close(client_sock_d);

	printf("Phase 3: End of Phase 3 for Patient 2.\n");
}

//function that initials connection with server, using stream socket
int ConnectHCServer()
{
	
	int client_sock_d;
	sockaddr_in server_saddr, client_saddr;
	socklen_t slen=sizeof(client_saddr);
	hostent *hent=NULL; 

	//create stream socket
	client_sock_d = socket(AF_INET, SOCK_STREAM, 0);
	if(client_sock_d == -1)
	{
//		perror("ConnectHCServer socket()");
		return -1;
	}
	
	//fill in server socket address structure
	hent = gethostbyname(SERVER_NAME);
	server_saddr.sin_family = AF_INET;
	server_saddr.sin_addr = *(struct in_addr *)(hent->h_addr);
	server_saddr.sin_port = htons(TCP_SERVER_PORT);

	//connect to the server
	if(connect(client_sock_d, (sockaddr *)&server_saddr, sizeof(server_saddr)) == -1)
	{
//		perror("ConnectHCServer connect()");
		return -1;
	}

	//get host info of this socket
	getsockname(client_sock_d, (sockaddr *)&client_saddr, &slen);
	printf("Phase 1: Patient 2 has TCP port number %d and IP address %s.\n", ntohs(client_saddr.sin_port), inet_ntoa(client_saddr.sin_addr));

	return client_sock_d;
}

//function that deals with login authentication
int ClientLoginAuth(int client_sock_d)
{
	FILE *fp=NULL;
	char buf[BUF_SIZE]="authenticate ";
	char username[16]={0}, password[16]={0};

	//read login info of patient2 from file
	fp = fopen(LOGIN_FILE_PATH, "r");
	if(fp == NULL)
	{
		perror(LOGIN_FILE_PATH);
		return -1;
	}
	fscanf(fp, "%[^\n]", buf+strlen("authenticate ")); 
	fclose(fp);
	sscanf(buf, "%*s %s %s", username, password);

	//send authentication info
	send(client_sock_d, buf, strlen(buf)+1, 0);

	printf("Phase 1: Authentication request from Patient 2 with username %s and password %s has been sent to the Health Center Server.\n", username, password);
	
	//receive authentication result
	memset(buf, 0, BUF_SIZE);
	recv(client_sock_d, buf, BUF_SIZE, 0);
	
	if(strcmp(buf, "success") != 0)
	{
		printf("Phase 1: Patient 2 authentication result: %s.\n", buf);
		return -1;
	}

	printf("Phase 1: Patient 2 authentication result: %s.\n", buf);
	
	printf("Phase 1: End of Phase 1 for Patient 2\n");

	return 0;
}

//function that deals with appointment with the Health Center Server
int ClientMakeAppointment(int client_sock_d, char* doc_name, int* doc_port)
{
	char buf[1024]={0};
	int index_array[64], index_num;
	char s_tmp[32]={0};
	int offset=0;
	int i=0, i_tmp, flag=1;

	//send request for available list
	send(client_sock_d, "available", sizeof("available"), 0);
	
	//receive and print out available list
	recv(client_sock_d, buf, 1024, 0);
	printf("Phase 2: The following appointments are available for Patient 2:\n%s", buf);

	//store available slot index number
	while(sscanf(buf+offset, "%s %*s %*s\n", s_tmp) == 1)
	{
		index_array[i++] = atoi(s_tmp);
		offset += strlen(s_tmp) + strlen(" XXX XXXX\n");
	}
	index_num = i;

	//loop until getting a valid input of index number
	while(flag)
	{
		printf("Please enter the preferred appointment index and press enter: ");
		if(scanf("%d", &i_tmp) != 1)
		{
			scanf("%*[^\n]%*c");	//flush input buffer
			continue;
		}

		for(i=0; i<index_num; i++)
		{
			if(index_array[i] == i_tmp)
			{
				flag = 0;
				break;
			}
		}
	}

	//send message of selected index number
	snprintf(s_tmp, sizeof(s_tmp), "selection %d", i_tmp);
	send(client_sock_d, s_tmp, strlen(s_tmp)+1, 0);

	//receive result of appointment
	recv(client_sock_d, buf, sizeof(buf), 0);
	
	if(strcmp("notavailable", buf) == 0)
	{
		printf("Phase 2: The requested appointment from Patient 2 is not available. Exiting...\n");
		return -1;
	}

	sscanf(buf, "%s %d",doc_name, doc_port);
	
	printf("Phase 2: The requested appointment is available and reserverd to Patient 2. The assigned doctor port number is %d\n", *doc_port);

	return 0;
}

