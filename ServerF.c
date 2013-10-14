#include "Server.h"
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
void catchkill(void){//Signal Handler for child, when signal arrives, write to global pipe
	int Status=1; 

 	if( write (FdPipe[1],&Status,sizeof(int) )!=sizeof(int)){perror("write1");printf("Write Problem!\n");}
    
	if(sem_wait(& (MyCore)->Sem_Total)){perror("sem wait"); exit(1);}
		MyCore->TerminationFlag=1;
	if(sem_post(& (MyCore)->Sem_Total)){perror("sem post"); exit(1);}
}
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
int main(int argc, char *argv[]) {
        int ClientIp;
        pthread_t thr;
        int Port, Socket, Client_Socket;
        struct sockaddr_in Server,Client;
        socklen_t Client_len;
        struct sockaddr *ServerPtr=(struct sockaddr *)&Server;
        struct sockaddr *ClientPtr=(struct sockaddr *)&Client;
        struct Input *Arguments;	
        char TimeBuffer[BUFFSIZE];
        struct pollfd fdt[2];  
        char *IpBuffer;
        int must_flag=0,index,c;
        char *value=NULL,*hay_file=NULL;
	Client_len=sizeof(Client);
        while ((c = getopt (argc, argv, "f:p:")) != -1)//Read inline arguments, all possible cases are dealt with
            switch (c){
                case 'f':
                    value = optarg;
                    if (value==NULL){fprintf(stdout,"Please give a file with -f flag.\n");return -1;}
                    must_flag++;
                    if ((hay_file = malloc((int)strlen(value) + 1)) == NULL) {perror("malloc");  return 1;}
                    strcpy(hay_file,value);
                    //fprintf(stdout, "Haystack file is: %s \n",hay_file);
                    break;
                case 'p':
                    value = optarg;
                    if (value==NULL){fprintf(stdout,"Please give a port number with the -p flag.\n");return 1;}
                    Port=atoi(value);
                    //fprintf(stdout, "Given port is: %d \n",Port);
                    must_flag++;
                    break;
            }

        for (index = optind; index < argc; index++)
            printf ("Non-option argument %s\n", argv[index]);	
        if( must_flag!=2){ printf("Not enough parameters given\n"); return 1;}
	
        PrintOutput(TimeBuffer);
        printf("[%s] :SERVER IS STARTING ON PORT:%d\n",TimeBuffer,Port);
        PrintOutput(TimeBuffer);
        printf("[%s] :INITIATING INTERNAL STRUCTURES\n",TimeBuffer);

        if (pipe(FdPipe) == -1) { perror("pipe "); return(-1); }/*Temporray pipe for polling and critical section*/
        pthread_cond_init (&cvar , NULL ); /* Initialize condition variable */

        act.sa_handler = (void*)catchkill; 
        sigemptyset (&(act.sa_mask)); 
        sigaddset (&(act.sa_mask), SIGINT); 
        sigaction ( SIGINT, &act , NULL ) ;

        if( pthread_sigmask(SIG_BLOCK,&(act.sa_mask),NULL)==-1 ){perror("blocking signals error");return -1;}
        /* Create socket */
        if ((Socket = socket(PF_INET, SOCK_STREAM, 0)) < 0)
        	perror_exit("socket");
 	
        fdt[0].fd=Socket; //Set up socket that client writes too
        fdt[0].revents=0;
        fdt[0].events=POLLIN;
    	fdt[1].fd=FdPipe[0]; //Set up poll for the pipe the signal writes too
        fdt[1].revents=0;
        fdt[1].events=POLLIN;
    	
    	Server.sin_family = AF_INET;       /* Internet domain */
    	Server.sin_addr.s_addr = htonl(INADDR_ANY);
    	Server.sin_port = htons(Port);      /* The given Port */
    	/* Bind socket to address */
    	if (bind(Socket, ServerPtr, sizeof(Server)) < 0)
    		perror_exit("bind");
    	/* Listen for connections */
    	if (listen(Socket, 10) < 0) perror_exit("listen");
	
        /*Initialize everthing, sempahores, haystack files, index... */
        CoreInitiate(&MyCore,10,hay_file);
        
        PrintOutput(TimeBuffer);
        printf("[%s] :SERVER IS READY FOR NEW CLIENTS\n",TimeBuffer);
	
    	while (1) {
        	if( pthread_sigmask(SIG_UNBLOCK,&(act.sa_mask),NULL)==-1 ){perror("unblocking signals error");return -1;}
                /*Poll socket for accept's and temporary pipe for any signals caught*/
                poll(fdt,2, -1);

            if( pthread_sigmask(SIG_BLOCK,&(act.sa_mask),NULL)==-1 ){perror("blocking signals error");return -1;}

            if (fdt[0].revents & POLLIN){/*Client has sent Connection Request*/
                    Arguments=malloc(sizeof(struct Input));
                    IpBuffer=malloc(sizeof(char)*BUFFSIZE);
                    /*Accept connection */
                    if ((Client_Socket = accept(Socket, ClientPtr, &Client_len)) < 0) perror_exit("accept");
    		
                    /*Get ip of request*/
                    ClientIp= Client.sin_addr.s_addr;
                    inet_ntop( AF_INET, &ClientIp, IpBuffer, INET_ADDRSTRLEN);
                
                    Arguments->Socket=Client_Socket;
                    Arguments->Ip=IpBuffer;
                    /*Create Thread to serve request*/
                    if (pthread_create(&thr, NULL, Server_Thread,(void*)Arguments)==-1 ){ /* New thread */
                    if(Server_Error(Client_Socket,IpBuffer)==-1){printf("server error print problem");return -1;}
                        perror("pthread_create");	 
                        return -1;
                    }
            }
            if (fdt[1].revents & POLLIN){/*Signal is sent and caught*/
                PrintOutput(TimeBuffer);
                printf("[%s] :SERVER IS SHUTTING DOWN.SERVER WILL NOT ACCEPT ANY OTHER CLIENTS\n",TimeBuffer);
                if(sem_wait(& (MyCore)->Sem_Total)){perror("sem wait"); return -1;}
				if(MyCore->Total_Threads>0){/*If threads alive*/

                        if (pthread_mutex_lock (& mtx )==-1) { /* Lock mutex */
                            perror(" pthread_mutex_lock " );  
                            return -1;
                        }	
                        if(sem_post(& (MyCore)->Sem_Total)){perror("sem post"); return -1;}
                        PrintOutput(TimeBuffer);
                        printf("[%s] :WAITING FOR RUNNING CLIENTS TO BE SERVED....\n",TimeBuffer);
                        
                        /*Process freezes until last thread signals*/
                        pthread_cond_wait (&cvar , &mtx );
                        if (pthread_mutex_unlock (& mtx )==-1) { /* Unlock mutex */
                            perror (" pthread_mutex_unlock " ); 
                            return -1;	
                        }

				}
				else{
                        if(sem_post(& (MyCore)->Sem_Total)){perror("sem post"); return -1;}
				}
			
			break;

            }
        }
   	
        PrintOutput(TimeBuffer);
        printf("[%s] :CLEANING UP...\n",TimeBuffer);

        /*Release everything*/
    	close(Socket);
        CoreDelete(&MyCore);
        free(hay_file);
        if (pthread_cond_destroy (& cvar )==-1) {
            perror (" pthread_cond_destroy " ); exit (1) ; 
        }
        PrintOutput(TimeBuffer);
        printf("[%s] :ADIOS AMIGOS, BUEN VERANO!!!\n",TimeBuffer);
        return 0;	
}
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
void Close_Thread_Simple(int Client_Socket){ 
	if(close(Client_Socket)==-1){perror("Closing socket error");}
	pthread_exit("Null");
}
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
void DecreaseTotal(int Client_Socket){
        if(sem_wait(& (MyCore)->Sem_Total)){perror("sem wait"); Close_Thread_Simple(Client_Socket);}
            /*Decrease total and signal if reaches 0*/
            MyCore->Total_Threads--;
            if(MyCore->TerminationFlag==1 && MyCore->Total_Threads==0){
	
					if (pthread_mutex_lock (& mtx )==-1) { /* Lock mutex */
						perror(" pthread_mutex_lock " );  
						Close_Thread_Simple(Client_Socket);
					}

					if(sem_post(& (MyCore)->Sem_Total)){perror("sem post"); Close_Thread_Simple(Client_Socket);}

					pthread_cond_signal (& cvar ); /* Awake other thread */

					if (pthread_mutex_unlock (& mtx )==-1) { /* Unlock mutex */
						perror (" pthread_mutex_unlock " ); 
						Close_Thread_Simple(Client_Socket);	
					}
            }
            else{
                if(sem_post(& (MyCore)->Sem_Total)){perror("sem post"); Close_Thread_Simple(Client_Socket);}
            }
}
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
void Close_Thread_Total(int Client_Socket){ 
	if(close(Client_Socket)==-1){perror("closing socket error\n");}
	DecreaseTotal(Client_Socket);
	pthread_exit("Null");
}
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
void Problem(int Client_Socket,char *Client){
	printf("------------------------------->Problem arised\n");
	if(Server_Error(Client_Socket,Client)==-1){printf("server error print problem");}
	Close_Thread_Simple(Client_Socket);
	DecreaseTotal(Client_Socket);
}
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
void * Server_Thread(void *Arguments) { 
        struct Input *Stats=(struct Input *)Arguments;
        int Client_Socket=Stats->Socket;
        char *Client=Stats->Ip;
        char TimeBuffer[BUFFSIZE];
    	int i=0, B_size=BUFFSIZE,FD,Id;
    	int flag=0,Number,IdRequest,Result,Request=UNKNOWN,DataFlag=0,Res,Offset;
        char *RemoveFile;
    	char* Current_Line; 	
    	char buf[1];
        int err ;
        
        if( pthread_sigmask(SIG_BLOCK,&(act.sa_mask),NULL)==-1 ){perror("blocking signals error");Close_Thread_Simple(Client_Socket);}

        //P(SemFile)
        if(sem_wait(& (MyCore)->Sem_Total)){perror("sem wait"); Close_Thread_Simple(Client_Socket);}
            MyCore->Total_Threads++;
        if(sem_post(& (MyCore)->Sem_Total)){perror("sem post"); Close_Thread_Simple(Client_Socket);}
        //V(SemFile)
    
        PrintOutput(TimeBuffer);
        printf("[%s] :NEW SERVER THREAD CREATED FOR CLIENT |%s|\n",TimeBuffer,Client);

        if( (err = pthread_detach ( pthread_self ()))==-1) { /* Detach thread */
            perror (" pthread_detach ");
            Problem(Client_Socket,Client);
        }
	
        if( (Current_Line=malloc(sizeof(char)*B_size))==NULL){perror("InitMatrix");Problem(Client_Socket,Client);}
        while( (Res=Read_All(Client_Socket, buf, 1)) >0){  /* Deal with 1 request */
                if(i>B_size){ if( (Current_Line=realloc(Current_Line, B_size*2))==NULL){perror("InitMatrix");Problem(Client_Socket,Client);} }
                Current_Line[i]=buf[0];
                if(flag==1){
                    if(buf[0]=='\n'){ /*Find kind of request*/
                        if(i==1)
                            break;
                            Current_Line[i-1]='\0';
                            Number=0;
                            Result=ParserType(Current_Line,&Number);
                            switch(Result){
                                case(POST):
                                    if(Request==UNKNOWN){
                                        Request=POST;
                                    }
                                    break;
                                case(LENGTH):
                                    if(Request==POST){
                                        DataFlag=1;
                                    }
                                    break;
                                case(GETD):
                                    if(Request==UNKNOWN){
                                        Request=GETD;
                                        IdRequest=Number;
                                    } 
                                    break;
                                case(GETR):	
                                    if(Request==UNKNOWN){
                                        Request=GETR;
                                        IdRequest=Number; 
                                    }
                                    break;
                            }
    				i=-1;
    			}
    		}
    		if(buf[0]=='\r') 
    			flag=1;
    		i++;
    	}
    	free(Current_Line);
    	if(Res==-1){ printf("read all problem\n");Problem(Client_Socket,Client);}
    	if(Res==0){ printf("Http Request error\n");Problem(Client_Socket,Client);}

        switch(Request){ /*Deal with request*/
                case(UNKNOWN):
                    if(Bad_Request(Client_Socket,Client)==-1){printf("bad request problem\n");Close_Thread_Total(Client_Socket);} break;
                case(POST):
                    if(DataFlag==1){/*if post and length where inside given http request*/
                        if(Number <= 0){
                            if(Bad_Request(Client_Socket,Client)==-1){printf("Not_Found print problem");Close_Thread_Total(Client_Socket);}
                                    break;
                        }
                        PrintOutput(TimeBuffer);
                        printf("[%s] :CLIENT -%s- REQUESTED TO UPLOAD A FILE WITH LENGTH=%d\n",TimeBuffer,Client,Number);
                        /*Read image into file*/
                        Read_data(Client_Socket,&FD,Number,&RemoveFile);
                        /*Add image into index/haystackfile*/
                        Id=CoreAdd(MyCore,FD,Number);
                        if(Id==-1){if(Server_Error(Client_Socket,Client)==-1){printf("server error print problem");Close_Thread_Total(Client_Socket);}}
                        if(Upload_Success( Client_Socket, Id,Client)==-1){printf("upload succ problem\n");Close_Thread_Total(Client_Socket);}
                        close(FD); 
                        if(remove(RemoveFile)==-1){perror("REMOVE PROBLEM"); Problem(Client_Socket,Client);}
                        free(RemoveFile);
                    }
                    break;	
                case(GETD):	
                    /*Check if given id exists*/
                    Offset=CoreExists(MyCore,IdRequest);
                    PrintOutput(TimeBuffer);
                    printf("[%s] :CLIENT -%s- REQUESTED DOWNLOAD WITH ID : %d\n",TimeBuffer,Client,IdRequest);
                    if(Offset>0){
                        //P(SemFile);
                        if(sem_wait(& (MyCore)->Sem_File)){perror("sem wait"); Problem(Client_Socket,Client);}
                        /*Sent requested image to client*/
                        if(Download(Client_Socket,IdRequest,Offset,Client)==-1){
                            if(sem_post(& (MyCore)->Sem_File)){perror("sem post"); Problem(Client_Socket,Client);}
                            PrintOutput(TimeBuffer);
                            printf("[%s] :CLIENT -%s- DOWNLOAD ERROR, FILE WITH ID=%d \n",TimeBuffer,Client,IdRequest);
                            Close_Thread_Total(Client_Socket);
                        }
                        if(sem_post(& (MyCore)->Sem_File)){perror("sem post"); Problem(Client_Socket,Client);}
                        //V(SemFile);
                    }
                    else if(Offset==-1){
                        if(Server_Error(Client_Socket,Client)==-1){printf("server error print problem");Close_Thread_Total(Client_Socket);}
                    }
                    else{
                        if(Not_Found(Client_Socket)==-1){printf("server error Not_Found print problem");Close_Thread_Total(Client_Socket);}
                        PrintOutput(TimeBuffer);
                        printf("[%s] :CLIENT -%s- DOWNLOAD FAILED, FILE WITH ID=%d WAS NOT FOUND\n",TimeBuffer,Client,IdRequest);
                    }
                    break;
    		case(GETR):
                    PrintOutput(TimeBuffer);
                    printf("[%s] :CLIENT -%s- REQUESTED DELETE OF FILE WITH ID=%d\n",TimeBuffer,Client,IdRequest);
                    /*Call function that checks if id exists and then removes from index/file*/
                    Res=CoreRemove(MyCore,IdRequest);
                    switch(Res){
                        case(-1):
                                if(Server_Error(Client_Socket,Client)==-1){printf("server error print problem");Close_Thread_Total(Client_Socket);}
                                break;
                        case(1):
                                if(Not_Found(Client_Socket)==-1){printf("Not_Found print problem");Close_Thread_Total(Client_Socket);}
                                PrintOutput(TimeBuffer);
                                printf("[%s] :CLIENT -%s- REMOVE FAILED,  FILE WITH ID=%d WAS NOT FOUND\n",TimeBuffer,Client,IdRequest);
                                break;
                        case(0):
                                if(Delete_Success(Client_Socket,IdRequest,Client)==-1){printf("Not_Found print problem");Close_Thread_Total(Client_Socket);}
                                break;
    				}			
            break;
    	}	
    	//CorePrint(MyCore);
		//if(sem_wait(& (MyCore)->Sem_File)){perror("sem wait"); Problem(Client_Socket);}
		//PrintHay(MyCore->Fd);
		//if(sem_post(& (MyCore)->Sem_File)){perror("sem post"); Problem(Client_Socket);}

    	
	if(close(Client_Socket)==-1){perror("cant close socket");DecreaseTotal(Client_Socket);pthread_exit(NULL);}	  /* Close socket */
	DecreaseTotal(Client_Socket);
	return NULL;
	
	
}
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
void perror_exit(char *message){
    perror(message);
    exit(EXIT_FAILURE);
}
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
int Read_All (int Fd , void *Buff , size_t Size) {/*Function that reads from socket unless error*/
	int Rec , N;
	for (Rec = 0; Rec < (int)Size ; Rec +=N) {
		if ((N = read(Fd , Buff+Rec, Size - Rec)) == -1)
			return -1; /* error */
		else if(N==0)
			return Rec;
	}
	return Rec;
}
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
int Write_All (int Fd , void *Buff , size_t Size) {/*Function that writes to socket unless error*/
	int Sent , N;
	for (Sent = 0; Sent < (int)Size ; Sent +=N) {
		if ((N = write(Fd , Buff+Sent , Size - Sent)) == -1)
			return -1; /* error */
	}
	return Sent;
}
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
int ParserType(char* Line, int* Number){ /*Parse a line into 3 categories: Post,Get and Length*/
	char *Get="GET",*Post="POST",*Length="Content-Length:";
	int i,j,k=0,flag=0;
	char Temp[BUFFSIZE];
	
    for(i=0;i<(int)strlen(Line);i++){
		if(Line[i]==' '){
			Temp[i]='\0';
			flag=1;
			break;
		}
		Temp[i]=Line[i];
	}
	if(flag==1){
		if( !strcmp(Temp,Get)){ 
			if(Line[i+1]!='/' && Line[i+2]!='?'){
				return UNKNOWN;
			}
			i+=3;
			if(Line[i]=='i'&& Line[i+1]=='d' && Line[i+2]=='='){
				i+=3;
				for(j=i;j<=(int)strlen(Line);j++){
					if(Line[j]==' ') break;
					Temp[k]=Line[j];
					k++;
				}
				Temp[k]='\0';				
				*Number=atoi(Temp); 
				return GETD;
			}
			else if(Line[i]=='d'&& Line[i+1]=='_' && Line[i+2]=='i' && Line[i+3]=='d' && Line[i+4]=='='){
				i+=5;
				for(j=i;j<=(int)strlen(Line);j++){
					if(Line[j]==' ') break;
					Temp[k]=Line[j];
					k++;
				}
				Temp[k]='\0';
				*Number=atoi(Temp); 						
				return GETR;
			}
		}
		if( !strcmp(Temp,Post)){ 
			return POST;
		}
		if( !strcmp(Temp,Length)){ 
			for(j=i+1;j<=(int)strlen(Line);j++){
				if(Line[j] !='\0'){
					Temp[k]=Line[j];
					k++;
				}
				else{ 
					Temp[k]='\0';
					break;
				}
			}
			*Number=atoi(Temp);
			return LENGTH;
		}
	}
	return UNKNOWN;
}
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
int Read_data(int Client_Socket,int* FD,int Number,char **RemoveFile){
	int j,Loop,Remaining;
	char  Buffer[BUFFSIZE];
    
    /*Create Temporary file to read image from socket too*/
	(*RemoveFile)=malloc(sizeof(char)*sizeof(int)+12);
	sprintf((*RemoveFile),"%ld",(-1)*pthread_self());
   	if (( *FD= open((*RemoveFile), O_CREAT |O_RDWR , PERMS ))== -1){perror("creating");return -1;}
    Loop=Number/BUFFSIZE;
	Remaining=Number%BUFFSIZE;
	for(j=0;j<Loop;j++){
		if (Read_All(Client_Socket, Buffer , BUFFSIZE) < BUFFSIZE){printf("read from file problem\n");return -1;}
		if (Write_All(*FD,Buffer,BUFFSIZE) < BUFFSIZE){printf("write from file problem\n");return -1;}
	}
	if (Read_All(Client_Socket, Buffer , Remaining) < Remaining){printf("read from file problem\n");return -1;}
	if (Write_All(*FD, Buffer , Remaining) < Remaining){printf("write from file problem\n");return -1;}
	lseek(*FD,0,SEEK_SET);
	return 0;
}
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
int Server_Error(int Client_Socket,char *Client){
	char Answer[BUFFSIZE];
	char TimeBuffer[BUFFSIZE];

	sprintf(Answer, "%s","HTTP/1.0 500 Internal Server Error\r\nServer: haystack_Server v1.0\r\nConnection: close\r\n\r\n");
    if(Write_All(Client_Socket,Answer,(int)strlen(Answer)) < (int)strlen(Answer)){printf("write from file problem\n");return -1;}
	PrintOutput(TimeBuffer);
	printf("[%s] :CLIENT -%s- SERVER ERROR\n",TimeBuffer,Client);
	return 0;
}
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
int Not_Found(int Client_Socket){
	char Answer[BUFFSIZE];
    
	sprintf(Answer, "%s","HTTP/1.0 404 Not Found\r\nServer: haystack_Server v1.0\r\nConnection: close\r\n\r\n");
    if(Write_All(Client_Socket,Answer,(int)strlen(Answer)) < (int)strlen(Answer)){printf("write from file problem\n");return -1;}
	return 0;	
}
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
int Bad_Request(int Client_Socket,char *Client){
	char TimeBuffer[BUFFSIZE];
	char Answer[BUFFSIZE];
    
	sprintf(Answer, "%s","HTTP/1.0 400 Bad request\r\nServer: haystack_Server v1.0\r\nConnection: close\r\n\r\n");
    if(Write_All(Client_Socket,Answer,(int)strlen(Answer)) < (int)strlen(Answer)){printf("write from file problem\n");return -1;}
	PrintOutput(TimeBuffer);
	printf("[%s] :CLIENT -%s- REQUEST ERROR\n",TimeBuffer,Client);
	return 0;
}
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////	
int Upload_Success(int Client_Socket, int Id,char *Client){
	char TimeBuffer[BUFFSIZE];
	char A_String[BUFFSIZE];
	char Answer[BUFFSIZE];
	int A_Size;	 	
    
    sprintf(A_String,"%s%d","File successfully uploaded with id : ",Id);
    A_Size=(int)strlen(A_String);
    sprintf(Answer, "%s%d%s%s","HTTP/1.0 200 OK\r\nServer: haystack_Server v1.0\r\nConnection: close\r\nContent-Type: text\r\nContent-Length: ",A_Size,"\r\n\r\n",A_String);
	if(Write_All(Client_Socket,Answer,(int)strlen(Answer)) < (int)strlen(Answer)){printf("write from file problem\n");return -1;}
	PrintOutput(TimeBuffer);
	printf("[%s] :CLIENT -%s- FILE WITH ID=%d UPLOADED SUCCESSFULLY\n",TimeBuffer,Client,Id);
	return 0;
}
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
int Delete_Success(int Client_Socket, int Id,char *Client){
	char A_String[BUFFSIZE];
	char TimeBuffer[BUFFSIZE];
	char Answer[BUFFSIZE];
	int A_Size;	 	
    sprintf(A_String,"%s%d","File successfully deleted with id : ",Id);
    A_Size=(int)strlen(A_String);
    sprintf(Answer, "%s%d%s%s","HTTP/1.0 200 OK\r\nServer: haystack_Server v1.0\r\nConnection: close\r\nContent-Type: text\r\nContent-Length: ",A_Size,"\r\n\r\n",A_String);
	if(Write_All(Client_Socket,Answer,(int)strlen(Answer)) < (int)strlen(Answer)){printf("write from file problem\n");return -1;}
	PrintOutput(TimeBuffer);
	printf("[%s] :CLIENT -%s- FILE WITH ID=%d SUCCESSFULLY DELETED\n",TimeBuffer,Client,Id);
	return 0;
}
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////	
int Download_Success(int Client_Socket,int Size){
	char Answer[BUFFSIZE];	 	
    	
    sprintf(Answer, "%s%d%s","HTTP/1.0 200 OK\r\nContent-Type: image/jpeg\r\nContent-Length: ",Size," \r\nConnection: close\r\n\r\n");
	if(Write_All(Client_Socket,Answer,(int)strlen(Answer)) < (int)strlen(Answer)){printf("write from file problem\n");return -1;}

	return 0;
}
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
int Download(int Client_Socket,int Id,int Offset,char *Client){
	int Temp,Loop,j,Size,Remaining;
	char Buffer[BUFFSIZE];
	char TimeBuffer[BUFFSIZE];
    
	/*Read Needle info*/
	if(lseek(MyCore->Fd,Offset,SEEK_SET)==-1){perror("lseek");return -1;}
	
	if(read(MyCore->Fd,&Temp,sizeof(int))!=sizeof(int)){perror("read id");return -1;}	
	if(read(MyCore->Fd,&Temp,sizeof(int))!=sizeof(int)){perror("read state");return -1;}
	if(Temp==0){
		if(Not_Found(Client_Socket)==-1){printf("Not_Found error print problem");return -1;}
		return 0;
	}
	if(read(MyCore->Fd,&Size,sizeof(int))!=sizeof(int)){perror("read size");return -1;}
    /*Create Http Response*/
	if(Download_Success(Client_Socket,Size)==-1){printf("Download_Success error print problem");return -1;}
    /*Write Data to response*/
	Loop=Size/BUFFSIZE;
	Remaining=Size%BUFFSIZE;
	for(j=0;j<Loop;j++){
		if ( Read_All(MyCore->Fd, Buffer , BUFFSIZE) < BUFFSIZE){perror("read from file");return -1;}
		if ( Write_All(Client_Socket,Buffer,BUFFSIZE) < BUFFSIZE){perror("write to file");return -1;}	
	}
	if (Read_All(MyCore->Fd, Buffer , Remaining) < Remaining){perror("read from file");return -1;}
	if ( Write_All(Client_Socket,Buffer,Remaining) < Remaining){perror("write to file");return -1;}
	PrintOutput(TimeBuffer);
	printf("[%s] :Client -%s- SUCCEFULLY DOWNLOADED FILE WITH ID=%d.TOTAL BYTES TRANSFERED %d \n",TimeBuffer,Client,Id,Size);
	return 0;
}
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////

