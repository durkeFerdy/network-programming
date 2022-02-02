//initialization and clean up of winsock2 library 

using namespace std;

#include <iostream>
#include <Winsock2.h>

//add ws2_32 to project library in order to use winsock2, ws2tcpip etc
//without code below, building this code from 'developer command prompt for VS' with 'cl' command will fail
#pragma comment(lib,"ws2_32.lib")

int main()
{
	//initialize winsock2 library
	WORD mVersionRequested = MAKEWORD(2, 2); //winsock2 version 2.2.
	WSADATA wsadata;
	int startupError;
	startupError = WSAStartup(mVersionRequested, &wsadata);
	if (startupError != 0)
	{
		//WSAGetLastError should be called right after occurence of error 
		cout << WSAGetLastError() << endl;
		cout << "Startup failed" << endl;

		//caching this is not necessary but visual studio underlines this line with warning, and it is bit annoying.
		char w = getchar();
		return -1;
	}

	//socket related code will be implemented here

	//end using winsock2 library
	WSACleanup();
	getchar();
}