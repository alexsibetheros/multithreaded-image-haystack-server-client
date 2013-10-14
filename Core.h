#ifndef core
#define core
#include <unistd.h>
#include <semaphore.h>
#include "Matrix.h"
#include "Queue.h"

////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
struct Core {
	MatrixPtr Matrix;
	QueuePtr Queue;
	int Id;
	int Fd;
	int Total_Threads;
	sem_t Sem_Total;
	sem_t Sem_File;
	sem_t Sem_Matrix;
	sem_t Sem_Queue;
	sem_t Sem_Id;
	int TerminationFlag;
};
typedef struct Core * CorePtr;
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////

int CoreAdd(CorePtr Core,int Buff_fd,int Size);
int CoreRemove(CorePtr Core,int Id);
int CoreInitiate(CorePtr *Core,int Size,char* WorkFile);
int CoreDelete(CorePtr *Core);
int CorePrint(CorePtr Core);
int CoreGetId(CorePtr Core);
int PrintOutput(char *string);
int CoreExists(CorePtr Core,int Id);
int RestoreQueue(CorePtr MyCore);
#endif /*core*/

