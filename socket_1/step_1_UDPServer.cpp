//server side - UDP protocol & IPv4

using namespace std;

#include <iostream>
#include <Winsock2.h>

#pragma comment(lib,"ws2_32.lib")

int main()
{

#pragma region Init_winsock_lib
	WORD mVersionRequested = MAKEWORD(2, 2); 
	WSADATA wsadata;
	int startupError = WSAStartup(mVersionRequested, &wsadata);
	if (startupError != 0)
	{
		cout << WSAGetLastError() << endl;
		cout << "Startup failed" << endl;
		auto w = getchar();
		return -1;
	}
#pragma endregion

	//udp server socket
	int addressFamily = AF_INET;
	int socketType = SOCK_DGRAM;
	int protocol = IPPROTO_UDP; //use 0 to auto-set protocol with socket type

	SOCKET sockUDP = socket(addressFamily, socketType, protocol);
	if (sockUDP == INVALID_SOCKET)
	{
		cout << WSAGetLastError() << endl;
		cout << "Invalid socket" << endl;
		auto w = getchar();
		return -1;
	}

	//server socket address 
	sockaddr_in socketAddress;

	int port = 8000;
	socketAddress.sin_family = addressFamily;
	socketAddress.sin_port = htons(port);
	memset(socketAddress.sin_zero, 0, sizeof(socketAddress.sin_zero));
	
	//INADDR_ANY: receive from all address. This is useful especially when the device has multiple NICs
	socketAddress.sin_addr.s_addr = INADDR_ANY;
	//several other methods to set sin_addr is commented in step_1 _client.cpp

	//bind. 
	int bindError = bind(sockUDP, (sockaddr*)&socketAddress, sizeof(socketAddress));
	if (bindError == -1)
	{
		cout << WSAGetLastError() << endl;
		cout << "bind error" << endl;
		auto w = getchar();
		return -1;
	}

	//receive data
	sockaddr_in clientSockAddr; //does not have to be initialized
	int fromlen = sizeof(clientSockAddr);//equivalents: sizeof(sockaddr_in), sizeof(sockaddr)
	
	int recvBuflen = 512;
	char* recvMessage = new char[recvBuflen];
	//char recvMessage[512] = {}; //either way works

	int recvLen = recvfrom(sockUDP, recvMessage, recvBuflen, 0, (sockaddr*)&clientSockAddr, &fromlen);
	//if recvfrom works correctly clientSockAddr is initialized with client data
	if (recvLen == -1)
	{
		cout << WSAGetLastError() << endl;
		cout << "recv error" << endl;
		return -1;
	}
	else
	{
		cout << "message recevied: " << endl;
		cout << recvMessage << endl;
	}

	//send data to client that sent message 
	const int sendBuflen = 512;
	char* sendMessage = new char[sendBuflen];

	cin.getline(sendMessage, sendBuflen);

	int sendError = sendto(sockUDP, sendMessage, sendBuflen, 0, (sockaddr*)&clientSockAddr, sizeof(clientSockAddr));
	if (sendError == SOCKET_ERROR)
	{
		cout << WSAGetLastError() << endl;
		cout << "send error" << endl;
		char w = getchar();
		return -1;
	}

	cout << "sent:" << sendMessage << endl;

	//shutdown & close socket 
	int closesocketError = closesocket(sockUDP);

	//end using winsock2 library
	WSACleanup();
	char w = getchar();
}