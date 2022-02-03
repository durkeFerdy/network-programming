//TCP host A. It's named server only to add intuitiveness.

using namespace std;

#include <iostream>
#include <Winsock2.h>

#pragma comment(lib, "ws2_32.lib")

int main()
{
	WSADATA wsadata; 
	int startupError = WSAStartup(MAKEWORD(2, 2), &wsadata);
	if(startupError != 0)
	{
		cout << WSAGetLastError() << endl;
		cout << "Startup error" << endl;
		getchar();
		return -1;
	}


	//create socket(this will be listening socket)
	int addressFamily = AF_INET;
	int socketType = SOCK_STREAM;
	int protocol = IPPROTO_TCP;

	SOCKET listenSock = socket(addressFamily, socketType, protocol);
	if(listenSock ==INVALID_SOCKET)
	{
		cout << WSAGetLastError() << endl;
		cout << "invalid socket error" << endl;
		getchar();
		return -1;
	}

	//bind socket
	
	 //this address will be shared by both listening socket and the socket returned by accept.
	sockaddr_in localAddress;
	int port = 8000;
	const char* ip = "127.0.0.1";//loop back. INADDR_ANY is recommendable for actual server.

	localAddress.sin_family = addressFamily;
	localAddress.sin_port = htons(port);
	memset(&localAddress.sin_zero, 0, sizeof(localAddress.sin_zero));
	localAddress.sin_addr.s_addr = inet_addr(ip);
	
	int bindError = bind(listenSock, (sockaddr*)&localAddress, sizeof(sockaddr));
	if(bindError==-1)
	{
		cout << WSAGetLastError() << endl;
		cout << "bind error" << endl;
		getchar();
		return -1;
	}

	int backlog = 2;
	int listenError = listen(listenSock, backlog);
	if(listenSock == -1)
	{
		cout << WSAGetLastError() << endl;
		cout << "listen error" << endl;
		getchar();
		return -1;
	}

	//accept
	//this implementation of server will accept first connection only. second connection will not be handled.
	sockaddr_in remoteAddress;
	int remoteLen= sizeof(sockaddr_in);

	SOCKET connectedSock = accept(listenSock, (sockaddr*)&remoteAddress, &remoteLen);

	cout << "connection established" << endl;

	//send message to client
	const int sendBuflen = 100;
	char sendMsg[sendBuflen];
	cout << "send: ";
	cin.getline(sendMsg, sendBuflen);

	int sendError = send(connectedSock, sendMsg, sendBuflen, 0);
	if(sendError ==-1 )
	{
		cout << WSAGetLastError() << endl;
		cout << "send error" << endl;
		getchar();
		return -1;
	}
	cout << "sent message: " << sendMsg << endl;

	//receive message from server
	const int recvBuflen = 100;
	char recvMsg[recvBuflen];

	int recvError = recv(connectedSock, recvMsg,recvBuflen, 0);
	if (recvError == -1)
	{
		cout << WSAGetLastError() << endl;
		cout << "recv error" << endl;
		getchar();
		return -1;
	}
	cout << "received message: " << recvMsg << endl;


	//closing sockets
	//both listening socket and all connected sockets(in this case there's only one) should be closed when both will not be used anymore

	//mehtod 1. insecure closing
	/*
	closesocket(listenSock);
	closesocket(connectedSock);
	*/

	//method 2. secure closing 
	//let this TCPServer initiate disconnection.(Client also can be initiator)
	//shutdown of listening socket is unnecessary, if not end of the program, and some case should be avoided
	//For instance, to connect other clients, listening socket should remain without shutdown or close
	
	//shutdown send stream not recv stream
	shutdown(connectedSock,SD_SEND);

	//reset recvMsg
	memset(recvMsg, '\0', recvBuflen);
	//recv remaining data.
	//remaining data can be data that arrived after shutdown.
	//in this case(inpractical case for learning purpose only) the data from host is just data that was sent after recv fails.
	int finalRecvError =recv(connectedSock, recvMsg, recvBuflen, 0);
	if(finalRecvError==-1)
	{
		cout << WSAGetLastError() << endl;
		cout << "recv error" << endl;
		getchar();
		return -1;
	}
	cout << "received message: " << recvMsg << endl;

	//close socket
	closesocket(listenSock);
	closesocket(connectedSock);


	WSACleanup();
	getchar();
}