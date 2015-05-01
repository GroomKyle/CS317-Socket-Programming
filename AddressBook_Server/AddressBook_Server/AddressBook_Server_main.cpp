/*Kyle Groom
	CS317
	Code Started 10/20/2014
	Code Worked On: 10/27/2014
					10/28/2014
					12/08/2014

		Ended:		12/08/2014
*/

#define AddressBook_Server
#undef UNICODE
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif

#include <windows.h>
#include <winsock2.h>
#include <ws2tcpip.h>
#include <stdlib.h>
#include <stdio.h>
#include <vector>
#include <string>
#include <fstream> //for ifstream
#include <cctype> //for to upper
using namespace std;

// Need to link with Ws2_32.lib
#pragma comment (lib, "Ws2_32.lib")

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
		MessageType = Response;
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

string ToUpper(string In)
{
	string Out;
	for (int i = 0; i < (int)In.size(); i++)
		Out = Out + (char)(toupper(In[i]));
	return Out;
}

vector<string> EMAILS;
vector<string> FNAMES;
void MakeDB(ifstream &File)
{
	string Value;
	while (getline(File, Value))
	{
		size_t seperator = Value.find_first_of(",");
		EMAILS.push_back(Value.substr(0, seperator));
		FNAMES.push_back(Value.substr(seperator + 1, Value.size()));
	}
}

bool FindUser(string AString)
{
	string tmp = ToUpper(AString);
	for (int i = 0; i < (int)FNAMES.size(); i++)
		if (tmp == ToUpper(FNAMES[i]))
			return true;
	return false;
}

bool FindUser(string AString, int& indx)
{
	string tmp = ToUpper(AString);
	for ( indx = 0; indx < (int)FNAMES.size(); indx++)
		if (tmp == ToUpper(FNAMES[indx]))
			return true;
	return false;
}

bool FindEmail(string AString)
{
	string tmp = ToUpper(AString);
	for (int i = 0; i < (int)EMAILS.size(); i++)
		if (tmp == ToUpper(EMAILS[i]))
			return true;
	return false;
}

bool FindEmail(string AString, int& indx)
{
	string tmp = ToUpper(AString);
	for (indx = 0; indx < (int)EMAILS.size(); indx++)
		if (tmp == ToUpper(EMAILS[indx]))
			return true;
	return false;
}

bool IsAdminCommand(string AString)
{
	if (AString == "_ADD_E" || AString == "_ADD_N" || AString == "REMOVE" ||
		AString == "EDIT_E" || AString == "EDIT_N" || AString == "_EDIT_")
		return true;
	return false;
}

int recv(SOCKET s, Message &RCV, Message &RCV2, int flags)
{
	char str[MaxStringSize + 9];
	int val = recv(s, str, MaxStringSize + 9, 0);
	string STR = str;
	StringToMSGType(str[0], RCV.MessageType);
	RCV.AStringLength = STR[1];
	RCV.AString = STR.substr(2, RCV.AStringLength);

	if (IsAdminCommand(RCV.AString))
	{
		string STR2 = STR.substr(RCV.AStringLength + 2);
		StringToMSGType(str[0], RCV2.MessageType);
		RCV2.AStringLength = STR[RCV.AStringLength + 2];
		RCV2.AString = STR.substr(RCV.AStringLength + 3, RCV2.AStringLength);
	}

	return val;
}

