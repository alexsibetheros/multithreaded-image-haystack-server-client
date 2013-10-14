#ifndef hay
#define hay
#include "Core.h"


int PrintHay(int FileDes);
int DeleteNeedle(int FileDes,int Offset);
int AddNeedleToFile(int FileDes,int BufferDes,int Id,int Size,int* Offset);
int InitializeHayStack(char *WorkFile,CorePtr Core);
int Exists(char *File);
int Compaction(int FileDes,int NewDes, CorePtr MyCore);


#endif /*hay*/
