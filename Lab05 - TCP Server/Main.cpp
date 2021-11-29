#define _WINSOCK_DEPRECATED_NO_WARNINGS
#define _CRT_SECURE_NO_WARNINGS
#include <stdio.h>

#include "winsock2.h"
#include "SysThread.h"
#include "Networking.h"
#include <vector>

#pragma comment(lib, "ws2_32.lib")
CRITICAL_SECTION CriticalSection;

char * getTime()
{
	int iResult;
	WSADATA wsaData;
	iResult = WSAStartup(MAKEWORD(2, 2), &wsaData);
	if (iResult != NO_ERROR)
		printf("Hiba a WSAStartup() –nál\n");

	SOCKET KlientSocket;
	//https://msdn.microsoft.com/enus/library/windows/desktop/ms737625%28v=vs.85%29.aspx
	KlientSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

	int Port = 13;
	char IP[10] = "127.0.0.1";
	sockaddr_in ServerAddr;
	int AddrLen = sizeof(ServerAddr);
	ServerAddr.sin_family = AF_INET;
	ServerAddr.sin_addr.s_addr = inet_addr(IP);
	ServerAddr.sin_port = htons(Port);

	if (connect(KlientSocket, (SOCKADDR*)&ServerAddr, AddrLen) == SOCKET_ERROR)
	{
		printf("Hiba a kapcsolódásnál a következő hibakóddal: %ld\n",
			WSAGetLastError());
		// WSACleanup befejezi a Windows Sockets működését minden szálban!!!!
		WSACleanup();
		exit(1);
	}

	const int RecBufLen = 1024;
	char RecBuf[RecBufLen];
	iResult = recv(KlientSocket, RecBuf, RecBufLen - 1, 0);
	//Ha hiba történt a fogadásnál
	if (iResult == SOCKET_ERROR)
	{
		printf("Hiba a fogadásnál a következő hibakóddal: %d\n",
			WSAGetLastError());
		closesocket(KlientSocket);
		// WSACleanup befejezi a Windows Sockets működését minden szálban!!!!
		WSACleanup();
		exit(1);
	}
	//ha lezátuk valhol a socketet
	if (iResult == 0)
	{
		exit(1);
	}
	//kiiratjuk az eredményt
	RecBuf[iResult] = '\0';
	iResult = shutdown(KlientSocket, SD_SEND); // keeps the socket intact
	return RecBuf;
	//Kapcsolat bontása

}
int main() {

	if (!InitializeCriticalSectionAndSpinCount(&CriticalSection, 0x00000400))
		return -1;

	std::vector<Networking *> threadList;
	// Initialize Winsock.
	WSADATA wsaData;
	int iResult = WSAStartup(MAKEWORD(2, 2), &wsaData);
	if (iResult != NO_ERROR) {
		printf("Error at WSAStartup()\n");
		return 1;
	}
	//----------------------
	// Create a SOCKET for listening for
	// incoming connection requests.
	SOCKET ListenSocket;
	ListenSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (ListenSocket == INVALID_SOCKET) {
		printf("Error at socket(): %ld\n", WSAGetLastError());
		WSACleanup();
		return 1;
	}
	//----------------------
	// The sockaddr_in structure specifies the address family,
	// IP address, and port for the socket that is being bound.
	sockaddr_in service;
	service.sin_family = AF_INET;
	service.sin_addr.s_addr = inet_addr("127.0.0.1");
	service.sin_port = htons(13000);
	if (bind(ListenSocket,
		(SOCKADDR*)&service,
		sizeof(service)) == SOCKET_ERROR) {
		printf("bind() failed.\n");
		closesocket(ListenSocket);
		WSACleanup();
		return 1;
	}
	//----------------------
	// Listen for incoming connection requests.
	// on the created socket
	if (listen(ListenSocket, 1) == SOCKET_ERROR) {
		printf("Error listening on socket.\n");
		closesocket(ListenSocket);
		WSACleanup();
		return 1;
	}
	//----------------------
	// Create a SOCKET for accepting incoming requests.
	//----------------------
	while (true)
	{
		printf("Waiting for client to connect...\n");
		// Accept the connection.
		auto AcceptSocket = accept(ListenSocket, NULL, NULL);
		if (AcceptSocket == INVALID_SOCKET) {
			printf("accept failed: %d\n", WSAGetLastError());
			closesocket(ListenSocket);
			WSACleanup();
			return 1;
		}
		else
			printf("Client connected.\n");
		//-----------------------------------------------
		// Call the recvfrom function to receive datagrams
		// on the bound socket.
		auto thread = new Networking(AcceptSocket,&threadList,&CriticalSection);
		thread->start();
		EnterCriticalSection(&CriticalSection);
		threadList.push_back(thread);
		LeaveCriticalSection(&CriticalSection);
	}

	DeleteCriticalSection(&CriticalSection);
	//---------------------------------------------
	// Clean up and quit.
	printf("Exiting.\n");
	WSACleanup();
	return 0;
}
