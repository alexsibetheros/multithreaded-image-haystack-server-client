OBJS 	= CoreF.o QueueF.o MatrixF.o HayF.o ServerF.o
SOURCE	= CoreF.c QueueF.c MatrixF.c HayF.c ServerF.c
HEADER  = 
OUT  	= run
CC	= gcc
FLAGS   = -c  #-g  -ggdb -pg -DDEBUG -Wall -Wextra
LIBS	= -lpthread

all: $(OBJS)
	$(CC) -g $(OBJS) -o $(OUT)  $(LIBS)
	
# create/compile the individual files >>separately<< 
CoreF.o: CoreF.c
	$(CC) $(FLAGS) CoreF.c
	
QueueF.o : QueueF.c
	$(CC) $(FLAGS) QueueF.c
	
MatrixF.o: MatrixF.c
	$(CC) $(FLAGS) MatrixF.c

HayF.o: HayF.c
	$(CC) $(FLAGS) HayF.c

ServerF.o: ServerF.c
	$(CC) $(FLAGS) ServerF.c

	
# clean house
clean:
	rm -f $(OBJS) $(OUT)


