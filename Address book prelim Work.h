/* Kyle Groom
 * CS317
 * 10/24/2014
 */
 
#include <iostream>
#include <string>
using namespace std;
Enum MessageTypes {Query = 'Q', Response = 'R'}

typedef unsigned_int8 TinyInt;
const TinyInt MaxStringSize = 255;

struct Node{
	private string FULLNAME = "";
	private string EMAIL = "";
	
	Node(string FN, string email)
	{
		FULLNAME = FN;
		EMAIL = email;
		
		
	}
}

struct Message {
	private MessageTypes MessageType;
	private TinyInt AStringLength;
	private string AString;
	
	Message(MessageTypes MT, TinyInt ASL, string AS)
	{
		if(AS.length() > MaxStringSize)
			AS.resize(MaxStringSize);
		if( ASL != (TinyInt) AS.length())
			ASL = (TinyInt) AS.length();
		
		AString = AS;
		AStringLength = ASL;
		MessageType = MT;
	}
	
	Message(MessageTypes MT, string AS)
	{
		if(AS.length() > MaxStringSize)
			AS.resize(MaxStringSize);
		
		AString = AS;
		AStringLength = AS.length();
		MessageType = MT;
	}
	
	public MessageTypes MT { get { return MessageType;} }
	public TinyInt ASL {get {return AStringLength;}}
	public sting AS {get {return AString;}}
}

