
TARGET=tweet
CC=gcc
CFLAGS= -ansi -std=c99 -c -W -Wall -Werror
LDFLAGS= -lcurl -loauth -lcrypto
OBJS = \
	tweet.o \
	utils.o \
	main.o


all: $(TARGET)

run: $(TARGET)
	./$(TARGET)

$(TARGET): $(OBJS)
	$(CC) -o $(TARGET) $(OBJS) $(LDFLAGS)


.c.o :
	gcc $(CFLAGS) -O2 $<

clean:
	-rm $(TARGET)
	-rm *.o

