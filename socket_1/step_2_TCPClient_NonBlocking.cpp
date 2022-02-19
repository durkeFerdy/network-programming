//non-blocking I/O with non blocking socket 
using namespace std;

#include <iostream>
#include <WinSock2.h>
#include <thread>

#pragma comment(lib, "ws2_32.lib")

#define recvBuflen 128
#define sendBuflen 128

int main()
{
#pragma region Prepare_Connection
	WSADATA wsadata;
	int startupError = WSAStartup(MAKEWORD(2, 2), &wsadata);
	if (startupError != 0)
	{
		cout << WSAGetLastError() << endl;
		cout << "Startup error" << endl;
		getchar();
		return -1;
	}

	int addressFamily = AF_INET;
	int socketType = SOCK_STREAM;
	int protocol = IPPROTO_TCP;

	SOCKET clientSock = socket(addressFamily, socketType, protocol);
	if (clientSock == INVALID_SOCKET)
	{
		cout << WSAGetLastError() << endl;
		cout << "invalid socket error" << endl;

		WSACleanup();
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

		WSACleanup();
		getchar();
		return -1;
	}

#pragma endregion

	//set socket to non blocking mode
	//non zero value for non blocking io
	u_long argp = 1;
	ioctlsocket(clientSock, FIONBIO, &argp);

	bool terminate = false;

	//waiting for input should not block process. use thread 
	auto inputRoutine = [](bool& terminate, SOCKET* sock)
	{
	
		while(!(terminate)) 
		{
			char sendMsg[sendBuflen];
			cout << "send: ";
			cin.getline(sendMsg, sendBuflen);
			if (strcmp(sendMsg, "terminate") == 0)
			{
				terminate = true;
			}
			int sendError = send(*sock, sendMsg, sendBuflen, 0);

			if (sendError == -1)
			{
				if (WSAGetLastError() == WSAEWOULDBLOCK)
				{
					//pass
				}
				else
				{
					cout << WSAGetLastError() << endl;
					cout << "send error" << endl;
					getchar();
					break;
				}
			}
		}
	};
	thread inputThread(inputRoutine, ref(terminate), &clientSock);

	while (!terminate)
	{
		char recvMsg[recvBuflen];
		//on receiving 'terminate', send 'disconnect' 
		int recvError = recv(clientSock, recvMsg, recvBuflen, 0);
		if(recvError==-1)
		{
			if (WSAGetLastError() == WSAEWOULDBLOCK)
			{
				//pass
			}
			else
			{
				cout << WSAGetLastError() << endl;
				cout << "recv error" << endl;
				getchar();
				break;
			}
		}
		//if message was actually received. 
		//if it is not checked, recvMsg will always be "" which is initial value of recvMsg;
		if(recvError>0)
		{
			//erase "send: "
			cout << "\b\b\b\b\b\b";
			cout << "received message: " << recvMsg << endl;
			cout << "send: ";
			if (strcmp(recvMsg, "terminate") == 0)
			{
				terminate = true;
				break;
			}
		}
		else if (recvError == 0) 
		{
			//there is no message received
			//pass
		}

	}


	inputThread.join();

	cout << "disconnect" << endl;
	closesocket(clientSock);
	WSACleanup();
	getchar();
}