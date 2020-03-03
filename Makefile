all: client
client:
	g++ cf_client.cpp -o cf_udpclient
	g++ cf_server.cpp -o cf_udpserver