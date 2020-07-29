all:
	rm -rf test_client test_server
		gcc -std=gnu99 -g -O0  util.h util.c hash_list.h hash_list.c  message.h message.c tcp_client.h tcp_client.c -o test_client -lpthread 
	gcc -std=gnu99 -g -O0  util.h util.c  hash_list.h hash_list.c  message.h message.c tcp_server.c -o test_server  -lpthread
clean:
	rm -rf test_*