CC=gcc
LD=gcc
CFLAGS=-Wall -m32 -c
LDFLAGS=-Wall -m32
TARGET=my_format
OBJS=my_format.o

.PHONY: all clean

all: $(TARGET)

$(TARGET):	$(OBJS)
	$(LD) $(LDFLAGS) $^ -o $@

%.o:	%.c
	$(CC) $(CFLAGS) $^ -o $@

clean:
	rm -f $(TARGET) $(OBJS) *.o
