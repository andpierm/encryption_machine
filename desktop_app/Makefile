CC = gcc
CFLAGS = -Wall -g
BINS = app.o


OBJS = app.o

all: app

app: $(OBJS)
	$(CC) $(OBJS) -o app $(LIBS)

app.o: app.c
	$(CC) $(CFLAGS) $(INCLUDES) -c app.c -o app.o

clean:
	rm -f *.o app
