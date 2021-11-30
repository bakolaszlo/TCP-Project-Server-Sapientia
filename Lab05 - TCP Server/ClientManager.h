#pragma once
#include "ClientInfo.h"
#include <string>
#include <map>
#include <fstream>
#include <vector>

class ClientManager
{
private:
	ClientInfo clientInfo;
	bool setUserName;
	bool initializeUsers();
	std::map<std::string, std::string> users;
	std::vector<std::string> split(const std::string& str, char delim);

public:
	ClientManager(ClientInfo &clientInfo);
	//static bool usersInitialized;

	bool Login(std::string &username, std::string &password);
};

