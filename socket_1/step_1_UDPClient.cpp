//client side - UDP protocol & IPv4

using namespace std;

#include <iostream>
#include <Winsock2.h>
//#include<WS2tcpip.h>

#pragma comment(lib,"ws2_32.lib")

int main()
{

#pragma region init_winsock_lib
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

	//socket code 

	//udp client socket
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
	const char* serverIp = "127.0.0.1"; //loop back to receive packet with the very device that sent message

	socketAddress.sin_family = addressFamily;
	socketAddress.sin_port = htons(port);
	memset(socketAddress.sin_zero, 0, sizeof(socketAddress.sin_zero));

	socketAddress.sin_addr.s_addr = inet_addr(serverIp);
	//ref: https://stackoverflow.com/questions/36683785/inet-addr-use-inet-pton-or-inetpton-instead-or-define-winsock-deprecated/47397620
	//inet_addr(ip) is deprecated. it could cause warning. 
	//use InetPton instead or just ignore the warning. 
	// - To use InetPton InetPton(addressFamily, serverIp, & socketAddress.sin_addr); (also include <WS2tcpip.h>)
	// -- actually InetPton causes build error , and still couldn't figure out why.
	// - To ignore the warning add #pragma warning(disable:4996)
	
	//ways to initialize socketAddress.sin_addr
#pragma region init_sin_addr_methods
	//method 1. one by one assignment
	/*
	socketAddress.sin_addr.S_un.S_un_b.s_b1 = 127;
	socketAddress.sin_addr.S_un.S_un_b.s_b2 = 0;
	socketAddress.sin_addr.S_un.S_un_b.s_b3 = 0;
	socketAddress.sin_addr.S_un.S_un_b.s_b4 = 1;
	*/

	//method 2. use of InetPton 
	//const char* serverIp = "127.0.0.1";
	//InetPton(AF_INET, (PCWSTR)serverIp, &socketAddress.sin_addr);

	//method 3 set sockaddr from url by DNS query. url -> sockaddr
	//below implementation is not tested yet. 
	/*
	const char* url = "www.google.com";
	const char* hostName = url; //dns
	//port number. either in form of name(ex: http) or corresponding decinal(ex: 80)
	//(http for instance is alias for port number 80)
	//Tip) from directory "%WINDIR%\system32\drivers\etc\services", list of possible port number can be found
	const char* servName = "80"; // or "http"

	addrinfo hints;
	//initialize hints
	memset(&hints, 0, sizeof(hints));
	hints.ai_family = AF_INET;

	addrinfo* res = nullptr; //double pointer is used to implement linked list structure for result(=out)

	//explicit casting of PCSTR, PADDRINFO is needed. PADDRINFO <- addrinfo**
	int getaddrinfoError = getaddrinfo((PCSTR)hostName, (PCSTR)servName, &hints, (PADDRINFOA*)&res);
	addrinfo* initRes = res;
	if (getaddrinfoError != 0 && res != nullptr)
	{
		cout << "getaddrinfo failed" << endl;
		freeaddrinfo(initRes);
		return -1;
	}

	//get valid result
	while(!(res->ai_addr)&& res-> ai_next)
	{
		res = res->ai_next;
	}

	//address not found(while loop broke by reaching end of res linked list(res->ai_next = nullptr))
	if(!res-> ai_addr)
	{
		cout << "not address found" << endl;
		freeaddrinfo(initRes);
		return -1;
	}

	//use this instead of socketAddress variable defined above
	sockaddr* socketAddressFromUrl = res->ai_addr;
	*/
#pragma endregion

	//bind
	//when sendto is called before binding, winsock lib will auto-bind socket to remaing port. 
	//In this case bind is not necessary, since sendto will be called before any other host sends message to this client

	//send message to server
	const int buflen = 512;
	char sendMessage[buflen];

	//Wrong trial: int buflen = strlen(sendMessgae) sends nothing. 
	//ref: https://stackoverflow.com/questions/36800440/why-strlen-returning-2-or-1-everytime
	//strlen returns length before first 'null(\0)'. 
	//If messgae ends with null(as is string), it will give message length as one intended.
	//However with message that does not necessarily ends with null(ex: char array). it searches for first 0 in binary code, and the result is hardly the one intended.

	cin.getline(sendMessage, buflen);

	//sockUDP = client socket, sockAddress = server socketAddress
	int sendError = sendto(sockUDP, sendMessage, sizeof(sendMessage), 0, (sockaddr*)&socketAddress, sizeof(sockaddr_in));
	if (sendError == SOCKET_ERROR)
	{
		cout << WSAGetLastError() << endl;
		cout << "recv error" << endl;
		char w = getchar();
		return -1;
	}

	cout << "sent:" << sendMessage << endl;

	const int recvBuflen = 512;
	char recvMessage[recvBuflen];
	int fromlen = sizeof(sockaddr_in);

	//receive data from server
	int recvLen = recvfrom(sockUDP, recvMessage, recvBuflen, 0, (sockaddr*)&socketAddress, &fromlen);
	if(recvLen == -1)
	{
		cout << WSAGetLastError() << endl;
		cout << "recv error" << endl;
		char w = getchar();
		return -1;
	}
	else
	{
		cout << "message recevied: " << endl;
		cout << recvMessage << endl;
	}

	//shutdown & close socket 
	int closesocketError = closesocket(sockUDP);


#pragma region end_winsock_lib
	//end using winsock2 library
	WSACleanup();
	getchar();
#pragma endregion

}