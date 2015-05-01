/*Kyle Groom
	CS317
	Code Started 10/20/2014
	Code Worked On: 10/27/2014
					10/28/2014
					12/08/2014

		Ended:		12/08/2014
*/

#define AddressBook_Client

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif

#include <Windows.h>
#include <WinSock2.h>
#include <WS2tcpip.h>
#include <stdlib.h>
#include <stdio.h>
#include <sstream>
#include <string>
#include <iostream>
#include <vector>
#include <limits>
using namespace std;

// Need to link with Ws2_32.lib, Mswsock.lib, and Advapi32.lib
#pragma comment (lib, "Ws2_32.lib")
#pragma comment (lib, "Mswsock.lib")
#pragma comment (lib, "AdvApi32.lib")

//Q & R are for requesting the full name from an email address
//q & r are for requesting the email address of a full name
enum MessageTypes{ Query = 'Q', Query2 = 'q', Response = 'R', Response2 = 'r' };

typedef char TinyInt;

#define MaxStringSize 255

struct Message {
	MessageTypes MessageType;
	TinyInt AStringLength;
	string AString;

	Message()
	{
		MessageType = Query;
		AStringLength = MaxStringSize;
	}

	Message(MessageTypes MT, TinyInt ASL, string AS)
	{
		if (AS.length() > MaxStringSize)
			AS.resize(MaxStringSize);
		if (ASL != (TinyInt)AS.length())
			ASL = (TinyInt)AS.length();

		AString = AS;
		AStringLength = ASL;
		MessageType = MT;
	}
	
	Message(MessageTypes MT, string AS)
	{
		if (AS.length() > MaxStringSize)
			AS.resize(MaxStringSize);

		AString = AS;
		AStringLength = AS.length();
		MessageType = MT;
	}
};

struct ComputerLab
{
	string BuildingCode;
	int RoomNumber;
	bool HasTeacherStation;
	int ComputerCount;

	ComputerLab(string Code, int Room, bool TSta, int comC)
	{
		BuildingCode = Code;
		RoomNumber = Room;
		HasTeacherStation = TSta;
		ComputerCount = comC;
	}

	string GetComputerName(int Comp)
	{
		stringstream convert;
		string result;
		if (HasTeacherStation)
		{
			if (Comp == 0)
				convert << BuildingCode << RoomNumber << "-" << "T";
			else if (Comp < 10)
				convert << BuildingCode << RoomNumber << "-0" << Comp;
			else
				convert << BuildingCode << RoomNumber << "-" << Comp;
		}
		else
		{
			if (Comp < 9)
				convert << BuildingCode << RoomNumber << "-0" << (Comp + 1);
			else
				convert << BuildingCode << RoomNumber << "-" << (Comp + 1);
		}
		convert >> result;
		return result;
	}

	int GetComCount() { return ComputerCount; }
	int GetRmNumber() { return RoomNumber; }
	string GetBldCode() { return BuildingCode; }

};

string MSGTypeToString(MessageTypes T)
{
	switch (T)
	{
	case Query:
		return "Q";
		break;
	case Query2:
		return "q";
		break;
	case Response:
		return "R";
		break;
	case Response2:
		return "R";
		break;
	}
}

bool StringToMSGType(char c, MessageTypes &O)
{
	switch (c)
	{
	case 'Q':
		O = Query;
		return true;
		break;
	case 'q':
		O = Query2;
		return true;
		break;
	case 'R':
		O = Response;
		return true;
		break;
	case 'r':
		O = Response2;
		return true;
		break;
	default:
		printf("Invalid Client Message Type");
		return false;
		break;
	}
}

int send(SOCKET s, Message MSG, int flags)
{
	string TotalMSG = MSGTypeToString(MSG.MessageType);
	TotalMSG = TotalMSG + MSG.AStringLength;
	TotalMSG = TotalMSG + MSG.AString;
	return send(s, TotalMSG.c_str(), (int)strlen(TotalMSG.c_str()), 0);
}

