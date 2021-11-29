#pragma once
#include <winsock.h>
class ClientInfo
{
	

public:
	ClientInfo() {};
	ClientInfo(SOCKET clientSocket, sockaddr_in clientAddr)
	{
		this->clientAddr = clientAddr;
		this->clientSocket = clientSocket;
	}

	SOCKET clientSocket;
	sockaddr_in clientAddr;
};

