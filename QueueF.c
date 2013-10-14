#include "Queue.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
int QueueAdd(QueuePtr Queue,int Element){
	NodePtr Temp;
		if(Queue==NULL){printf("error ,null queue\n");return -1;}
		if((Temp=(NodePtr)malloc(sizeof(struct Node)))==NULL ){perror("Malloc");return -1;}
	
		Temp->Element=Element;
		Temp->Next=NULL;
		if(Queue->Start==NULL){
			Queue->Start=Temp;
			Queue->End=Temp;
			
		}
		else{
			Queue->End->Next=Temp;
			Queue->End=Temp;
		}
		return 0;
	}
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
int QueueRemoveHead(QueuePtr Queue,int *Element){
	NodePtr Temp;
	if(Queue==NULL){printf("error ,null queue\n");return -1;}
	if(Queue->Start==NULL ){
		(*Element)=-1;
		return 1;
	}
	*Element=Queue->Start->Element;
		
	Temp=Queue->Start;
	Queue->Start=Queue->Start->Next;
	free(Temp);
	if(Queue->Start==NULL){
		Queue->End=NULL;
	}
	return 0;
	
}
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
int QueuePrint(QueuePtr Queue){
	NodePtr Start;

	printf("printing Queue:\n");
	if(Queue==NULL){
		printf("error ,null queue\n");
		return -1;
	}
	Start=Queue->Start;
	while(Start!=NULL){
		printf("{%d}--->",Start->Element);
		Start=Start->Next;
	}
	printf("X\n");
	return 0;
}
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
int QueueDelete(QueuePtr *Queue){
	NodePtr Temp;
	if(Queue==NULL){
		
		return -1;
	}
	while((*Queue)->Start){
		Temp=(*Queue)->Start;
		(*Queue)->Start=(*Queue)->Start->Next;
		free(Temp);
		
	}
	free(*Queue);
	(*Queue)=NULL;
	return 0;
}
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
int QueueInitiate(QueuePtr *Queue){
	if(((*Queue)=(QueuePtr)malloc(sizeof(struct Queue)))==NULL){perror("malloc"); return -1;}
	(*Queue)->Start=NULL;
	(*Queue)->End=NULL;
	return 0;
}
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
int QueueEmpty(QueuePtr Queue){
	if(Queue==NULL){printf("queue is null\n");return -1;}
		
	if(Queue->Start==NULL)
		return 1;
	else
	    	return 0;
	    
}
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
