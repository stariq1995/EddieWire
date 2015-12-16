all: code/recv-server.c code/transfer-client.c
	rm -rf bin tmp
	mkdir bin tmp
	gcc -o bin/recv code/recv-server.c -lbluetooth
	gcc -o bin/transfer code/transfer-client.c -lbluetooth
	gcc -o bin/forwd code/forwarder.c -lbluetooth
	gcc -o bin/bt_forwd code/bt_forwarder.c -lbluetooth

server: code/recv-server.c
	rm -rf tmp
	mkdir tmp
	gcc -o bin/recv code/recv-server.c -lbluetooth

client: code/transfer-client.c
	gcc -o bin/transfer code/transfer-client.c -lbluetooth
	

opp: code/opportunistic_forwarder.c
	rm -rf tmp
	mkdir tmp
	gcc -o bin/oppf code/opportunistic_forwarder.c
