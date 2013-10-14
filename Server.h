
#include <stdio.h>
#include <stdlib.h>
#include <sys/wait.h>	     /* sockets */
#include <sys/types.h>	     /* sockets */
#include <sys/socket.h>	     /* sockets */
#include <netinet/in.h>	     /* internet sockets */
#include <netdb.h>	         /* gethostbyaddr */
#include <unistd.h>	         /* fork */		
#include <stdlib.h>	         /* exit */
#include <ctype.h>	         /* toupper */
#include <signal.h>          /* signal */
#include <string.h>
#include <fcntl.h>
#include "Core.h"
#include <pthread.h>   /* For threads  */
#include <poll.h>
#define PERMS 0644
#define UNKNOWN 0
#define GETD 1
#define GETR 2
#define POST 3
#define LENGTH 4
#define BUFFSIZE 512
#include <time.h>
#include <arpa/inet.h>
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////

struct Input{
	int Socket;
	char *Ip;
};

pthread_mutex_t mtx = PTHREAD_MUTEX_INITIALIZER ;
pthread_cond_t cvar ;
CorePtr MyCore;
static struct sigaction act;
int FdPipe[2];
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
void catchkill(void);
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
void * Server_Thread(void *Arguments) ;
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
void perror_exit(char *message);
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
void Close_Thread_Simple(int Client_Socket);
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
void DecreaseTotal(int Client_Socket);
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
void Close_Thread_Total(int Client_Socket);
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
void Problem(int Client_Socket,char *Client);
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
void perror_exit(char *message);
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
int Read_All (int Fd , void *Buff , size_t Size);
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
int Write_All (int Fd , void *Buff , size_t Size) ;
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
int ParserType(char* Line, int* Number);
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
int Read_data(int Client_Socket,int* FD,int Number,char **RemoveFile);
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
int Server_Error(int Client_Socket,char *Client);
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
int Not_Found(int Client_Socket);
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
int Bad_Request(int Client_Socket,char *Client);
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////	
int Upload_Success(int Client_Socket, int Id,char *Client);
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
int Delete_Success(int Client_Socket, int Id,char *Client);
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////	
int Download_Success(int Client_Socket,int Size);
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
int Download(int Client_Socket,int Id,int Offset,char *Client);
