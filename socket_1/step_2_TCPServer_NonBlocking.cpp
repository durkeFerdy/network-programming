//non - blocking TCP server using 'select' function
//when select is returned,
//readfds received packet. Therefore recv called on this socket will not block
//writefds are ready to send data. Therefore send called on this socket will not block

using namespace std;

#include <iostream>
#include <WinSock2.h>
#include <vector>

#pragma comment(lib, "ws2_32.lib")


#define recvBuflen 128
#define sendBuflen 128

//add socket to fd_set.
void FillFdsFromVector(vector<SOCKET>& sockptrVector, fd_set& fd)
{
	//initialize fd_sets using FD_ZERO macro
	FD_ZERO(&fd);

	for (auto sock : sockptrVector)
	{
		//check if socket is already added 
		if (FD_ISSET(sock, &fd))
		{
			continue;
		}
		//add socket
		FD_SET(sock, &fd);
	}
}

void FillVectorFromFds(fd_set& fd, vector<SOCKET>& inSockptrVector, vector<SOCKET>& outSockptrVector)
{
	outSockptrVector.clear();
	for (auto sock : inSockptrVector)
	{
		if (FD_ISSET(sock, &fd))
		{
			outSockptrVector.push_back(sock);
		}
	}
}

int main()
{

#pragma region Listening_socket_preparation
	WSADATA wsadata;
	int startupError = WSAStartup(MAKEWORD(2, 2), &wsadata);
	if (startupError != 0)
	{
		cout << WSAGetLastError() << endl;
		cout << "Startup error" << endl;
		getchar();
		return -1;
	}

	//listening socket - creation
	int addressFamily = AF_INET;
	int socketType = SOCK_STREAM;
	int protocol = IPPROTO_TCP;

	SOCKET listenSock = socket(addressFamily, socketType, protocol);
	if (listenSock == INVALID_SOCKET)
	{
		cout << WSAGetLastError() << endl;
		cout << "invalid socket error" << endl;

		WSACleanup();
		getchar();
		return -1;
	}

	//listening socket - binding
	sockaddr_in localAddress;
	int port = 8000;
	const char* ip = "127.0.0.1";

	localAddress.sin_family = addressFamily;
	localAddress.sin_port = htons(port);
	memset(&localAddress.sin_zero, 0, sizeof(localAddress.sin_zero));
	localAddress.sin_addr.s_addr = inet_addr(ip);

	int bindError = bind(listenSock, (sockaddr*)&localAddress, sizeof(sockaddr));
	if (bindError == -1)
	{
		cout << WSAGetLastError() << endl;
		cout << "bind error" << endl;

		WSACleanup();
		getchar();
		return -1;
	}

	//listening socket - listening
	int backlog = 2;
	int listenError = listen(listenSock, backlog);
	if (listenError == -1)
	{
		cout << WSAGetLastError() << endl;
		cout << "listen error" << endl;

		WSACleanup();
		getchar();
		return -1;
	}

#pragma endregion

	//number of file descripter sets. 
	//In window, this wil be ignored. Thus can be any value
	int nfds = 0;

	//file descripter set
	fd_set readfds;
	fd_set writefds;
	fd_set exceptfds;

	//vector of socket pointers to be added to fds
	vector<SOCKET> readSockVector;
	vector<SOCKET> writeSockVector;
	vector<SOCKET> exceptSockVector;

	//vector of socket pointers that remained in fd_set after select
	vector<SOCKET> returnedreadSockVector;
	vector<SOCKET> returnedwriteSockVector;
	vector<SOCKET> returnedexceptSockVector;

	//maximum time limit
	timeval timeout;
	timeout.tv_sec = 2;

	bool terminate = false;


	//use of vector<char[recvBuflen]> instead of vector<char*> will result in c3074 error
	//This is because vector stl implementation will call () operator on data type char[recvBuflen]
	//, thus calling char[recvBuflen]() which is not allowed 
	vector<char*> msgVector;
	msgVector.clear();

	//add listening socket to readSocPtrVector. move
	readSockVector.push_back(std::move(listenSock));

	while (!terminate)
	{
		//select will remove returned sockets from fd_set.
		//Therefore fd_set should be reinitialized for every function 'select' call
		FillFdsFromVector(readSockVector, readfds);
		FillFdsFromVector(writeSockVector, writefds);
		FillFdsFromVector(exceptSockVector, exceptfds);

		//block until packet is received
		int remainNum = select(nfds, &readfds, &writefds, &exceptfds, &timeout);
		//if select returned 0 due to timeout, continue
		if (!remainNum)
		{
			continue;
		}

		//search for sockets that are remained in fds
		FillVectorFromFds(readfds, readSockVector, returnedreadSockVector);
		FillVectorFromFds(writefds, writeSockVector, returnedwriteSockVector);
		FillVectorFromFds(exceptfds, exceptSockVector, returnedexceptSockVector);

		//get result from each returned sockets
		//read
		for (auto sock : returnedreadSockVector)
		{
			//if listening Socket received pac
			if (sock == listenSock)
			{
				sockaddr remoteAddress;
				int addrLen = sizeof(remoteAddress);
				auto connectedSock = accept(sock, &remoteAddress, &addrLen);
				if (connectedSock == INVALID_SOCKET)
				{
					cout << WSAGetLastError() << endl;
					cout << "accept error" << endl;
					WSACleanup();
					getchar();
					return -1;
				}

				cout << "client connected" << endl;
				auto writeSock = connectedSock; //should be copy
				//add connectedSock to readSockVector, writeSockVector
				//don't forget to move. else, all socket will refer to the same socket after for loop
				readSockVector.push_back(std::move(connectedSock));
				writeSockVector.push_back(std::move(writeSock));
			}
			//normal socket
			//receive data
			else
			{
				char recvBuf[recvBuflen];
				int recvError = recv(sock, recvBuf, recvBuflen, 0);
				if (recvError == -1)
				{

					cout << WSAGetLastError() << endl;
					cout << "recv error" << endl;
					WSACleanup();
					getchar();
					return -1;

				}
				cout << "received message from socket #"<<sock<<": " << recvBuf << endl;
				//add msg to queue
				msgVector.push_back(recvBuf);
			}
		}
		//write
		//send all message in msgVector to all clients if all sockets are ready to send data
		if (returnedwriteSockVector.size() == writeSockVector.size() &&(msgVector.size()!=0))
		{
			cout << "echo message to clients " << endl;
			for (auto sock : returnedwriteSockVector)
			{
				for (auto msg : msgVector)
				{
					cout << "send to socket #"<<sock<<": " << msg << endl;
					if (strcmp(msg, "terminate") == 0)
					{
						terminate = true;
					}
					// BEWARE: msgVector is not vector of char[sendBuflen] but a vector of char*
					//using sizeof(msg) instead of sendBuflen will result in sending only part of the message
					int sendError = send(sock, msg, sendBuflen, 0);
					if (sendError == -1)
					{
						cout << WSAGetLastError() << endl;
						cout << "send error" << endl;
						WSACleanup();
						getchar();
						return -1;
					}
				}
			}
			cout << "sent messages to clients" << endl;
			msgVector.clear();
		}
		//except
		for (auto sockptr : returnedexceptSockVector)
		{
			//implementation
		}
	}


	cout << "terminate" << endl;
	closesocket(listenSock);
	WSACleanup();
	getchar();
}
