#pragma once
#include <WinSock2.h>
class ClientInfo
{
	

public:
	ClientInfo(SOCKET clientSocket, sockaddr_in clientAddr)
	{
		this->clientAddr = clientAddr;
		this->clientSocket = clientSocket;
	}
	SOCKET clientSocket;
	struct sockaddr_in clientAddr;
};

