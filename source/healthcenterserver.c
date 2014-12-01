#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <string.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <pthread.h>

#define SERVER_PORT		21647
#define BUF_SIZE		128
#define MAX_USER_NUM	128
#define USER_INFO_PATH	"./input/users.txt"
#define DOC_SCH_PATH	"./input/availabilities.txt"
#define RESERVED		1
#define UNRESERVED		0

typedef struct sockaddr_in sockaddr_in;
typedef struct sockaddr	sockaddr;

//time slot node structure
typedef struct slot
{
	int		index;
	char	day[4];
	char	ttime[5];
	char	name[5];
	int		port_num;
	char	mark;
	struct slot		*next;
}slot;

//thread-function input parameters sturcture
typedef struct ParaList
{
	int server_c_sock_d;
	char *user_info_buf;
	int user_info_num;
	struct slot* schedule_table;
}ParaList;

int ServerInit(void);
int ServerLoginAuth(int server_c_sock_d, char* user_info_buf, int user_info_num, char* username);
int MakeAppointment(int server_c_sock_d, slot* schedule_table, char* username);
int LoadUserInfo(char* user_info_buf, int buf_size);
slot* LoadDocSchedule(void);
void* ThreadFunc(void* arg);

int main(void)
{
	int server_p_sock_d, server_c_sock_d;
	char user_info_buf[MAX_USER_NUM * BUF_SIZE];
	sockaddr_in server_c_saddr;
	socklen_t server_c_saddr_size;
	int user_info_num;
	slot *schedule_table;
	pthread_t tid;
	ParaList *p_para_list;
	
	//call ServerInit() and get parent socket descriptor 
	server_p_sock_d = ServerInit();
	if(server_p_sock_d == -1)
	{
		return -1;
	}
	
	//call LoadUserInfo() and load user info into buffer
	user_info_num = LoadUserInfo(user_info_buf, sizeof(user_info_buf));
	if(user_info_num == -1)
	{
		return -1;
	}
	
	//call LoadDocSchedule() and load schedule availablist into buffer
	schedule_table = LoadDocSchedule();

	while(1)
	{
		//call accept() and get child socket descriptor
		memset(&server_c_saddr, 0, sizeof(sockaddr_in));
		server_c_sock_d = accept(server_p_sock_d, (sockaddr *)&server_c_saddr, &server_c_saddr_size); 
		if(server_c_sock_d == -1)
		{
//			perror("Server accept()");
			return -1;
		}		
		
		//initial parameters-included structure for thread-function call
		p_para_list = NULL;
		p_para_list = (ParaList *)malloc(sizeof(ParaList));
		if(p_para_list == NULL)
		{
			return -1;
		}
		
		//fill parameters-included structure
		p_para_list->schedule_table = schedule_table;
		p_para_list->server_c_sock_d = server_c_sock_d;
		p_para_list->user_info_buf = user_info_buf;
		p_para_list->user_info_num = user_info_num;
	
		//call thread-function
		pthread_create(&tid, NULL, &ThreadFunc, p_para_list);
	}
}

//function of new thread
void* ThreadFunc(void* arg)
{
	ParaList pl = *(ParaList *)arg;
	int server_c_sock_d = pl.server_c_sock_d;
	int user_info_num = pl.user_info_num;
	char *user_info_buf = pl.user_info_buf;
	slot* schedule_table = pl.schedule_table;	
	char username[16]={0};

	free(arg);

	//login authentication
	if(ServerLoginAuth(server_c_sock_d, user_info_buf, user_info_num, username) == -1)
	{
		close(server_c_sock_d);
		return NULL;
	}
		

	//make appointment with paient
	if(MakeAppointment(server_c_sock_d, schedule_table, username) == -1)
	{
		close(server_c_sock_d);
		return NULL;
	}

	//close child socket descriptor
	close(server_c_sock_d);
	return NULL;
}

//funciton that initals server and prepares for incoming connection
int ServerInit(void)
{
	int server_p_sock_d;
	sockaddr_in server_saddr;
	
	//call socket()	
	server_p_sock_d = socket(AF_INET, SOCK_STREAM, 0);
	if(server_p_sock_d == -1)
	{
//		perror("Server socket()");
		return -1;
	}
		
	//fill server socket address structure
	memset(&server_saddr, 0, sizeof(sockaddr_in));
	server_saddr.sin_family = AF_INET;
	server_saddr.sin_port = htons(SERVER_PORT);
	server_saddr.sin_addr.s_addr = INADDR_ANY;

	if(bind(server_p_sock_d, (sockaddr *)&server_saddr, sizeof(sockaddr_in)) == -1)
	{
//		perror("Server bind()");
		return -1;
	}
	if(listen(server_p_sock_d, 128) == -1)
	{
//		perror("Server listen()");
		return -1;
	}
	
	printf("Phase 1: The Health Center Server has port number %d and IP address %s.\n", SERVER_PORT, inet_ntoa(server_saddr.sin_addr));	
	return server_p_sock_d;
}

//function that loads "user.txt" into buffer
int LoadUserInfo(char* user_info_buf, int buf_size)
{
	FILE *fp=NULL;
	int i=0, n=0;
	char *sp=NULL;

	//open "user.txt"
	fp = fopen(USER_INFO_PATH, "r");
	if(fp == NULL)
	{
//		perror(USER_INFO_PATH);
		return -1;
	}

	//store user info into buffer
	while(((BUF_SIZE*i+BUF_SIZE) <= buf_size) && !feof(fp))
	{
		sp = user_info_buf + BUF_SIZE*i++;
		memset(sp, 0, BUF_SIZE);
		fgets(sp, BUF_SIZE, fp);
		n = strlen(sp);
		if(sp[n-1] == '\n')
		{
			sp[n-1] = 0;
		}

	}

	//close file handle
	fclose(fp);
	return i-1;
}

