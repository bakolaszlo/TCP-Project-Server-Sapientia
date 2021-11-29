#define _WINSOCK_DEPRECATED_NO_WARNINGS
#include <stdio.h>

#include "SysThread.h"
#include "Networking.h"
#include <vector>
#include "ClientInfo.h"
#include <iostream>

#pragma comment(lib, "ws2_32.lib")
CRITICAL_SECTION CriticalSection;

int main() {

	if (!InitializeCriticalSectionAndSpinCount(&CriticalSection, 0x00000400))
		return -1;

	std::vector<Networking*> threadList;
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
	sockaddr_in serverAddr;
	serverAddr.sin_family = AF_INET;
	serverAddr.sin_addr.s_addr = inet_addr("127.0.0.1");
	serverAddr.sin_port = htons(13000);
	if (bind(ListenSocket,
		(SOCKADDR*)&serverAddr,
		sizeof(serverAddr)) == SOCKET_ERROR) {
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
		sockaddr_in clientAddr;
		int nSize = sizeof(clientAddr);

		auto AcceptSocket = accept(ListenSocket, (SOCKADDR*)&clientAddr, &nSize);
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
		ClientInfo clientInfo(AcceptSocket,clientAddr);

		std::cout << "Client connected from " << inet_ntoa(clientAddr.sin_addr) << std::endl;
		auto thread = new Networking(&threadList,&CriticalSection,clientInfo);
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
