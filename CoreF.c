#include "Core.h"
#include <stdio.h>
#include <stdlib.h>
#include <semaphore.h>
#include <string.h>
#include <time.h>
#include "Hay.h"
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
int PrintOutput(char *string){ /*Calculates wall time*/
	time_t ltime;
		time(&ltime);
		ctime_r(&ltime,string);
		string[strlen(string)-1]='\0';
		return 0;

}
	
int CoreAdd(CorePtr Core,int Buff_fd,int Size){
	int Id,Offset;
	/*Get next available ID*/
	if( (Id=CoreGetId(Core))==-1){printf("Id problem\n");return -1;}
	//P(SemFile)
	if(sem_wait(& (Core)->Sem_File)){perror("sem wait"); return -1;}
    /*Add image to file*/
	if(AddNeedleToFile( (Core)->Fd,Buff_fd,Id,Size,&Offset)==-1){
		printf("AddNeedle Problem\n");
		if(sem_post(& (Core)->Sem_File)){perror("sem post"); return -1;}
		return -1;
	}

	if(sem_post(& (Core)->Sem_File)){perror("sem post"); return -1;}
	//V(SemFile)

	//P(SemMatrix)
	if(sem_wait(& (Core)->Sem_Matrix)){perror("sem wait"); return -1;}
    /*Add needle offset to matrix in "id" cell*/
	if(AddMatrix(Core->Matrix,Id,Offset)==-1){
		printf("AddMatrix Problem\n");
		if(sem_post(& (Core)->Sem_Matrix)){perror("sem post"); return -1;}
		return -1;
	}
	
	if(sem_post(& (Core)->Sem_Matrix)){perror("sem post"); return -1;}
	//V(SemMatrix)

	return Id;
}
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////

int CoreRemove(CorePtr Core,int Id){
	int Offset;
	
    //P(SemMatrix)
	if(sem_wait(& (Core)->Sem_Matrix)){perror("sem wait"); return -1;}
    /*See if id exists in index*/
	if( (Offset=GetElementMatrix(Core->Matrix,Id))==-1){
		printf("Get Element Matrix Problem\n");
		if(sem_post(& (Core)->Sem_Matrix)){perror("sem post"); return -1;}
		return -1;
	}

	if(Offset>0)/*If exists, remove from index*/
		if(RemoveMatrix(Core->Matrix,Id)==-1){
			printf("RemoveMatrix Problem\n");
			if(sem_post(& (Core)->Sem_Matrix)){perror("sem post"); return -1;}
			return -1;
	}

	if(sem_post(& (Core)->Sem_Matrix)){perror("sem post"); return -1;}
	//V(SemMatrix)

	if(Offset>0){/*If exists, change status of corresdonding needle to deleted*/
		//P(SemFile)
		if(sem_wait(& ((Core)->Sem_File))){perror("sem wait"); return -1;}
		
		if(DeleteNeedle((Core)->Fd, Offset)==-1){
			printf("DeleteNeedle Problem\n");
			if(sem_post(& ((Core)->Sem_File))){perror("sem post"); return -1;}
			return -1;
		}

		if(sem_post(& ((Core)->Sem_File))){perror("sem post"); return -1;}
		//V(File)
		
		//P(SemQueue)
		if(sem_wait(& (Core)->Sem_Queue)){perror("sem wait"); return -1;}
        /*Id removed from Id is added to queue*/
		if(QueueAdd(Core->Queue,Id)==-1){
			printf("QueueAdd Problem\n");
			if(sem_post(& (Core)->Sem_Queue)){perror("sem post"); return -1;}
			return -1;
		}
		
		if(sem_post(& (Core)->Sem_Queue)){perror("sem post"); return -1;}
		//V(SemQueue)
		return 0;
	}
	else{
		return 1;
	}
}

