#pragma once
#include <vector>

#include "SysThread.h"
#include "ClientInfo.h"

class Networking : public SysThread
{
	ClientInfo clientInfo;
	std::vector<Networking*>* threadList;
	CRITICAL_SECTION* critical_section;
	void cleanString(std::string& in);
	void WorkOnRequest(char * recBuf);
	std::string splitResponse(std::string& response);
	std::string GetAllOnlineUsers();
	void NotifyAllUsers(char protocol, const std::string &notify);
	bool IsUserLoggedIn(std::string& user);
	void SendMessageToUser(std::string& user, char* recBuf);
	void SendFileAcceptRequest(std::string& user, std::string& receiver, char*recBuf);
	void SendFileRequestResponse(std::string& user, std::string& receiver, char* recBuf);
	void SendFile(std::string& user, std::string& receiver, std::string& filename, char* recBuf);

public:
	Networking(std::vector<Networking*>* threadList, CRITICAL_SECTION* critical_section, ClientInfo &clientInfo);
	void run(void);
};

