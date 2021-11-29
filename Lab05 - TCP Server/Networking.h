#pragma once
#include <vector>

#include "SysThread.h"

class Networking : public SysThread
{
	SOCKET socket;
	std::vector<Networking*>* threadList;
	CRITICAL_SECTION* critical_section;
	
public:
	Networking(SOCKET socket, std::vector<Networking*>* threadList, CRITICAL_SECTION* critical_section);
	void run(void);
};

