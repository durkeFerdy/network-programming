//TCP mutithreading client side

using namespace std;

#include <iostream>
#include <WinSock2.h>

#pragma comment(lib, "ws2_32.lib")

int main()
{
#pragma region Prepare_Connection
	WSADATA wsadata;
	int startupError = WSAStartup(MAKEWORD(2, 2), &wsadata);
	if(startupError!=0)
	{
		cout << WSAGetLastError() << endl;
		cout << "Startup error" << endl;
		getchar();
		return -1;
	}

	//listening socket 
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

	sockaddr_in remoteAddress; 
	int remotePort = 8000;
	const char* remoteIP = "127.0.0.1";
	remoteAddress.sin_family = addressFamily;
	remoteAddress.sin_port = htons(remotePort);
	remoteAddress.sin_addr.s_addr = inet_addr(remoteIP);

	int connectError = connect(clientSock, (sockaddr*)&remoteAddress, sizeof(remoteAddress));
	if (connectError == -1)
	{
		cout << WSAGetLastError() << endl;
		cout << "connect error" << endl;
		getchar();
		return -1;
	}

#pragma endregion
	const int sendBufLen = 100;
	char sendMsg[sendBufLen];

	while(sendMsg!="end")
	{
		cout << "send: ";
		cin.getline(sendMsg, sendBufLen);
		int sendError = send(clientSock, sendMsg, sendBufLen, 0);
		if (sendError == -1)
		{
			cout << WSAGetLastError() << endl;
			cout << "send error" << endl;
			break;
		}
		cout << "sent message: " << sendMsg << endl;
	}

	cout << "connection ended" << endl;

	WSACleanup();
	getchar();
}

