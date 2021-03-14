build:
	gcc -Wall -g server.c -o server
	gcc -Wall -g subscriber.c -o subscriber

run_server:
	./server 8080


run_subscriber:
	./subscriber $(ARGS) 127.0.0.1 8080


clean:
	rm -f server client subscriber
