CC = ~/CHIP-SDK/CHIP-buildroot/output/host/usr/bin/arm-linux-gnueabihf-gcc
LD_FLAG := -lpthread

clean:
	rm bridgeServer server switchBoard || true

server:
	$(CC) bridgeServer.c -o bridgeServer -w $(LD_FLAG)
	$(CC) server.c -o server -w $(LD_FLAG)

sb:
	$(CC) switchBoard.c -o switchBoard -w $(LD_FLAG)
