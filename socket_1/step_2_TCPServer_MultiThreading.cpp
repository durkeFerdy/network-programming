//TCP mutithreading server side
//race condition not considered for simplicity

using namespace std;

#include <iostream>
#include <WinSock2.h>
#include <thread>
#include <queue>
#include <string>

#pragma comment(lib, "ws2_32.lib")

#pragma warning (disable: 4996)

//In thread routines, only variables passed via parameter can be used unlike c# internal function.
//To avoid thread routine function parameter getting too long, some variables are defined using preprocessor
#define recvBuflen 100
#define clientTO 3


//receive message
static void clientRoutine(SOCKET& lSock, SOCKET&& cSock, bool& terminate)
{
	cout << "client thread started" << endl;
	while (!terminate)
	{
		char recvBuf[recvBuflen];
		int recvError = recv(cSock, recvBuf, recvBuflen, 0);

		if (recvError == -1)
		{
			cout << "recv error" << endl;
			break;
		}

		cout << recvBuf << endl;
		
		//strcmp returns 0 if two comparables are equal
		//recvBuf == "end" will check whether each char* points same location or not
		//therefore strcmp should be used to check char sequence equality
		if (strcmp(recvBuf, "end")==0)
		{
			cout << "ended connection" << endl;
			//set terminate flag
			terminate = true;
			//close listening socket 
			//close Listening socket In order to return listening Thread accept. 
			//else, it will be blocked unitl max client number is reached
			closesocket(lSock);
		}
	}
	cout << "client thread ended" << endl;
	closesocket(cSock);
}


//thread cannot use reference argument such as SOCKET&. Therefore SOCKET pointer should be used 
//ref: https://stackoverflow.com/questions/61974060/c-threading-no-matching-overloaded-function-found
static void listeningRoutine(void (cRoutine)(SOCKET&, SOCKET&&, bool&), SOCKET& sock, bool& terminate)
{
	cout << "listening thread started" << endl;
	//thread is neither copyable nor assignable. 
	//ex. threads := array of thread. th := thread object.
	//threads[0] = th -> error. attempt to use deleted function. 
	//In this case, the deleted function is copy constructer of thread. 
	//threads[0] = std::move(th) will solve the problem
	//another solution. instead of array of threads, use array of thread pointers
	thread threadArr[clientTO];

	int clientNum = 0;
	while ((clientNum < clientTO) && !terminate)
	{
		sockaddr recvAddress;
		int len = sizeof(sockaddr);
		//accept should return when terminate flag is on.
		//therefore, listeningSocket will be closed by client on termination
		cout << "waiting for client connection" << endl;
		SOCKET clientSock = accept(sock, &recvAddress, &len);
		if (clientSock == INVALID_SOCKET)
		{
			//when listening socket is closed, accept will fail. 
			//by conitinue, check terminate flag
			continue;
		}
		cout << "client connected" << endl;
		//clientSock should not be passed with reference
		//in other world, either cRoutine(SOCKET&, SOCKET&&, bool&) or cRoutine(SOCKET&, SOCKET, bool&) is ok
		//however cRoutine(SOCKET&, SOCKET&, bool&) will end up making client threads reference same client socket
		//clientSock is redefined every loop. however may have same memory location, thus causing all client thread refer to same socket when passed by lvaue reference
		thread clientThread(cRoutine,ref(sock), clientSock, ref(terminate));
		//move constructor is used. since thread copy constructor is deleted
		threadArr[clientNum] = move(clientThread); 
		clientNum++;
	}
	if(clientNum==clientTO)
	{
		cout << "max client reached" << endl;
		//until listening socket is closed, clients can still successfully connect.
		//however, no new client thread will be created in this implementation
		//ref: https://stackoverflow.com/questions/2409277/can-connect-call-on-socket-return-successfully-without-server-calling-accept
	}
	//wait for all client threads to terminate
	//since thread copy constructor is deleted, use of auto instead of auto&& will result in error
	for (auto&& t: threadArr)
	{
		//if some client thread ended first, joining it would result in error.(abort is called error)
		if (!t.joinable()) 
		{
			continue;
		}
		t.join(); 
	}


	cout << "listening thread ended" << endl;
}

int main()
{

#pragma region Listening_socket_preparation
	WSADATA wsadata;
	int startupError = WSAStartup(MAKEWORD(2, 2), &wsadata);
	if(startupError!=0)
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
	if(listenSock == INVALID_SOCKET)
	{
		cout << WSAGetLastError() << endl;
		cout << "invalid socket error" << endl;
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
	if(bindError == -1)
	{
		cout << WSAGetLastError() << endl;
		cout << "bind error" << endl;
		getchar();
		return -1;
	}

	//listening socket - listening
	int backlog = 2;
	int listenError = listen(listenSock, backlog);
	if(listenError ==-1 )
	{
		cout << WSAGetLastError() << endl;
		cout << "listen error" << endl;
		getchar();
		return -1;
	}

#pragma endregion

	bool terminateFlag = false;

	thread listeningThread(listeningRoutine, clientRoutine,ref(listenSock), ref(terminateFlag));

	cout << "main" << endl;
	
	

	//wait for listening thread to end. 
	listeningThread.join();

	cout << "terminate flag: "<<terminateFlag << endl;
	WSACleanup();
	getchar();
}
