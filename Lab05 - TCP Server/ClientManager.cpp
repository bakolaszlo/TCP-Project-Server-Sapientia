#include "ClientManager.h"

//bool ClientManager::usersInitialized = false;
bool ClientManager::initializeUsers()
{

	std::ifstream input("clients.txt");
	std::string line;
	char delimiter = ':';
	while (std::getline(input, line)) {
		std::vector<std::string> output = split(line, delimiter);
		users.emplace(output[0], output[1]);
	}
	//usersInitialized = true;
	return true;
}

std::vector<std::string> ClientManager::split(const std::string& str, char delim)
{
	std::vector<std::string> strings;
	size_t start;
	size_t end = 0;
	while ((start = str.find_first_not_of(delim, end)) != std::string::npos) {
		end = str.find(delim, start);
		strings.push_back(str.substr(start, end - start));
	}
	return strings;
}

ClientManager::ClientManager(ClientInfo& clientInfo)
{
	this->clientInfo = clientInfo;
	
		initializeUsers();
}

bool ClientManager::Login(std::string& username, std::string& password)
{
	if (users.find(username) == users.end())
	{
		return false;
	}
	else
	{
		if (users[username] == password)
		{
			clientInfo.username = username;
			return true;
		}
		return false;
	}
	return false;
}

