#pragma once
#include <winsock.h>
#include <string>
#include <map>
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
	bool loggedIn = false;
	std::map<std::string, bool> acceptedFiles;
};