//function that handles login authentication in the server side
int ServerLoginAuth(int server_c_sock_d, char* user_info_buf, int user_info_num, char* username)
{
	char buf[BUF_SIZE]={0};
	char password[16]={0}; 
	int i=0;

	//receive and store authentication command
	recv(server_c_sock_d, buf, BUF_SIZE, 0);
	if(strncmp(buf, "authenticate ", strlen("authenticate ")) != 0)
	{
		return -1;
	}	

	sscanf(buf, "%*s %s %s", username, password);
	printf("Phase 1: The Health Center Server has received request from a patient with username %s and password %s.\n", username, password);

	//compare received authentication info with record loaded from "user.txt"
	for(i=0; i<user_info_num; i++)
	{
		if(strcmp(user_info_buf+BUF_SIZE*i, buf+strlen("authenticate ")) == 0)
		{
			//record hit and send response "success"
			send(server_c_sock_d, "success", sizeof("success"), 0);
			printf("Phase 1: The Health Center Server sends the response success to patient with username %s.\n", username);
			return 0;	
		}
	}

	//record miss and response "failure" 
	send(server_c_sock_d, "failure", sizeof("failure"), 0);
	printf("Phase 1: The Health Center Server sends the response failure to patient with username %s.\n", username);
	return -1;	
}

//function that loads "availablities.txt" into buffer
slot* LoadDocSchedule(void)
{
	
	FILE *fp=NULL;
	slot *schedule_table=NULL, *node=NULL, *pre=NULL;
	int index, port;
	char day[4], ttime[5], name[5];

	//open file
	fp=fopen(DOC_SCH_PATH, "r");
	if(fp == NULL)
	{
//		perror(DOC_SCH_PATH);
		return NULL;
	}

	//store time slot info into a linklist 
	while(fscanf(fp, "%d %s %s %s %d\n", &index, day, ttime, name, &port) == 5)
	{
		pre = node;
		node = (slot*)malloc(sizeof(slot));
		if(node == NULL)
		{
//			perror("New Node");
			return NULL;
		}

		if(schedule_table == NULL)
		{
			schedule_table = node;
		}
		else
		{
			pre->next = node;
		}
		
		node->index = index;
		node->port_num = port;
		strcpy(node->day, day);
		strcpy(node->ttime, ttime);
		strcpy(node->name, name);
		node->mark = UNRESERVED;
		node->next = NULL;

	}
	//close file
	fclose(fp);

	return schedule_table;
}

//function that handles appointment with patient
int MakeAppointment(int server_c_sock_d, slot* schedule_table, char* username)
{
	char buf[BUF_SIZE]={0}, available_info[1024]={0};
	slot *node=schedule_table;
	int selected_index;
	sockaddr_in peer_saddr;
	socklen_t slen = sizeof(peer_saddr);

	//receive available schedule request
	recv(server_c_sock_d, buf, BUF_SIZE, 0);
	if(strcmp(buf, "available") != 0)
	{
		return -1;
	}
	
	//obtain patient connection info
	getpeername(server_c_sock_d, (sockaddr *)&peer_saddr, &slen);

	printf("Phase 2: The Health Center Server, receives a request for available time slots from patients with port number %d and IP address %s.\n", ntohs(peer_saddr.sin_port), inet_ntoa(peer_saddr.sin_addr)); 

	//fetch available time slots(unreserved slots) and store the info into buffer 
	while(node != NULL)
	{
		if(node->mark == UNRESERVED)
		{
			snprintf(available_info+strlen(available_info), 1024, "%d %s %s\n", node->index, node->day, node->ttime);
		}
		node = node->next;
	}

	//send the content of the buffer
	send(server_c_sock_d, available_info, strlen(available_info)+1, 0);

	printf("Phase 2: The Health Center Server sends available time slots to patient with username %s.\n", username);

	//receive selected slot index
	memset(buf, 0, BUF_SIZE);
	recv(server_c_sock_d, buf, BUF_SIZE, 0);
	if(strncmp("selection ", buf, strlen("selection ")) != 0)
	{
		return -1;
	}

	sscanf(buf, "%*s %d", &selected_index);

	printf("Phase 2: The Health Center Server receives a request for appointment %d from patient with port number %d and username %s.\n", selected_index, ntohs(peer_saddr.sin_port), username);

	//chect if the selected index is still available
	node = schedule_table;
	while(node != NULL)
	{
		if(node->index == selected_index)
		{
			if(node->mark == UNRESERVED)
			{
				snprintf(buf, sizeof(buf), "%s %d", node->name, (node->port_num)+647);
				send(server_c_sock_d, buf, strlen(buf)+1, 0);
				printf("Phase 2: The Health Center Server comfirms the following appointment %d to patient with username %s.\n", selected_index, username);
				node->mark = RESERVED;
			}
			else
			{
				send(server_c_sock_d, "notavailable", sizeof("notavailable"), 0);
				printf("Phase 2: The Health Center Server rejects the following appointment %d to patient with username %s.\n", selected_index, username);
				break;
			}
		}
		node = node->next;				
	}
	return 0;
}
