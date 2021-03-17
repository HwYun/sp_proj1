CC=gcc
CFLAGS=-g -Wall
OBJS=20161615.o my_dir.o my_memory.o
TARGET=20161615.out

all: $(TARGET)

clean:
	rm -f *.o
	rm -f $(TARGET)


$(TARGET): $(OBJS)
	$(CC) -o $@ $(OBJS)

20161615.o: 20161615.h 20161615.c
my_dir.o: 20161615.h my_dir.c
my_memory.o: 20161615.h my_memory.c
