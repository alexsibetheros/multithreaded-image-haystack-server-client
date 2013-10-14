#include <stdio.h> 
#include <stdlib.h> 
#include <string.h> 
#include <fcntl.h>
#include "Core.h" 
#include <errno.h>
#define PERMS 0644
#define MagicNumber 76
#define BUFFSIZE 256
#include "Hay.h"

////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
int CheckFile(char* WorkFile,int* FileDes){/*Return 0 if file changed in some way, 1 if file is ok*/
	
	int Num;
	int Result;
	char TimeBuffer[BUFFSIZE];
	Result=access(WorkFile, F_OK);/*Check if files exists*/
	errno=0;
	

	if (Result == 0){/*If exists, check if type haystack*/
		if (( *FileDes = open(WorkFile , O_RDWR , PERMS ))== -1){perror("creating");return -1;}	
		if(read(*FileDes,&Num,sizeof(int))!=sizeof(int)){perror("read Magic");return -1;}	
		
		
		if( Num!=MagicNumber){
			if(remove(WorkFile)==-1){perror("remove");return -1;}


			PrintOutput(TimeBuffer);
			printf("[%s] :{%s} IS NOT A HAYSTACK FILE,REMOVING IT AND MAKING A NEW ONE WITH THE SAME NAME\n",TimeBuffer,WorkFile);

			if (( *FileDes = open(WorkFile , O_CREAT|  O_RDWR  , PERMS ))== -1){perror("creating");return -1;}
			
			return 0;
		}
		PrintOutput(TimeBuffer);
		printf("[%s] :{%s} IS IN VALID HAYSTACK FORM\n",TimeBuffer,WorkFile);
		return 1;
	}
	PrintOutput(TimeBuffer);
	printf("[%s] :{%s} DOES NOT EXIST,CREATING HAYSTACK FILE\n",TimeBuffer,WorkFile);
	
	if (( *FileDes = open(WorkFile , O_CREAT|  O_RDWR  , PERMS ))== -1){perror("creating");return -1;}
	return 0;

}
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
int InitializeHayStack(char *WorkFile,CorePtr Core){
	if(WorkFile==NULL) return -1;
	int FileDes,NewDes;
	int Result;
	int num=MagicNumber;
	char TimeBuffer[BUFFSIZE];
	char Name[strlen(WorkFile)+strlen("New_")+1];

	/*Check if Haystack*/
	if( (Result=CheckFile( WorkFile, &FileDes))==-1){printf("Result problem\n");return -1;}

	if(Result==1){/*If checkfile returned 1, files need compaction*/
		
		
		sprintf(Name,"%s%s","New_",WorkFile);

		PrintOutput(TimeBuffer);
		printf("[%s] :BOOTSTRAP COMPACTION IN TO NEW FILE :|%s|\n",TimeBuffer,Name);
	
		if( access(Name, F_OK)==0){PrintOutput(TimeBuffer); printf("[%s] :NEW FILE NAME EXISTS,OVERWRITTING IT\n",TimeBuffer); remove(Name); }
		errno=0;
		if (( NewDes = open(Name ,O_CREAT|  O_RDWR, PERMS ))== -1){perror("creating");return -1;}
		
		if(Compaction(FileDes,NewDes, Core)==-1){printf("Compaction problem\n");return -1;}
		
		return NewDes;
	}
	else if(Result==0){/*Add starting data to new file*/
		if(write(FileDes, &num, sizeof(int))!=sizeof(int)){perror("write magic number");return -1;}
		num=0;
		if(write(FileDes, &num, sizeof(int))!=sizeof(int)){perror("write total");return -1;}
		return FileDes;
	}
	
	return -1;//check FIX
	
	
}
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
int AddNeedleToFile(int FileDes,int BufferDes,int Id,int Size,int* Offset){
	
	int Total,State=1,Nread;
	char Buffer[BUFFSIZE];
	/*Write classic info*/
	if(lseek(FileDes,sizeof(int),SEEK_SET)==-1){perror("lseek");return -1;}
	if(read(FileDes,&Total,sizeof(int))!=sizeof(int)){perror("read total");return -1;}
	Total++;
	if(lseek(FileDes,sizeof(int),SEEK_SET)==-1){perror("lseek");return -1;}
	if(write(FileDes, &Total, sizeof(int))!=sizeof(int)){perror("write total");return -1;}
	if( (*Offset=lseek(FileDes,0,SEEK_END))==-1){perror("lseek");return -1;}
	
	if(write(FileDes, &Id, sizeof(int))!=sizeof(int)){perror("write Id");return -1;}
	if(write(FileDes, &State, sizeof(int))!=sizeof(int)){perror("write state");return -1;}
	if(write(FileDes, &Size, sizeof(int))!=sizeof(int)){perror("write size");return -1;}
	/*Write Jpeg*/
	
	while ( (Nread =read (BufferDes, Buffer , BUFFSIZE ) ) > 0 ){
		if ( write(FileDes,Buffer,Nread) < Nread){perror("write to file");return -1;}
	}	
	if(Nread==-1){perror("read");return -1;}
	return 0;
}
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
int DeleteNeedle(int FileDes,int Offset){
	int State=0,Total;
	/*Change info of Superblock*/
	if(lseek(FileDes,sizeof(int),SEEK_SET)==-1){perror("lseek");return -1;}
	if(read(FileDes,&Total,sizeof(int))!=sizeof(int)){perror("delete read total");return -1;}
	Total--;
	if(lseek(FileDes,sizeof(int),SEEK_SET)==-1){perror("lseek");return -1;}
	if(write(FileDes, &Total, sizeof(int))!=sizeof(int)){perror("write total");return -1;}
	/*Change status of needle*/
	if(lseek(FileDes,Offset+sizeof(int),SEEK_SET)==-1){perror("lseek");return -1;}
	if(write(FileDes, &State, sizeof(int))!=sizeof(int)){perror("write total");return -1;}
	return 0;
}
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
/*int PrintHay(int FileDes){
	printf("\n-------------------------PRINTING HAYSTACK FILE----------------\n");
	int i,Total,Num,Id,State,Size,FD,Loop, Remaining;
	int Buffer[BUFFSIZE],j;
	char Name[(sizeof(int)*2)+1+strlen(".jpeg")];
	if(lseek(FileDes,0,SEEK_SET)==-1){perror("lseek");return -1;}
	
	if(read(FileDes,&Num,sizeof(int))!=sizeof(int)){perror("read Magic");return -1;}	
	printf("Magic Num= |%d|\n",Num);
	if( Num!=MagicNumber){
		printf("Not a HayStack file\n");
		return 1;
	}
	
	if(read(FileDes,&Total,sizeof(int))!=sizeof(int)){perror("print read total");return -1;}
	printf("Total Needles= |%d|\n",Total);
	
	
	for( i=0; i<Total;i++){
		//sprintf(Name,"%d%s",getpid()+i,".jpeg");
		
		//if (( FD= open(Name, O_CREAT |O_RDWR , PERMS ))== -1){perror("creating");return -1;}
		
		if(read(FileDes,&Id,sizeof(int))!=sizeof(int)){perror("read id");return -1;}
		if(read(FileDes,&State,sizeof(int))!=sizeof(int)){perror("read state");return -1;}
		if(read(FileDes,&Size,sizeof(int))!=sizeof(int)){perror("read size");return -1;}
		if( State==0){ 
			//printf("Found Deleted: ID=[%d] State=[%d] Size=[%d]\n",Id,State,Size);
			i--; 
			if(lseek(FileDes,Size,SEEK_CUR)==-1){perror("lseek");return -1;}
			continue;
		}
			if(lseek(FileDes,Size,SEEK_CUR)==-1){perror("lseek");return -1;}
		
			//continue;
		
		
		//printf("Current Needle: ID=[%d] State=[%d] Size=[%d] Jpeg Name[%s]\n",Id,State,Size,Name);
		printf("Current Needle: ID=[%d] State=[%d] Size=[%d]\n",Id,State,Size);
		}
		//Loop=Size/BUFFSIZE;
		//Remaining=Size%BUFFSIZE;
		
//		for(j=0;j<Loop;j++){
//			if (read (FileDes, Buffer , BUFFSIZE) < BUFFSIZE){perror("read from file");return -1;}
//			if ( write(FD,Buffer,BUFFSIZE) < BUFFSIZE){perror("write to file");return -1;}	
//		}
		
//		if (read (FileDes, Buffer , Remaining) < Remaining){perror("read from file");return -1;}
//		if ( write(FD,Buffer,Remaining) < Remaining){perror("write to file");return -1;}
//		close(FD);	
	//}
	printf("\n-------------------------END OF PRINT----------------\n");
	return 0;
	
}*/
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
int Compaction(int FileDes,int NewDes, CorePtr MyCore){
	int num=MagicNumber,Res,Offset;
	int Total,i,Loop, Remaining,Id,State,Size,MaxId=-1;
	int Buffer[BUFFSIZE],j;
	
	if(lseek(FileDes,sizeof(int),SEEK_SET)==-1){perror("lseek");return -1;}
	/*add magic number*/
	if(write(NewDes, &num, sizeof(int))!=sizeof(int)){perror("write magic number");return -1;}
	
	/*get total add to new*/
	if((Res=read(FileDes,&Total,sizeof(int)))!=sizeof(int)){perror("compatcion read total");return -1;}
	if(write(NewDes, &Total, sizeof(int))!=sizeof(int)){perror("compatcion write total number");return -1;}
	Offset=sizeof(int)*2;
	/*Copy Neeldes*/
	for( i=0; i<Total;i++){
	
		if(read(FileDes,&Id,sizeof(int))!=sizeof(int)){perror("read id");return -1;}	
		if(read(FileDes,&State,sizeof(int))!=sizeof(int)){perror("read state");return -1;}
		if(read(FileDes,&Size,sizeof(int))!=sizeof(int)){perror("read size");return -1;}
	
		if( State==0){ 
			i--; 
			if(lseek(FileDes,Size,SEEK_CUR)==-1){perror("lseek");return -1;}
			continue;
		}
		
		/*Create new index*/
		if(AddMatrix(MyCore->Matrix,Id,Offset)==-1){printf("addMatrix problem\n");return -1;}
		Offset+=sizeof(int)*3+Size;
		
		if(Id>MaxId){ MaxId=Id;}
		if(write(NewDes, &Id, sizeof(int))!=sizeof(int)){perror("write magic number");return -1;}
		if(write(NewDes, &State, sizeof(int))!=sizeof(int)){perror("write magic number");return -1;}
		if(write(NewDes, &Size, sizeof(int))!=sizeof(int)){perror("write magic number");return -1;}
		Loop=Size/BUFFSIZE;
		Remaining=Size%BUFFSIZE;
		for(j=0;j<Loop;j++){
			if ( read(FileDes, Buffer , BUFFSIZE) < BUFFSIZE){perror("read from file");return -1;}
			if ( write(NewDes,Buffer,BUFFSIZE) < BUFFSIZE){perror("write to file");return -1;}	
		}
		
		if (read (FileDes, Buffer , Remaining) < Remaining){perror("read from file");return -1;}
		if ( write(NewDes,Buffer,Remaining) < Remaining){perror("write to file");return -1;}
	
	}
	/*Calculate Id to be given to next Upload*/
	if(MaxId==-1){
		MyCore->Id=0;
	}else{
		MyCore->Id=MaxId+1;
		if(RestoreQueue(MyCore)==-1){printf("restore queue problem\n");return -1;}
	}
	MyCore->Fd=NewDes;
	return 0;
}
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
int RestoreQueue(CorePtr MyCore){/*Searchs index for unused cells*/
	int i;
	for (i=0;i<MyCore->Id;i++){
		if( MyCore->Matrix->Table[i]==0){
			if(QueueAdd(MyCore->Queue,i)==-1){printf("queue add problem\n");return -1;}
		}
	}
	return 0;	
}
////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////// 
