OBJS	= readScript.o
SOURCE	= readScript.cpp
OUT	= ejecutar
FLAGS	= -g -c -Wall
CC	= g++

all:	ejecutar

ejecutar: $(OBJS)
	$(CC) -g $(OBJS) -o $(OUT)

readScript.o: readScript.cpp
	$(CC) $(FLAGS) readScript.cpp 

clean:
	rm -f $(OBJS) $(OUT)