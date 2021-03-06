#include <stdio.h>
#include <stdlib.h>
#include "Matrix.h"
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
int InitiateMatrix(MatrixPtr *Matrix,int Size){
	int i;
	if(((*Matrix)=malloc(sizeof(struct Matrix)))==NULL){perror("InitMatrix");return -1;}
	if(((*Matrix)->Table=malloc(Size*sizeof(int)))==NULL){perror("InitMatrix");return -1;}
	
	(*Matrix)->Size=Size;
	for(i=0;i<Size;i++)
		(*Matrix)->Table[i]=0; 
	return 0;

}
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
int DeleteMatrix(MatrixPtr *Matrix){

	if(*Matrix!=NULL){
		free((*Matrix)->Table);
		free(*Matrix);
	}
	*Matrix=NULL;
	return 0;
}
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
int AddMatrix(MatrixPtr Matrix,int Position,int Offset){
		
	int OldSize,i;
	if(Matrix==NULL){printf("Matrix is null\n");return -1;}
	OldSize=Matrix->Size; /*Old size is needed to fill black spaces after reallocation*/

	if( Matrix->Size <= Position ){/*Not enough space, reallocate and add zero's*/

	
		Matrix->Size=(Position)*2; 

		if((Matrix->Table=realloc(Matrix->Table,(Matrix->Size)*sizeof(int)))==NULL){perror("Realloc failed");return -1;}
		for(i=OldSize;i<Matrix->Size;i++)
			Matrix->Table[i]=0;
	}

	Matrix->Table[Position]=Offset;
	return 0;

}
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
int PrintMatrix(MatrixPtr Matrix){ /*For Debugging*/

	int i;	
	if(Matrix==NULL){printf("Matrix is null\n");return -1;}
		
	if(Matrix->Table==NULL){printf("Matrix Table is null\n");return -1;}

	printf("Printing Matrix:\n");

	for(i=0;i<Matrix->Size;i++)
		printf("[%d]={%d}\n",i,Matrix->Table[i]);
	return 0;	
}
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
int GetElementMatrix(MatrixPtr Matrix,int Position){/*Get offset of given id*/
	if(Matrix==NULL){printf("Matrix is null\n");return -1;}
		
	if(Position > Matrix->Size)
		return 0;
	return(Matrix->Table[Position]);
}
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
int RemoveMatrix(MatrixPtr Matrix,int Position){
	if(Matrix==NULL){printf("Matrix is null\n");return -1;}
	Matrix->Table[Position]=0;
	return 0;
}
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
