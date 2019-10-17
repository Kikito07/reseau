all : sender
sender: sender
	gcc -g sender.c -o sender packet_implem.c utils.c -Wall -Werror -lz