void GetClientMessage(Message In, Message In2, Message &Out, bool &end)
{
	//used when expecting email
	static bool Expt_E = false;

	//used when currently expecting a name
	static bool Expt_N = false;

	//Used for specifying with _Edit_ and making sure no other edits go through until after completion of current edit
	static bool Edt_CMPL = true;

	//The Index to which the operation is done
	static int  Edt_INDX = -1;

	//admin wants to close the server (probably for maintenence)
	if (In.AString == "_CLOSE")
	{
		Out.AString = "Server Is Shutting Down";
		Out.AStringLength = strlen(Out.AString.c_str());
		end = true;
	}
	//Admin wants to add a new user's email address
	else if (In.AString == "_ADD_E" && Expt_E)
	{
		EMAILS.push_back(In2.AString);
		Expt_E = false;
		printf("\tUser Email Added\n");
		Out.AString = "User's Email Added";
		Out.AStringLength = strlen(Out.AString.c_str());
	}
	//Admin wants to Add A NEW User's nameto a corresponding new email
	else if (In.AString == "_ADD_N")
	{
		printf("\nRequest to Add New User %s Recieved.\n", In2.AString.c_str());
		if (FindUser(In2.AString) || FindEmail(In2.AString))
		{
			printf("\tRequest Denied: Uder Already Exists.\n");
			Out.AString = "User Exists";
			Out.AStringLength = strlen(Out.AString.c_str());
		}
		else
		{
			FNAMES.push_back(In2.AString);
			Expt_E = true;
			printf("\tUser Name Added\n");
			Out.AString = "User's Name Added";
			Out.AStringLength = strlen(Out.AString.c_str());
		}
	}
	//Admin wants to remove a user
	else if (In.AString == "REMOVE")
	{
		printf("\nRemoval Request Recieved For %s\n", In2.AString.c_str());
		//removes not allowed to prevent skewing of indexes
		if (!Edt_CMPL)
		{
			printf("\tRemoval Request Denied: Edit Pending.\n");
			Out.AString = "Edit Pending";
			Out.AStringLength = strlen(Out.AString.c_str());
			return;
		}

		int indx=-1;
		if (FindUser(In2.AString, indx))
		{
			printf("\t Removal Approved.\n");
			FNAMES.erase(FNAMES.begin() + indx);
			EMAILS.erase(EMAILS.begin() + indx);
			Out.AString = "User Removed";
			Out.AStringLength = strlen(Out.AString.c_str());
		}
		else if (FindEmail(In2.AString, indx))
		{
			printf("\t Removal Approved.\n");
			FNAMES.erase(FNAMES.begin() + indx);
			EMAILS.erase(EMAILS.begin() + indx);
			Out.AString = "User Removed";
			Out.AStringLength = strlen(Out.AString.c_str());
		}
		else
		{
			printf("\t Removal Denied: User Not Found.\n");
			Out.AString = "User Not Found";
			Out.AStringLength = strlen(Out.AString.c_str());
		}

	}
	//Admin wants to edit the email of a user
	else if (In.AString == "EDIT_E" && Expt_E)
	{
		EMAILS[Edt_INDX] = In2.AString;

		Out.AString = "User Email Edited";
		Out.AStringLength = strlen(Out.AString.c_str());

		Expt_E = false;
		Edt_INDX = -1;
		Edt_CMPL = true;
	}
	//admon wants to edit the Name of a user
	else if (In.AString == "EDIT_N" && Expt_N)
	{
		Out.AString = "User Name Edited";
		Out.AStringLength = strlen(Out.AString.c_str());

		Expt_N = false;
		Edt_INDX = -1;
		Edt_CMPL = true;
	}
	//admin sending The key for editing a user's name or email
	else if (In.AString == "_EDIT_")
	{
		printf("\nEdit Request recieved for %s.\n", In2.AString.c_str());
		if (!Edt_CMPL)
		{
			printf("\tEdit Already Pending, Denying Edit Request.\n");
			Out.AString = "Edit Pending";
			Out.AStringLength = strlen(Out.AString.c_str());
			return;
		}

		if (FindUser(In2.AString, Edt_INDX))
		{
			printf("\t Edit Request Ready.\n");
			Edt_CMPL = false;
			Expt_E = true;
			Out.AString = "Edit Ready";
			Out.AStringLength = strlen(Out.AString.c_str());
		}
		else if (FindEmail(In2.AString, Edt_INDX))
		{
			printf("\t Edit Request Ready.\n");
			Edt_CMPL = false;
			Expt_N = true;
			Out.AString = "Edit Ready";
			Out.AStringLength = strlen(Out.AString.c_str());
		}
		else
		{
			printf("\t Edit Request Canceld: User Not Found.\n");
			Edt_INDX = -1;
			Out.AString = "User Not Found";
			Out.AStringLength = strlen(Out.AString.c_str());
		}
	}
	//client requested full name
	else if (In.MessageType==Query)
	{
		printf("\nRecieved the Name Request for %s\n", In.AString.c_str());
		int index;
		if (FindEmail(In.AString, index))
		{
			printf("\tResponding to request.\n");
			Out.AString = FNAMES[index];
			Out.AStringLength = strlen(Out.AString.c_str());
		}
		else
		{
			printf("No Entry found for request.\n");
			Out.AString = "User Not Found";
			Out.AStringLength = strlen(Out.AString.c_str());
		}
	}
	//Client Requested email
	else if (In.MessageType == Query2)
	{
		printf("\nRecieved the Email Request for %s\n", In.AString.c_str());
		int index;
		if (FindUser(In.AString, index))
		{
			printf("\tResponding to request.\n");
			Out.AString = EMAILS[index];
			Out.AStringLength = strlen(Out.AString.c_str());
		}
		else
		{
			printf("\tNo Entry found for request.\n");
			Out.AString = "User Not Found";
			Out.AStringLength = strlen(Out.AString.c_str());
		}

	}

}

