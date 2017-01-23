all: build_server build_client clean

package:
	@zip -r serverclient.zip server.out client.out readme.txt
	@rm -f server.out
	@rm -f client.out

build_server:
	@gcc -o server.out server.c

build_client:
	@gcc -o client.out client.c

clean:
	@rm -f serverclient.zip
	@echo "Done"
