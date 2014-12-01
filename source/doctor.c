#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <arpa/inet.h>

typedef struct sockaddr_in sockaddr_in;
typedef struct sockaddr sockaddr;
typedef struct insurance_ls
{
	char	path[64];
	int		insurance1;
	int		insurance2;
	int		insurance3;
}insurance_ls;

#define BUF_SIZE		128
#define DOC1_PORT_NUM	41647
#define DOC2_PORT_NUM	42647
#define DOC1_FILE		"./input/doc1.txt"
#define DOC2_FILE		"./input/doc2.txt"

int ServerInit(int name, int port_num, insurance_ls* insurance_buf);
void CommWithPatient(int server_sock_d, int name, insurance_ls ils);



//function that initials server: initial communication socket, read "doc#.txt" and store insurance info into buffer
int ServerInit(int name, int port_num, insurance_ls* insurance_buf)
{
	int server_sock_d;
	sockaddr_in server_saddr;
	int ret;
	FILE *fp=NULL;

	//create socket
	server_sock_d = socket(AF_INET, SOCK_DGRAM, 0);
	if(server_sock_d == -1)
	{
//		perror("ServerInit socket()");
		return -1;
	}

	//fill in socket address structure
	server_saddr.sin_family = AF_INET;
	server_saddr.sin_addr.s_addr = INADDR_ANY;
	server_saddr.sin_port = htons(port_num);

	//bind socket
	ret = bind(server_sock_d, (sockaddr *)&server_saddr, sizeof(sockaddr_in));
	if(ret == -1)
	{
//		perror("ServerInit bind()");
		return -1;
	}

	printf("Phase 3: Doctor %d has a static UDP port %d and IP address %s.\n", name, port_num, inet_ntoa(server_saddr.sin_addr));
	
	//open file
	fp = fopen(insurance_buf->path, "r");
	if(fp == NULL)
	{
//		perror(insurance_buf->path);
		return -1;
	}

	//store file data into memory
	fscanf(fp, "%*s %d\n", &(insurance_buf->insurance1));
	fscanf(fp, "%*s %d\n", &(insurance_buf->insurance2));
	fscanf(fp, "%*s %d\n", &(insurance_buf->insurance3));

	return server_sock_d;
}

//function that deals with communication with patient
void CommWithPatient(int server_sock_d, int name, insurance_ls ils)
{
	sockaddr_in client_saddr;
	socklen_t slen;
	char buf[BUF_SIZE]={0};
	int plan=0, price=0;

	//receive insurance cost inquery from patient
	recvfrom(server_sock_d, buf, BUF_SIZE, 0, (sockaddr *)&client_saddr, &slen);
	
	//check insurance plan
	if(strcmp(buf, "insurance1") == 0)
	{
		plan = 1;
		price = ils.insurance1;
	}
	else if(strcmp(buf, "insurance2") == 0)
	{
		plan = 2;
		price = ils.insurance2;
	}
	else if(strcmp(buf, "insurance3") == 0)
	{
		plan = 3;
		price = ils.insurance3;
	}
	else
	{
		return;
	}

	printf("Phase 3: Doctor %d receives the request from the patient with port number %d and the insurance plan %d.\n", name, ntohs(client_saddr.sin_port), plan); 

	//send cost of insurance plan
	sprintf(buf, "%d", price);
	sendto(server_sock_d, buf, strlen(buf)+1, 0, (sockaddr *)&client_saddr, slen);

	printf("Phase 3: Doctor %d has sent estimated price %d $ to patient with port number %d.\n", name, price, ntohs(client_saddr.sin_port));
}
