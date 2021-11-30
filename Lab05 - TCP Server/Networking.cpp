#include "Networking.h"
#include <sstream>
#include <stdio.h>
#include <iostream>
#include "ClientManager.h"

//4bytes as host
//1 byte for type
//1 byte for sub-type
//12 byte username origniated from


enum PROTOCOL {
	LOGIN = 'L',
	MESSAGE = 'M',
	QUIT = 'Q',
	REQUESTRESPONSE = 'R',
	FILLER = '>',
};

void Networking::cleanString(std::string& in)
{
	in.erase(remove(in.begin(), in.end(), PROTOCOL::FILLER), in.end());
}

Networking::Networking(std::vector<Networking*>* threadList, CRITICAL_SECTION* critical_section,ClientInfo &clientInfo)
{
	this->clientInfo = clientInfo;
	this->threadList = threadList;
	this->critical_section = critical_section;
}

void Networking::run(void)
{
	int iResult;
	const int BufLen = 1024;
	char recBuf[BufLen];
	char host[6];
	std::string response;
	host[0] = 127;
	host[1] = '0';
	host[2] = '0';
	host[3] = 1;
	host[5] = 0;

	while (1)
	{
		iResult = recv(this->clientInfo.clientSocket, recBuf, sizeof(recBuf), 0);
		if (iResult > 0)
		{
			recBuf[iResult] = '\0';
			std::cout << "Received " << recBuf << " from " << inet_ntoa(clientInfo.clientAddr.sin_addr) << std::endl;

			// Convert the string to upper case
			recBuf[4] = std::toupper(recBuf[4]);

			if (recBuf[4] == PROTOCOL::LOGIN)
			{
				std::string input(recBuf);
				std::string username = input.substr(5, 12);
				cleanString(username);
				std::string password = input.substr(5 + 12, 12);
				cleanString(password);
				ClientManager cm(clientInfo);
				host[4] = PROTOCOL::REQUESTRESPONSE;
				response = std::string(host);

				//if login was succesfull
				if (cm.Login(username, password))
				{
					response.append("Logged in succesfully.");
				}
				else {
					response.append("Login attempt failed.");
				}
				//Login();
			}
			if (recBuf[4] == PROTOCOL::QUIT)
			{
				closesocket(clientInfo.clientSocket);
				return;
			}

			// send( ) may not be able to send the complete data in one go.
			// So try sending the data in multiple requests
			int nCntSend = 0;
			const char* pBuffer = response.c_str();

			while ((nCntSend  != 256))
			{
				nCntSend = send(clientInfo.clientSocket, pBuffer, 256, 0);
				if (nCntSend == -1)
				{
					std::cout << "Error sending the data to " << inet_ntoa(clientInfo.clientAddr.sin_addr) << std::endl;
					break;
				}
				//if (nCntSend == iResult)
				//	break;

				pBuffer += nCntSend;
				//iResult -= nCntSend;
			}
			
		}
		else
		{
			std::cout << "Error reading the data from " << inet_ntoa(clientInfo.clientAddr.sin_addr) << std::endl;
			break;
		}

	}
	//if (threadList->size())
	//{
	//	
	//	int pos = 0;
	//	char SendBuf[BufLen];
	//	// ido lekerese es elkuldese
	//	//---------------------------------------------
	//   // Send a datagram
	//	while (true) {

	//	
	//		for (auto thread : *threadList)
	//		{
	//			if (thread.isConnected())
	//			{
	//				char RecvBuf[BufLen];
	//				printf("Waiting for the client to send something...\n");
	//				iResult = recv(thread.clientInfo.clientSocket, RecvBuf, BufLen, 0);
	//				printf("Wait complete.\n");
	//				if (iResult == SOCKET_ERROR)
	//				{
	//					printf("Error occured during recv: %ld\n",
	//						WSAGetLastError());
	//					WSACleanup();
	//					return;
	//				}
	//				else if (iResult == 0)
	//				{
	//					thread.setToDisconnect();
	//					return;
	//				}
	//				RecvBuf[iResult] = '\0';
	//				printf("%s\n", RecvBuf);
	//				/// insert working stuff here

	//			
	//			
	//				//sending response
	//				std::stringstream ss;
	//				ss << "Answer From Thread You are client nr:" << pos << '\0';
	//				printf("Sending a datagram... %i\n", pos);
	//				const std::string tmp = ss.str();
	//				const char* cstr = tmp.c_str();
	//				strcpy_s(SendBuf, cstr);
	//				iResult = send(thread.clientInfo.clientSocket, SendBuf, BufLen, 0);
	//				if (iResult == SOCKET_ERROR)
	//				{
	//					printf("Error occured during recv: %ld\n",
	//						WSAGetLastError());
	//					WSACleanup();
	//					
	//					return;
	//				}
	//				printf("Datagram sent...\n");
	//			}
	//			else
	//			{
	//				EnterCriticalSection(critical_section);
	//				auto temporaryThread = &thread;
	//				threadList->erase(threadList->begin() + pos);
	//				delete temporaryThread;
	//				pos--;
	//				LeaveCriticalSection(critical_section);
	//			}
	//			pos++;
	//		}
	//	
	//	}
	//}
	//---------------------------------------------
	// When the application is finished sending, close the socket.
	//printf("Finished sending.\n");
}
