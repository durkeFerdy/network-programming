//TCP host A. It's named client only to add intuitiveness.

using namespace std;

#include <iostream>
#include <Winsock2.h>

#pragma comment(lib, "ws2_32.lib")

int main()
{
	WSADATA wsadata;
	int startupError = WSAStartup(MAKEWORD(2, 2), &wsadata);
	if (startupError != 0)
	{
		cout << WSAGetLastError() << endl;
		cout << "Startup error" << endl;
		getchar();
		return -1;
	}

	//create client socket 
	int addressFamily = AF_INET;
	int socketType = SOCK_STREAM;
	int protocol = IPPROTO_TCP;

	SOCKET clientSock = socket(addressFamily, socketType, protocol);
	if(clientSock == INVALID_SOCKET)
	{
		cout << WSAGetLastError() << endl;
		cout << "invalid socket error" << endl;
		getchar();
		return -1;
	}

	//bind - skipped
	//bind is not recommended for client in classic server-client relation for several reason. one can do it but not so recommendable
	//if bind is not called before, os will auto-assign port to the socket if 'connect' is aclled in TCP or 'sendto' is called in UDP
	//Therefore in client code without explicit bind, sockaddr for client also is not needed and only sockaddress for destination(server) is needed.

	//connect
	
	//server sockaddr
	sockaddr_in remoteAddress;
	int remotePort = 8000; 
	const char* remoteIP = "127.0.0.1";
	remoteAddress.sin_family = addressFamily;
	remoteAddress.sin_port = htons(remotePort);
	remoteAddress.sin_addr.s_addr = inet_addr(remoteIP);

	int connectError = connect(clientSock, (sockaddr*)&remoteAddress, sizeof(sockaddr));
	if(connectError==-1)
	{
		cout << WSAGetLastError() << endl;
		cout << "connect error" << endl;
		getchar();
		return -1;
	}

	//recv message from server
	const int recvBuflen = 100;
	char recvMsg[recvBuflen];

	int recvError = recv(clientSock, recvMsg, recvBuflen, 0);
	if (recvError == -1)
	{
		cout << WSAGetLastError() << endl;
		cout << "recv error" << endl;
		getchar();
		return -1;
	}
	cout << "received message: " << recvMsg << endl;

	//send message to server
	const int sendBuflen = 100;
	char sendMsg[sendBuflen];
	
	cout << "send: ";
	cin.getline(sendMsg, sendBuflen);

	int sendError = send(clientSock, sendMsg, sendBuflen, 0);
	if (sendError == -1)
	{
		cout << WSAGetLastError() << endl;
		cout << "send error" << endl;
		getchar();
		return -1;
	}
	cout << "sent message: " << sendMsg << endl;


	closesocket(clientSock);

	WSACleanup();
	getchar();
}