int send(SOCKET s, Message MSG, string Admin_CMD, int flags)
{
	string TotalMSG = MSGTypeToString(MSG.MessageType);
	TotalMSG = TotalMSG + (char)strlen(Admin_CMD.c_str());
	TotalMSG = TotalMSG + Admin_CMD.c_str();
	TotalMSG = TotalMSG + MSG.AStringLength;
	TotalMSG = TotalMSG + MSG.AString;
	return send(s, TotalMSG.c_str(), (int)strlen(TotalMSG.c_str()), 0);
}


int recv(SOCKET s, Message &RCV, int flags)
{
	char str[MaxStringSize];
	int val = recv(s, str, MaxStringSize, 0);
	string STR = str;
	StringToMSGType(str[0], RCV.MessageType);
	RCV.AStringLength = STR[1];
	RCV.AString = STR.substr(2, RCV.AStringLength);
	return val;
}

string ToUpper(string In)
{
	string Out;
	for (int i = 0; i < (int)In.size(); i++)
		Out = Out + (char)(toupper(In[i]));
	return Out;
}

//Default Buffer Length
#define DEFAULT_BUFLEN 256

//Default Port - 1977 Address Book
#define DEFAULT_PORT "1977"

vector<string> MakeERC()
{
	vector<ComputerLab> BLDG;
	vector<string> ERC;

	BLDG.push_back(ComputerLab("ERC", 107, false, 14));
	BLDG.push_back(ComputerLab("ERC", 108, true, 25));
	BLDG.push_back(ComputerLab("ERC", 109, true, 21));
	BLDG.push_back(ComputerLab("ERC", 117, true, 15));

	ERC.push_back("localhost");
	for (int i = 0; i < (int)BLDG.size(); i++)
	{
		for (int j = 0; j < BLDG[i].GetComCount(); j++)
		{
			ERC.push_back(BLDG[i].GetComputerName(j) + ".dwc.edu");
		}
	}

	ERC.push_back("780-TechBenchPlus.dwc.edu");
	ERC.push_back("790-TechBench.dwc.edu");
	ERC.push_back("SHMOE-LEFT.dwc.edu");
	ERC.push_back("SHMOE-RIGHT.dwc.edu");

	return ERC;
}


