all: clean cracker tests

cracker: cracker
	gcc -g src/sender.c -o sender -lpthread -Wall -Werror

clean:
	rm -f cracker

