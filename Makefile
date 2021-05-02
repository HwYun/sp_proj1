CC=gcc
CFLAGS=-g -Wall
OBJS=20161615.o my_dir.o my_memory.o opcode.o type.o assemble.o loader.o
TARGET=20161615.out

all: $(TARGET)

clean:
	rm -f *.o
	rm -f *.lst
	rm -f 2_5.obj
	rm -f $(TARGET)


$(TARGET): $(OBJS)
	$(CC) -o $@ $(OBJS)

20161615.o: 20161615.h 20161615.c
my_dir.o: 20161615.h my_dir.c
my_memory.o: 20161615.h my_memory.c
opcode.o: 20161615.h opcode.c
type.o: 20161615.h type.c
assemble.o: 20161615.h assemble.c
loader.o: 20161615.h loader.c
