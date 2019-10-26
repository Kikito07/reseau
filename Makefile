sender: sender.c packet_implem.c sender_list.c utils.c
	gcc -o sender packet_implem.c sender_list.c utils.c sender.c -Wall -Werror -lz 

clean: 
	rm -rf *o sender

tests: sender.c packet_implem.c sender_list.c utils.c
	gcc -o tests tests.c packet_implem.c sender_list.c utils.c -Wall -Werror -lz -lcunit
	./tests