#include "Networking.h"
#include <sstream>
#include <stdio.h>
#include <iostream>
#include "ClientManager.h"


#define HEADER_LENGTH 7
//4bytes as host
//1 byte for type
//1 byte for sub-type
//12 byte username origniated from


enum PROTOCOL {
	LOGIN = 'L',
	MESSAGE = 'm',
	QUIT = 'Q',
	REQUESTRESPONSE = 'R',
	FILLER = '>',
	PEOPLE = 'P',
	START = 'S',
	CONTINUATION = 'C',
	END = 'E',
	MESSAGEFOREVERYONE = 'M',
	FILEREQUEST = 'F',
	ATTACHEMENT = 'A'
};

void Networking::cleanString(std::string& in)
{
	in.erase(remove(in.begin(), in.end(), PROTOCOL::FILLER), in.end());
}

void Networking::WorkOnRequest(char* recBuf)
{
	char host[8];

	host[0] = 127;
	host[1] = '0';
	host[2] = '0';
	host[3] = 1;
	host[4] = PROTOCOL::FILLER;
	host[5] = PROTOCOL::FILLER;
	host[6] = PROTOCOL::END;
	host[7] = 0;
	std::string response(host);
	
	if (recBuf[4] == PROTOCOL::LOGIN && clientInfo.loggedIn == false)
	{
		std::string input(recBuf);
		std::string username = input.substr(7, 12);
		cleanString(username);
		std::string password = input.substr(7 + 12, 12);
		cleanString(password);
		ClientManager cm(&clientInfo);
		response[4] = PROTOCOL::LOGIN;
		response[5] = PROTOCOL::REQUESTRESPONSE;

		//if login was succesfull
		if (cm.Login(username, password))
		{
			response.append("1");
			NotifyAllUsers((char)PROTOCOL::PEOPLE,"");

		}
		else {
			response.append("0");
		}
	}
	else if (!clientInfo.loggedIn)
	{
		response[4] = PROTOCOL::REQUESTRESPONSE;
		response.append("You need to be logged in for further actions.");
	}
	else if(recBuf[4] == PROTOCOL::PEOPLE)
	{
		//further actions here
		response[4] = PROTOCOL::PEOPLE;
		std::string users = GetAllOnlineUsers();
		response.append(users);
	}
	else if (recBuf[4] == PROTOCOL::MESSAGE)
	{
		if (recBuf[5] == PROTOCOL::MESSAGEFOREVERYONE)
		{

		}
		else if (recBuf[5] == PROTOCOL::FILEREQUEST)
		{
			std::string input(recBuf);
			std::string username = input.substr(7, 12);
			cleanString(username);
			std::string receiver = input.substr(7 + 12, 12);
			cleanString(receiver);
			if (input.size() <= 31)
			{
				if (IsUserLoggedIn(receiver))
				{
					SendFileAcceptRequest(username, receiver, recBuf);
					return;
				}
			}
			else if(input.size() == 32)
			{
				if (IsUserLoggedIn(receiver))
				{
					SendFileRequestResponse(username, receiver, recBuf);
					return;
				}
			}
		}
		else if (recBuf[5] == PROTOCOL::ATTACHEMENT)
		{
			std::string input(recBuf);
			std::string username = input.substr(7, 12); //19 + 12
			cleanString(username);
			std::string receiver = input.substr(7 + 12, 12);
			cleanString(receiver);
			std::size_t found = input.find((char)PROTOCOL::FILLER, 31);
			if (found == std::string::npos)
			{
				return;
			}
			std::string filename = input.substr(31, found-31);

			SendFile(username, receiver, filename, recBuf);
			

		}
		else
		{
			std::string input(recBuf);
			std::string username = input.substr(7, 12);
			cleanString(username);
			std::string receiver = input.substr(7+12, 12);
			cleanString(receiver);
			if (IsUserLoggedIn(receiver))
			{
				SendMessageToUser(receiver, recBuf);
				return;
			}
			
		}
	}
	else if (recBuf[4] == PROTOCOL::MESSAGEFOREVERYONE)
	{
		NotifyAllUsers((char)PROTOCOL::MESSAGE, "Message for everyone from the system.");
	}
	else if (recBuf[4] == PROTOCOL::FILEREQUEST)
	{
		
	}


	// send( ) may not be able to send the complete data in one go.
	// So try sending the data in multiple requests
	int nCntSend = 0;
	std::string result = splitResponse(response);
	const char* pBuffer = result.c_str();
	int bytesSent = 0;
	size_t messageSize = 256;
	const int messageTotalSize = response.size();

	while (bytesSent < result.size())
	{
		nCntSend = send(clientInfo.clientSocket, pBuffer, result.size(), 0);
			
		if (nCntSend == -1)
		{
			std::cout << "Error sending the data to " << inet_ntoa(clientInfo.clientAddr.sin_addr) << std::endl;

			clientInfo.loggedIn = false;
			NotifyAllUsers((char)PROTOCOL::PEOPLE, "");

			break;
		}
		std::cout << "Sending " << pBuffer << std::endl;
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
	std::string placeHolder = response.substr(0, 7);
	response.erase(0, 7);
	if (response.size() <= 249)
	{
		placeHolder[6] = PROTOCOL::END;
	}
	else
	{
		placeHolder[6] = PROTOCOL::START;
	}

	result.append(placeHolder);

	std::string temp = response.substr(0, 249);
	response.erase(0, 249);
	
	result.append(temp);

	placeHolder[6] = PROTOCOL::CONTINUATION;

	while (response.size() != 0)
	{
		temp = response.substr(0, 249);
		response.erase(0, 249);

		if (response.size() == 0)
		{
			placeHolder[6] = PROTOCOL::END;
		}

		result.append(placeHolder);
		result.append(temp);
	}

	return result;
}

std::string Networking::GetAllOnlineUsers()
{
	std::string result;
	if (threadList->size())
	{
		EnterCriticalSection(critical_section);
		for (auto thread : *threadList)
		{
			if (thread->clientInfo.loggedIn)
			{
				result.append(thread->clientInfo.username + '\n');
			}
		}
		LeaveCriticalSection(critical_section);
	}

	return result;
}

void Networking::NotifyAllUsers(char protocol,const std::string &notify = "")
{
	if (threadList->size())
	{
		EnterCriticalSection(critical_section);
		for (auto thread : *threadList)
		{
			if (thread->clientInfo.loggedIn)
			{
				char host[8];

				host[0] = 127;
				host[1] = '0';
				host[2] = '0';
				host[3] = 1;
				host[4] = PROTOCOL::MESSAGEFOREVERYONE;
				host[5] = PROTOCOL::MESSAGE;
				host[6] = PROTOCOL::END;
				host[7] = 0;
				std::string misc;
				std::string response;

				switch (protocol)
				{
					case (char)PROTOCOL::PEOPLE:
						host[5] = PROTOCOL::PEOPLE;
						LeaveCriticalSection(critical_section);
						misc = GetAllOnlineUsers();
						EnterCriticalSection(critical_section);
						response.append(host);
						response.append(misc);
						break;
					default:
						response.append(host);
						response.append(notify);
						break;
				}
				
				
				int nCntSend = 0;
				std::string result = splitResponse(response);
				const char* pBuffer = result.c_str();
				int bytesSent = 0;
				size_t messageSize = 256;
				const int messageTotalSize = response.size();

				while (bytesSent < result.size())
				{
					nCntSend = send(thread->clientInfo.clientSocket, pBuffer, result.size(), 0);

					if (nCntSend == -1)
					{
						std::cout << "Error sending the data to " << inet_ntoa(thread->clientInfo.clientAddr.sin_addr) << std::endl;
						thread->clientInfo.loggedIn = false;
						LeaveCriticalSection(critical_section);
						NotifyAllUsers((char)PROTOCOL::PEOPLE, "");
						break;
					}
					//if (nCntSend == iResult)
					//	break;

					pBuffer += nCntSend;
					bytesSent += nCntSend;
					//iResult -= nCntSend;
				}
			}
		}
		LeaveCriticalSection(critical_section);
	}

}


bool Networking::IsUserLoggedIn(std::string& user)
{
	if (threadList->size())
	{
		EnterCriticalSection(critical_section);
		for (auto thread : *threadList)
		{
			if (thread->clientInfo.loggedIn && thread->clientInfo.username == user)
			{
				LeaveCriticalSection(critical_section);
				return true;
			}
		}
		LeaveCriticalSection(critical_section);
	}

	return false;
}

void Networking::SendMessageToUser(std::string& user, char* recBuf)
{
	if (threadList->size())
	{
		EnterCriticalSection(critical_section);
		for (auto thread : *threadList)
		{
			if (thread->clientInfo.loggedIn && thread->clientInfo.username == user)
			{
				int nCntSend = 0;
				std::string result(recBuf);
				result = splitResponse(result);
				const char* pBuffer = result.c_str();
				int bytesSent = 0;
				size_t messageSize = 256;

				while (bytesSent < result.size())
				{
					nCntSend = send(thread->clientInfo.clientSocket, pBuffer, result.size(), 0);

					if (nCntSend == -1)
					{
						std::cout << "Error sending the data to " << inet_ntoa(thread->clientInfo.clientAddr.sin_addr) << std::endl;
						thread->clientInfo.loggedIn = false;
						LeaveCriticalSection(critical_section);
						NotifyAllUsers((char)PROTOCOL::PEOPLE, "");
						EnterCriticalSection(critical_section);
						break;
					}
					//if (nCntSend == iResult)
					//	break;
					std::cout << "Sending " << pBuffer << std::endl;
					pBuffer += nCntSend;
					bytesSent += nCntSend;
					//iResult -= nCntSend;
				}
				LeaveCriticalSection(critical_section);
				return;
			}
		}
		LeaveCriticalSection(critical_section);
	}

	return;
}

void Networking::SendFileAcceptRequest(std::string& user, std::string& receiver, char * recBuf)
{

	recBuf[0] = 127;
	recBuf[1] = '0';
	recBuf[2] = '0';
	recBuf[3] = 1;
	recBuf[4] = PROTOCOL::MESSAGE;
	recBuf[5] = PROTOCOL::FILEREQUEST;
	recBuf[6] = PROTOCOL::END;
	//recBuf[7] = 0;

	if (threadList->size())
	{
		EnterCriticalSection(critical_section);
		for (auto thread : *threadList)
		{
			if (thread->clientInfo.loggedIn && thread->clientInfo.username == receiver)
			{
				int nCntSend = 0;

				std::string result(recBuf);
				result = splitResponse(result);
				const char* pBuffer = result.c_str();
				int bytesSent = 0;
				size_t messageSize = 256;

				while (bytesSent < result.size())
				{
					nCntSend = send(thread->clientInfo.clientSocket, pBuffer, result.size(), 0);

					if (nCntSend == -1)
					{
						std::cout << "Error sending the data to " << inet_ntoa(thread->clientInfo.clientAddr.sin_addr) << std::endl;
						thread->clientInfo.loggedIn = false;
						LeaveCriticalSection(critical_section);
						NotifyAllUsers((char)PROTOCOL::PEOPLE, "");
						EnterCriticalSection(critical_section);
						break;
					}
					//if (nCntSend == iResult)
					//	break;
					std::cout << "Sending " << pBuffer << std::endl;
					pBuffer += nCntSend;
					bytesSent += nCntSend;
					//iResult -= nCntSend;
				}
				LeaveCriticalSection(critical_section);
				return;
			}
		}
		LeaveCriticalSection(critical_section);
	}
}

void Networking::SendFileRequestResponse(std::string& user, std::string& receiver, char* recBuf)
{

		recBuf[0] = 127;
		recBuf[1] = '0';
		recBuf[2] = '0';
		recBuf[3] = 1;
		recBuf[4] = PROTOCOL::MESSAGE;
		recBuf[5] = PROTOCOL::FILEREQUEST;
		recBuf[6] = PROTOCOL::END;
		//recBuf[7] = 0;

		if (threadList->size())
		{
			EnterCriticalSection(critical_section);
			for (auto thread : *threadList)
			{
				if (thread->clientInfo.loggedIn && thread->clientInfo.username == receiver)
				{
					int nCntSend = 0;

					std::string result(recBuf);
					result = splitResponse(result);
					const char* pBuffer = result.c_str();
					int bytesSent = 0;
					size_t messageSize = 256;

					while (bytesSent < result.size())
					{
						nCntSend = send(thread->clientInfo.clientSocket, pBuffer, result.size(), 0);

						if (nCntSend == -1)
						{
							std::cout << "Error sending the data to " << inet_ntoa(thread->clientInfo.clientAddr.sin_addr) << std::endl;
							thread->clientInfo.loggedIn = false;
							LeaveCriticalSection(critical_section);
							NotifyAllUsers((char)PROTOCOL::PEOPLE, "");
							EnterCriticalSection(critical_section);
							break;
						}
						//if (nCntSend == iResult)
						//	break;
						std::cout << "Sending " << pBuffer << std::endl;
						pBuffer += nCntSend;
						bytesSent += nCntSend;
						//iResult -= nCntSend;
					}
					LeaveCriticalSection(critical_section);
					return;
				}
			}
			LeaveCriticalSection(critical_section);
		}
}

void Networking::SendFile(std::string& user, std::string& receiver, std::string& filename, char* recBuf)
{
	recBuf[0] = 127;
	recBuf[1] = '0';
	recBuf[2] = '0';
	recBuf[3] = 1;
	recBuf[4] = PROTOCOL::MESSAGE;
	recBuf[5] = PROTOCOL::ATTACHEMENT;
	recBuf[6] = PROTOCOL::END;
	//recBuf[7] = 0;

	if (threadList->size())
	{
		EnterCriticalSection(critical_section);
		for (auto thread : *threadList)
		{
			if (thread->clientInfo.loggedIn && thread->clientInfo.username == receiver)
			{
				int nCntSend = 0;

				std::string result(recBuf);
				result = splitResponse(result);
				const char* pBuffer = result.c_str();
				int bytesSent = 0;
				size_t messageSize = 256;

				while (bytesSent < result.size())
				{
					nCntSend = send(thread->clientInfo.clientSocket, pBuffer, result.size(), 0);

					if (nCntSend == -1)
					{
						std::cout << "Error sending the data to " << inet_ntoa(thread->clientInfo.clientAddr.sin_addr) << std::endl;
						thread->clientInfo.loggedIn = false;
						LeaveCriticalSection(critical_section);
						NotifyAllUsers((char)PROTOCOL::PEOPLE, "");
						EnterCriticalSection(critical_section);
						break;
					}
					//if (nCntSend == iResult)
					//	break;
					std::cout << "Sending " << pBuffer << std::endl;
					pBuffer += nCntSend;
					bytesSent += nCntSend;
					//iResult -= nCntSend;
				}
				LeaveCriticalSection(critical_section);
				return;
			}
		}
		LeaveCriticalSection(critical_section);
	}
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
	std::string data;
	bool first = true;
	while (1)
	{
		iResult = recv(this->clientInfo.clientSocket, recBuf, sizeof(recBuf), 0);
		if (iResult > 0)
		{
			recBuf[iResult] = '\0';

			std::string temp(recBuf);


			if (recBuf[6] != PROTOCOL::END)
			{
				if (!first)
				{
					temp = temp.erase(0, HEADER_LENGTH);
				}
				first = false;

				data.append(temp);
				continue;
			}
			else
			{
				if (!first)
				{
					temp = temp.erase(0, HEADER_LENGTH);
				}
				first = false;

				data.append(temp);
				first = true;
			}




			std::cout << "Received " << recBuf << " from " << inet_ntoa(clientInfo.clientAddr.sin_addr) << std::endl;

			
			// Convert the string to upper case
			if (recBuf[4] == PROTOCOL::QUIT)
			{
				closesocket(clientInfo.clientSocket);
				return;
			}
			else
			{
				char * dataToWork = new char[data.length()];
				dataToWork = (char*)data.c_str();
				WorkOnRequest(dataToWork);
				data = std::string("");
			}
			
		}
		else
		{
			std::cout << "Error reading the data from " << inet_ntoa(clientInfo.clientAddr.sin_addr) << std::endl;
			clientInfo.loggedIn = false;
			NotifyAllUsers((char)PROTOCOL::PEOPLE,"");
			break;
		}

	}
}
