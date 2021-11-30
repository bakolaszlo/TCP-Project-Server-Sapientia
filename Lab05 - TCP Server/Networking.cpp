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
	START= 'S',
	CONTINUATION = 'C',
	END = 'E',
};

void Networking::cleanString(std::string& in)
{
	in.erase(remove(in.begin(), in.end(), PROTOCOL::FILLER), in.end());
}

void Networking::workOnRequest(char* recBuf)
{
	char host[7];

	host[0] = 127;
	host[1] = '0';
	host[2] = '0';
	host[3] = 1;
	host[4] = PROTOCOL::FILLER;
	host[5] = PROTOCOL::END;
	host[6] = 0;
	std::string response(host);
	
	if (recBuf[4] == PROTOCOL::LOGIN && clientInfo.loggedIn == false)
	{
		std::string input(recBuf);
		std::string username = input.substr(6, 12);
		cleanString(username);
		std::string password = input.substr(6 + 12, 12);
		cleanString(password);
		ClientManager cm(&clientInfo);
		response[4] = PROTOCOL::REQUESTRESPONSE;

		//if login was succesfull
		if (cm.Login(username, password))
		{
			response.append("1");
		}
		else {
			response.append("0");
		}
		//Login();
	}
	else if (!clientInfo.loggedIn)
	{
		response[4] = PROTOCOL::REQUESTRESPONSE;
		response.append("You need to be logged in for further actions.");
	}
	else
	{
		//further actions here
	}


	// send( ) may not be able to send the complete data in one go.
	// So try sending the data in multiple requests
	int nCntSend = 0;
	std::string result = splitResponse(response);
	const char* pBuffer = result.c_str();
	int bytesSent = 0;
	size_t messageSize = 256;
	const int messageTotalSize = response.size();

	while (bytesSent <= result.size())
	{
		nCntSend = send(clientInfo.clientSocket, pBuffer, messageSize, 0);
			
		
		if (nCntSend == -1)
		{
			std::cout << "Error sending the data to " << inet_ntoa(clientInfo.clientAddr.sin_addr) << std::endl;
			clientInfo.loggedIn = false;
			break;
		}
		//if (nCntSend == iResult)
		//	break;

		pBuffer += nCntSend;
		bytesSent += nCntSend;
		//iResult -= nCntSend;
	}
}

std::string Networking::splitResponse(std::string& response)
{
	size_t messageLength = 256;
	std::string result;
	std::string placeHolder = response.substr(0, 6);
	response.erase(0, 6);
	if (response.size() <= 250)
	{
		placeHolder[5] = PROTOCOL::END;
	}
	else
	{
		placeHolder[5] = PROTOCOL::START;
	}

	result.append(placeHolder);

	std::string temp = response.substr(0, 250);
	response.erase(0, 250);
	
	result.append(temp);

	placeHolder[5] = PROTOCOL::CONTINUATION;

	while (response.size() != 0)
	{
		temp = response.substr(0, 250);
		response.erase(0, 250);

		if (response.size() == 0)
		{
			placeHolder[5] = PROTOCOL::END;
		}

		result.append(placeHolder);
		result.append(temp);
	}

	return result;
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
	const int BufLen = 256;
	char recBuf[BufLen];

	while (1)
	{
		iResult = recv(this->clientInfo.clientSocket, recBuf, sizeof(recBuf), 0);
		if (iResult > 0)
		{
			recBuf[iResult] = '\0';
			std::cout << "Received " << recBuf << " from " << inet_ntoa(clientInfo.clientAddr.sin_addr) << std::endl;

			
			// Convert the string to upper case
			recBuf[4] = std::toupper(recBuf[4]);
			if (recBuf[4] == PROTOCOL::QUIT)
			{
				closesocket(clientInfo.clientSocket);
				return;
			}
			else
			{
				workOnRequest(recBuf);
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
