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
public:
	Networking(std::vector<Networking*>* threadList, CRITICAL_SECTION* critical_section, ClientInfo &clientInfo);
	void run(void);
};