//Default Buffer Length
#define DEFAULT_BUFLEN 256

//Default Port - 1977 Address Book
#define DEFAULT_PORT "1977"

int _cdecl main(void)
{
	//Comment This out to enter debug mode
	FreeConsole();
	
	//Opens reference file
	ifstream MyFile("Address Book File.csv");
	
	//Creates Vectors from MyFile
	MakeDB(MyFile);

	//Doesn't keep file open to save on time
	MyFile.close();

	WSADATA wsaData;
	int iResult;

	SOCKET ListenSocket = INVALID_SOCKET;
	SOCKET ClientSocket = INVALID_SOCKET;

	struct addrinfo *result = NULL;
	struct addrinfo hints;

	int iSendResult;
	int recvbuflen = DEFAULT_BUFLEN;

	// Initialize Winsock
	iResult = WSAStartup(MAKEWORD(2, 2), &wsaData);
	if (iResult != 0) {
		printf("WSAStartup failed with error: %d\n", iResult);
		system("pause");
		return 1;
	}

	ZeroMemory(&hints, sizeof(hints));
	hints.ai_family = AF_INET;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_protocol = IPPROTO_TCP;
	hints.ai_flags = AI_PASSIVE;

	// Resolve the server address and port
	iResult = getaddrinfo(NULL, DEFAULT_PORT, &hints, &result);
	if (iResult != 0) {
		printf("getaddrinfo failed with error: %d\n", iResult);
		WSACleanup();
		system("pause");
		return 1;
	}

	// Create a SOCKET for connecting to server
	ListenSocket = socket(result->ai_family, result->ai_socktype, result->ai_protocol);
	if (ListenSocket == INVALID_SOCKET) {
		printf("socket failed with error: %ld\n", WSAGetLastError());
		freeaddrinfo(result);
		WSACleanup();
		system("pause");
		return 1;
	}

	// Setup the TCP listening socket
	iResult = bind(ListenSocket, result->ai_addr, (int)result->ai_addrlen);
	if (iResult == SOCKET_ERROR) {
		printf("bind failed with error: %d\n", WSAGetLastError());
		freeaddrinfo(result);
		closesocket(ListenSocket);
		WSACleanup();
		system("pause");
		return 1;
	}

	freeaddrinfo(result);

	iResult = listen(ListenSocket, SOMAXCONN);
	if (iResult == SOCKET_ERROR) {
		printf("listen failed with error: %d\n", WSAGetLastError());
		closesocket(ListenSocket);
		WSACleanup();
		system("pause");
		return 1;
	}

	//Establish Server
	char ServerName[256];

	gethostname(ServerName, 256);

	printf("Server Established: %s \n", ServerName);
	bool quit = false;
	do
	{
		// Accept a client socket
		ClientSocket = accept(ListenSocket, NULL, NULL);
		if (ClientSocket == INVALID_SOCKET) {
			printf("accept failed with error: %d\n", WSAGetLastError());
			closesocket(ListenSocket);
			WSACleanup();
			system("pause");
			return 1;
		}

		// No longer need server socket but keeping it for multiple uses
		//closesocket(ListenSocket);

		// Receive until the peer shuts down the connection
		do {
			Message RCV;
			Message RCV2;
			RCV.MessageType = Query;

			iResult = recv(ClientSocket, RCV, RCV2, 0);
			if (iResult > 0)
			{
				printf("\nMessage received from Client.\n");

				Message SND;

				//Still gots things to do here....
				GetClientMessage(RCV, RCV2, SND, quit);

				// Echo the buffer back to the sender
				iSendResult = send(ClientSocket, SND, 0);
				if (iSendResult == SOCKET_ERROR) {
					printf("send failed with error: %d\n", WSAGetLastError());
					closesocket(ClientSocket);
				}
				printf("\nReply to Client Sent\n");
			}
			else if (iResult == 0)
			{
				printf("\nConnection with Client closing...\n");
			}
			else 
			{
				printf("recv failed with error: %d\n", WSAGetLastError());
				closesocket(ClientSocket);
			}

		} while (iResult > 0);
	} while (!quit);

	closesocket(ListenSocket);
	WSACleanup();
	system("pause");
	return 0;

}