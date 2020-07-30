all:
	rm -rf tcp_server tcp_client
	gcc -std=gnu99 -g -O0  util.h util.c hash_list.h hash_list.c  message.h message.c p2p_client.h p2p_client.c -o tcp_client -lpthread 
	gcc -std=gnu99 -g -O0  util.h util.c  hash_list.h hash_list.c  message.h message.c p2p_server.c -o tcp_server  -lpthread
clean:
	rm -rf tcp_server tcp_client