int CoreExists(CorePtr Core,int Id){
	int Offset;
	//P(SemMatrix)
	if(sem_wait(& (Core)->Sem_Matrix)){perror("sem wait"); return -1;}
    /*See if id exists in index*/
	if( (Offset=GetElementMatrix(Core->Matrix,Id))==-1){
		if(sem_post(& (Core)->Sem_Matrix)){perror("sem post"); return -1;}
		printf("Get Element Matrix Problem\n");
		return -1;
	}
	
	if(sem_post(& (Core)->Sem_Matrix)){perror("sem post"); return -1;}
	//V(SemMatrix)

	if(Offset>0){
		return Offset;
		
	}
	else{/*Doesn't exist*/
		return 0;
	}
}
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
int CoreInitiate(CorePtr *Core,int Size,char* WorkFile){

	int Fd;
	if(WorkFile==NULL) 
		return -1;

	if(((*Core)=malloc(sizeof(struct Core)))==NULL){perror("Core malloc");return -1;}
	if(InitiateMatrix(&((*Core)->Matrix),Size)!=0){printf("Init Matrix Problem\n");return -1;}
	if(QueueInitiate(&((*Core)->Queue))!=0){printf("Init Queue Problem\n");return -1;}
	
	(*Core)->Id=0;
	if( (Fd=InitializeHayStack( WorkFile,*Core))==-1){printf("Init HayStack Problem\n");return -1;}
	(*Core)->Fd=Fd;
	(*Core)->Total_Threads=0;
	(*Core)->TerminationFlag=0;
	if( sem_init (& ((*Core)->Sem_Total) ,0 ,1)!=0){ perror("Sem Init"); return -1; }
	if( sem_init (& ((*Core)->Sem_File) ,0 ,1)!=0){ perror("Sem Init"); return -1; }
	if( sem_init (& ((*Core)->Sem_Matrix) ,0 ,1)!=0){ perror("Sem Init"); return -1; }
	if( sem_init (& ((*Core)->Sem_Queue) ,0 ,1)!=0){ perror("Sem Init"); return -1; }
	if( sem_init (& ((*Core)->Sem_Id) ,0 ,1)!=0){ perror("Sem Init"); return -1; }

	return 0;
}	
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
int CoreDelete(CorePtr *Core){

	if((*Core)!=NULL){
		if(DeleteMatrix(& ((*Core)->Matrix))==-1){printf("Delete Matrix Problem\n");return -1;}
		if(QueueDelete(& ((*Core)->Queue))==-1){printf("Delete Queue Problem\n");return -1;}
		if(close((*Core)->Fd)==-1){perror("close");return -1;}
		if(sem_destroy (& ((*Core)->Sem_File) )==-1){ perror("Sem Dest"); return -1; }
		if(sem_destroy (& ((*Core)->Sem_Matrix))==-1){ perror("Sem Dest"); return -1; }
		if(sem_destroy (& ((*Core)->Sem_Queue))==-1){ perror("Sem Dest"); return -1; }
		if(sem_destroy (& ((*Core)->Sem_Id) )==-1){ perror("Sem Dest"); return -1; }
		free(*Core);
	}
	*Core=NULL;
	return 0;

	
}
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
int CorePrint(CorePtr Core){  /*Debuggin*/
	
	printf("Printing Core:\n");
	if(Core!=NULL){
		//P(Queue)
		if(sem_wait(& ((Core)->Sem_Queue))){perror("sem wait"); return -1;}
        
		if(QueuePrint(Core->Queue)==-1){printf("Get Element Matrix Problem\n");return -1;}

		if(sem_post(& (Core)->Sem_Queue)){perror("sem post"); return -1;}
		//V(Queue)

		//P(Matrix)
		if(sem_wait(& (Core)->Sem_Matrix)){perror("sem wait"); return -1;}

		if(PrintMatrix(Core->Matrix)==-1){printf("Get Element Matrix Problem\n");return -1;}

		if(sem_post(& (Core)->Sem_Matrix)){perror("sem post"); return -1;}
		//V(Matrix)
	}
	return 0;
}
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
int CoreGetId(CorePtr Core){
	int Id,Err;
	if(Core==NULL){printf("Core NUll\n");return -1;}
		
	//p(SemQueue)
	if(sem_wait(& ((Core)->Sem_Queue))){perror("sem wait"); return -1;}
    /*Get a id from queue if available*/
	if( (Err=QueueRemoveHead(Core->Queue,&Id))==-1){
		printf("QueueRemoveH Problem\n");
		if(sem_post(& (Core)->Sem_Queue)){perror("sem post"); return -1;}
		return -1;
	}

	if(sem_post(& (Core)->Sem_Queue)){perror("sem post"); return -1;}
	//v(SemQueue

	if(Err==1){/*If list is empty, increase the id flag and use it*/
		//p(SemId)
		if(sem_wait(& ((Core)->Sem_Id))){perror("sem wait"); return -1;}
		Id=Core->Id;
		Core->Id++;
		if(sem_post(& (Core)->Sem_Id)){perror("sem post"); return -1;}
		//v(SemId)
	}
		return Id;
}
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////

