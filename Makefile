CC = gcc 
LD = gcc

SRCS = $(wildcard *.c)
OBJS = $(patsubst %c, %o, $(SRCS))

TARGET = RTPParser

.PHONY:all clean

all: $(TARGET)

$(TARGET): $(OBJS)
	$(LD) -o $@ $^ -pthread

%o:%c
	$(CC) -c $^ -std=c11

clean:
	rm -f $(OBJS) $(TARGET)
