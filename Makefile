all:
	rm -rf test_client test_server
	gcc -std=gnu99 -g -O0  utils.h hashfn.h hashfn.c hash_list.h hash_list.c  client.c -o test_client -lpthread
	gcc -std=gnu99 -g -O0  utils.h hashfn.h hashfn.c hash_list.h hash_list.c  server.c -o test_server -lpthread