int __cdecl main(int argc, char **argv)
{
	string ServerName;

	WSADATA wsaData;
	SOCKET ConnectSocket = INVALID_SOCKET;
	struct addrinfo *result = NULL, *ptr = NULL, hints;
	int iResult;
	int recvbuflen = DEFAULT_BUFLEN;

	// Initialize Winsock
	iResult = WSAStartup(MAKEWORD(2, 2), &wsaData);
	if (iResult != 0) {
		printf("WSAStartup failed with error: %d\n", iResult);
		system("pause");
		return 1;
	}

	ZeroMemory(&hints, sizeof(hints));
	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_protocol = IPPROTO_TCP;

	vector <addrinfo*> results;
	//Validate the parameters
	if (argc == 1)
	{
		vector<string> ERC = MakeERC();
		addrinfo* temp = NULL;
		for (int i = 0; i < (int)ERC.size(); i++)
		{
			if (getaddrinfo(ERC[i].c_str(), DEFAULT_PORT, &hints, &temp) == 0)
			{
				iResult = 0;
				temp->ai_canonname = (char*)ERC[i].c_str();
				results.push_back(temp);
			}
		}
	}
	else if (argc == 2)
	{
		ServerName = argv[1];
		iResult = getaddrinfo(argv[1], DEFAULT_PORT, &hints, &result);
	}
	else
	{
		printf("usage: %s server-name\n", argv[0]);
		system("pause");
		return 1;
	}

	// Resolve the server address and port
	if (iResult != 0) {
		printf("getaddrinfo failed with error: %d\n", iResult);
		WSACleanup();
		system("pause");
		return 1;
	}

	for (int i = 0; i < (int)results.size(); i++)
	{
		if (argc == 2 || results[0] || ptr->ai_next == NULL)
			ptr = results[i];
		else
			ptr = ptr->ai_next;

		ConnectSocket = socket(ptr->ai_family, ptr->ai_socktype, ptr->ai_protocol);
		if (ConnectSocket == INVALID_SOCKET) {
			printf("socket failed with error: %ld\n", WSAGetLastError());
			WSACleanup();
			system("pause");
			return 1;
		}

		// Connect to server.
		iResult = connect(ConnectSocket, ptr->ai_addr, (int)ptr->ai_addrlen);
		if (iResult == SOCKET_ERROR) {
			closesocket(ConnectSocket);
			ConnectSocket = INVALID_SOCKET;
			continue;
		}
		if (ServerName == "")
			ServerName = ptr->ai_canonname;

		break;
	}

	//free the info obtained from getAddrInfo from results, except for what we are still using
	for (int i = (int)results.size() - 1; i >= 0; i--)
	{
		if (ServerName == ptr->ai_canonname)
			continue;
		results[i]->ai_canonname = NULL;
		freeaddrinfo(results[i]);
		results.pop_back();
	}

	if (ConnectSocket == INVALID_SOCKET) {
		printf("Unable to connect to server!\n");
		WSACleanup();
		return 1;
	}

	bool quit = false;
	bool cmd = false;
	string Prev_Added = "";
	bool PrevA_Cmpl = true;

	bool PrevE_Cmpl = true;
	bool PrevEN = false;

	do{
		Message SND;
	
		if (!PrevA_Cmpl)
		{
			string Email;

			do
			{
				printf("\t%s To Be Added (with @ to denote the server): ", Prev_Added.c_str());
				getline(cin, Email);
				printf("\n");
			} while (Email.find_first_of('@') == string::npos);

			SND.AStringLength = strlen(Email.c_str());
			SND.AString = Email;

			PrevA_Cmpl = true;

			iResult = send(ConnectSocket, SND, "_ADD_E", 0);
			printf("Message Sent To Server...\n");
			if (iResult == SOCKET_ERROR) {
				printf("send failed with error: %d\n", WSAGetLastError());
				closesocket(ConnectSocket);
				WSACleanup();
				return 1;
			}
			break;
		}

		if (!PrevE_Cmpl)
		{
			if (PrevEN)
			{
				string NewN;
				do
				{
					printf("Enter the NEW Full Name Of the User (with a space inbetween the names): ");
					getline(cin, NewN);
					printf("\n");
				} while (NewN.find_first_of(' ') == string::npos);

				SND.AString = NewN;
				SND.AStringLength = strlen(NewN.c_str());

				PrevE_Cmpl = true;

				iResult = send(ConnectSocket, SND, "Edit_N", 0);
				printf("Message Sent To Server...\n");
				if (iResult == SOCKET_ERROR) {
					printf("send failed with error: %d\n", WSAGetLastError());
					closesocket(ConnectSocket);
					WSACleanup();
					return 1;
				}
			}
			else
			{
				string NewE;
				do
				{
					printf("Enter the NEW Email Of the User (with a space inbetween the names): ");
					getline(cin, NewE);
					printf("\n");
				} while (NewE.find_first_of(' ') == string::npos);

				SND.AString = NewE;
				SND.AStringLength = strlen(NewE.c_str());

				PrevE_Cmpl = true;

				iResult = send(ConnectSocket, SND, "Edit_E", 0);
				printf("\n");
				printf("Message Sent To Server...\n");
				if (iResult == SOCKET_ERROR) {
					printf("send failed with error: %d\n", WSAGetLastError());
					closesocket(ConnectSocket);
					WSACleanup();
					return 1;
				}
			}
			break;
		}

		do
		{

			printf("Send \'Email\' or Full \'Name\'? ");
			string val;
			getline(cin, val);
			val = ToUpper(val);

			//user wants to get the full name
			if (val == "EMAIL")
			{
				cmd = true;
				SND.MessageType = Query;
				printf("Input the Email Address You want to get the Full Name Of:\n");
				getline(cin, SND.AString);
				SND.AStringLength = strlen(SND.AString.c_str());

				//Send A Message
				iResult = send(ConnectSocket, SND, 0);
				printf("\n");
				printf("Message Sent To Server...\n");
				if (iResult == SOCKET_ERROR) {
					printf("send failed with error: %d\n", WSAGetLastError());
					closesocket(ConnectSocket);
					WSACleanup();
					return 1;
				}
				
			}
			//user wants to get the email
			else if (val == "FULL NAME")
			{
				cmd = true;
				SND.MessageType = Query2;
				printf("Input the Full Name You want to get the Email Of:\n");
				getline(cin, SND.AString);
				printf("\n");
				SND.AStringLength = strlen(SND.AString.c_str());

				//Send A Message
				iResult = send(ConnectSocket, SND, 0);
				printf("Message Sent To Server...\n");
				if (iResult == SOCKET_ERROR) {
					printf("send failed with error: %d\n", WSAGetLastError());
					closesocket(ConnectSocket);
					WSACleanup();
					return 1;
				}
			}
			//Breaks the Send into two parts, sending the ADD Name And Add Email separately to help prevent overflow
			else if (val == "ADD")
			{
				cmd = true;
				string Name;
				
				do{
					printf("Full Name Of the User to Be Added (with a space inbetween the names): ");
					getline(cin, Name);
					printf("\n");
				}while (Name.find_first_of(' ') == string::npos);

				SND.AStringLength = strlen(Name.c_str());
				SND.AString = Name;

				Prev_Added = Name;
				PrevA_Cmpl = false;

				iResult = send(ConnectSocket, SND, "_ADD_N", 0);
				printf("Message Sent To Server...\n");
				if (iResult == SOCKET_ERROR) {
					printf("send failed with error: %d\n", WSAGetLastError());
					closesocket(ConnectSocket);
					WSACleanup();
					return 1;
				}
				
			}
			else if (val == "REMOVE")
			{
				cmd = true;
				string Key;
				printf("Full Name or Email Address Of User to Be Removed: ");
				getline(cin, Key);

				SND.AString = Key;
				SND.AStringLength = strlen(Key.c_str());
				
				iResult = send(ConnectSocket, SND, "REMOVE", 0);
				printf("Message Sent To Server...\n");
				if (iResult == SOCKET_ERROR) {
					printf("send failed with error: %d\n", WSAGetLastError());
					closesocket(ConnectSocket);
					WSACleanup();
					return 1;
				}
			}
			else if (val == "EDIT")
			{
				cmd = true;
				bool good = false;
				string Ans;
				do
				{
					printf("Edit User \'Name\' or \'Email\'? ");
					getline(cin, Ans);

					Ans = ToUpper(Ans);

					if (Ans == "NAME" || Ans == "EMAIL")
						good = true;

				} while (!good);

			
				string Key;
				do
				{
					printf("Enter the CURRENT Full Name/Email Of the User (with a space inbetween the names, or @ to denote adress location):\n");
					getline(cin, Key);
					printf("\n");
				} while (Key.find_first_of(' ') == string::npos && Key.find_first_of('@') == string::npos);

				SND.AString = Key;
				SND.AStringLength = strlen(Key.c_str());

				if (Ans == "NAME")
					PrevEN = true;
				else //ANS == "EMAIL
					PrevEN = false;

				PrevE_Cmpl = false;

				iResult = send(ConnectSocket, SND, "_EDIT_", 0);
				printf("Message Sent To Server...\n");
				if (iResult == SOCKET_ERROR) {
					printf("send failed with error: %d\n", WSAGetLastError());
					closesocket(ConnectSocket);
					WSACleanup();
					return 1;
				}
									
			}
			else if (val == "_CLOSE")
			{
				cmd = true;
				SND.AString = "_CLOSE";
				SND.AStringLength = 6;
				//Send A Message
				iResult = send(ConnectSocket, SND, 0);
				printf("Message Sent To Server...\n");
				if (iResult == SOCKET_ERROR) {
					printf("send failed with error: %d\n", WSAGetLastError());
					closesocket(ConnectSocket);
					WSACleanup();
					return 1;
				}
			}
			else if (val == "ADMIN COMMANDS" || val == "HELP")
			{
				printf("1. \"ADD\" - Adds a User's Name then Email to the Address Book as two separate messages.\n");
				printf("2. \"REMOVE\" - Remove User - Completely Removes a user's record from the Address Book.\n");
				printf("3. \"EDIT\" - Edit User - Edits the existing values of the Email or the Name of a user in the Address Book.\n");
				printf("4. \"_CLOSE\" - Close Server - Ends All Server Operations.\n");
			}
			else
			{
				printf("INVALID SELECTION. Please Enter in a proper option.\n");
			}

		} while (!cmd);

		//Wait for a response

		bool ask = true;
		do{
			Message RCV;
			RCV.MessageType = Response;

			iResult = recv(ConnectSocket, RCV, 0);
			if (iResult > 0)
			{
				if (RCV.AString == "Server Is Shutting Down")
					printf("%s by Admin's Request.\n", RCV.AString.c_str());
				else if (RCV.AString == "User Email Edited")
					printf("%s by Admin's Request.\n", RCV.AString.c_str());
				else if (RCV.AString == "User Name Edited.\n")
					printf("%s by Admin's Request.\n", RCV.AString.c_str());
				else if (RCV.AString == "User Found")
				{
					printf("%s.\n", RCV.AString.c_str());
					ask = false;
				}
				else if (RCV.AString == "User Not Found")
				{
					printf("%s.\n", RCV.AString.c_str());
					PrevE_Cmpl = true;
				}
				else if (RCV.AString == "User's Name Added")
				{
					printf("%s, Please Add the Email Address of\n", RCV.AString.c_str());
					ask = false;
				}
				else if (RCV.AString == "User's Email Added")
					printf("User Fully Added by Admin Request.\n");
				else if (RCV.AString == "User Removed")
					printf("%s by Admin's Request", RCV.AString.c_str());
				else if (RCV.AString == "Edit Ready")
				{
					printf("User Ready To Be Edited.\n", RCV.AString.c_str());
					ask = false;
				}
				else if (RCV.AString == "User Exists")
				{
					printf("%s, so no need to re-add them.\n", RCV.AString.c_str());
					PrevA_Cmpl = true;
				}
				else if (RCV.AString == "Edit Pending")
				{
					printf("There is another edit (already) pending completion, please try your edit later.\n");
					PrevE_Cmpl = true;
				}
				else if (RCV.MessageType == Response)
					printf("<%s> is the Requested Full Name\nof the Email Address <%s>\n", RCV.AString.c_str(), SND.AString.c_str());
				else if (RCV.MessageType == Response2)
					printf("<%s> is the Requested Email Address\nof the Full Name <%s>\n", SND.AString.c_str(), RCV.AString.c_str());
				break;
			}
			else if (iResult == 0)
			{
				printf("Connection closed\n");
				return 0;
			}
			else
				printf("recv failed with error: %d\n", WSAGetLastError());

		} while (iResult > 0);
		
		while (true && ask)
		{
			char ANS;
			printf("Send Another Message? (Y/N): ");
			cin >> ANS;
			if (ANS == 'Y' || ANS == 'y')
			{
				quit = false;
				break;
			}
			else if (ANS == 'N' || ANS == 'n')
			{
				quit = true;

				// shutdown the connection since no more data will be sent
				iResult = shutdown(ConnectSocket, SD_SEND);
				if (iResult == SOCKET_ERROR) {
					printf("shutdown failed with error: %d\n", WSAGetLastError());
					closesocket(ConnectSocket);
					WSACleanup();
					return 1;
				}

				break;
			}
			else
			{
				printf("invalid Input, please enter \'Y\' or \'N\'\n");
				cin.clear();
				//Done to Avoid use of Max macro
				std::cin.ignore((numeric_limits<streamsize>::max)(), '\n');
			}
		}

	} while (!quit);
	
	system("pause");
	return 0;
}