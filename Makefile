all:
	rm -rf p2p_client p2p_server
	gcc -std=gnu99 -g -O0  util.h util.c hash_list.h hash_list.c  message.h message.c tcp_client.h tcp_client.c -o p2p_client -lpthread 
	gcc -std=gnu99 -g -O0  util.h util.c  hash_list.h hash_list.c  message.h message.c tcp_server.c -o p2p_server  -lpthread
clean:
	rm -rf p2p_client p2p_server