CC=gcc
CFLAGS=-g -Wall
OBJS=20161615.o
TARGET=20161615.out

all: $(TARGET)

clean:
	rm -f *.o
	rm -f $(TARGET)


$(TARGET): $(OBJS)
	$(CC) -o $@ $(OBJS)

20161615.o: 20161615.h 20161615.c


