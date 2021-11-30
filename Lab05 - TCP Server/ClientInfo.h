#pragma once
#include <winsock.h>
#include <string>
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
	std::string username;
	bool loggedIn = false;;
};

