CC=gcc
FLAGS=-Wall

all: clean server client
server: 
	$(CC) $(FLAGS) server.c -o server

client:
	$(CC) $(FLAGS) client.c -o client

clean:
	rm -rf server client
