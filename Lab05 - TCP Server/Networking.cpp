#include "Networking.h"
#include <winsock.h>
#include <sstream>
#include <stdio.h>

Networking::Networking(SOCKET socket, std::vector<Networking*>* threadList, CRITICAL_SECTION* critical_section)
{
	this->socket = socket;
	this->threadList = threadList;
	this->critical_section = critical_section;
}

void Networking::run(void)
{
	int iResult;
	//printf("Receiving datagrams...\n");
	const int BufLen = 1024;
	//char RecvBuf[BufLen];
	//iResult = recv(socket, RecvBuf, BufLen, 0);
	//if (iResult == SOCKET_ERROR)
	//{
	//	printf("Error occured during recv: %ld\n",
	//		WSAGetLastError());
	//	WSACleanup();
	//	return;
	//}
	//printf("Message received: %s\n", RecvBuf);
	
	if (threadList->size())
	{
		
		int pos = 0;
		char SendBuf[BufLen];
		// ido lekerese es elkuldese
		//---------------------------------------------
	   // Send a datagram
		while (true) {

		
			for (auto thread : *threadList)
			{
				if (thread->isConnected())
				{
					char RecvBuf[BufLen];
					printf("Waiting for the client to send something...\n");
					iResult = recv(thread->socket, RecvBuf, BufLen, 0);
					printf("Wait complete.\n");
					if (iResult == SOCKET_ERROR)
					{
						printf("Error occured during recv: %ld\n",
							WSAGetLastError());
						WSACleanup();
						return;
					}
					else if (iResult == 0)
					{
						thread->setToDisconnect();
						return;
					}
					RecvBuf[iResult] = '\0';
					printf("%s\n", RecvBuf);
					/// insert working stuff here

				
				
					//sending response
					std::stringstream ss;
					ss << "Answer From Thread You are client nr:" << pos << '\0';
					printf("Sending a datagram... %i\n", pos);
					const std::string tmp = ss.str();
					const char* cstr = tmp.c_str();
					strcpy_s(SendBuf, cstr);
					iResult = send(thread->socket, SendBuf, BufLen, 0);
					if (iResult == SOCKET_ERROR)
					{
						printf("Error occured during recv: %ld\n",
							WSAGetLastError());
						WSACleanup();
						
						return;
					}
					printf("Datagram sent...\n");
				}
				else
				{
					EnterCriticalSection(critical_section);
					auto temporaryThread = thread;
					threadList->erase(threadList->begin() + pos);
					delete temporaryThread;
					pos--;
					LeaveCriticalSection(critical_section);
				}
				pos++;
			}
		
		}
	}
	//---------------------------------------------
	// When the application is finished sending, close the socket.
	//printf("Finished sending.\n");
}
