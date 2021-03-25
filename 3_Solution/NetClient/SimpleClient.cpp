#define _CRT_SECURE_NO_WARNINGS
#include <iostream>
#include <olc_net.h>
#include <sstream>


enum class CustomMsgTypes : uint32_t
{
	ServerAccept,
	ServerDeny,
	ServerPing,
	MessageAll,
	ServerMessage,
	RegisterRequest,
};



class CustomClient : public olc::net::client_interface<CustomMsgTypes>
{
public:
	void PingServer()
	{
		olc::net::message<CustomMsgTypes> msg;
		msg.header.id = CustomMsgTypes::ServerPing;

		// Caution with this...
		std::chrono::system_clock::time_point timeNow = std::chrono::system_clock::now();

		msg << timeNow;
		Send(msg);
	}

	void MessageAll()
	{
		olc::net::message<CustomMsgTypes> msg;
		msg.header.id = CustomMsgTypes::MessageAll;
		Send(msg);
	}

	void Register(const std::string& username, const std::string& password, const std::string& email)
	{
		std::string help = "";
		std::string readusername;
		std::string readpassword;
		std::string reademail;
		std::cout << "\nUsername: ";
		std::getline(std::cin, readusername);
		std::cout << "Password: ";
		std::getline(std::cin, readpassword);
		std::cout << "Email: ";
		std::getline(std::cin, reademail);
		std::cout << "\n";

		olc::net::message<CustomMsgTypes> msg;
		msg.header.id = CustomMsgTypes::RegisterRequest;
		char l_username[1024]; 
		char l_password[1024]; 
		char l_email[1024];

		if (readusername == "" && readpassword == "" && reademail == "")
		{
			strcpy(l_username, username.c_str());
			strcpy(l_password, password.c_str());
			strcpy(l_email, email.c_str());
		}
		else
		{
			strcpy(l_username, readusername.c_str());
			strcpy(l_password, readpassword.c_str());
			strcpy(l_email, reademail.c_str());
		}

		msg << l_username << l_password << l_email;
		Send(msg);
	}
};

std::vector<std::string> getIpPort()
{

	// CHECK ALSO FOR SECURITY!!!!! KEY BASED ALGO. TO DO.

	FILE* fin = fopen("ngrok.txt", "r");
	char buffer[1024];
	fgets(buffer, sizeof(buffer), fin);
	std::string ip_port = buffer;
	std::string ip;
	std::string port;
	std::stringstream ss(ip_port);
	std::getline(ss, ip, ':');
	std::getline(ss, port, ':');

	std::vector<std::string> utils;
	utils.push_back(ip); utils.push_back(port);

	return utils;
	
}
int main()
{
	CustomClient c;
	std::vector<std::string> connects = getIpPort();
	c.Connect(connects[0], std::stoi(connects[1]));


	bool key[4] = { false, false, false,false };
	bool old_key[4] = { false, false, false,false };

	bool bQuit = false;
	while (!bQuit)
	{
		if (GetForegroundWindow() == GetConsoleWindow()) {
			key[0] = GetAsyncKeyState('1') & 0x8000;
			key[1] = GetAsyncKeyState('2') & 0x8000;
			key[2] = GetAsyncKeyState('3') & 0x8000;
			key[3] = GetAsyncKeyState('R') & 0x8000;
		}
		
		if (key[0] && !old_key[0]) c.PingServer();
		if (key[1] && !old_key[1]) c.MessageAll();
		if (key[2] && !old_key[2]) bQuit = true;
		if (key[3] && !old_key[3]) { c.Register("rifflord", "rifflord123", "rifflord@gmail.com"); }

		for (int i = 0; i < 4; i++) old_key[i] = key[i];

		if (c.IsConnected())
		{
			if (!c.Incoming().empty())
			{

				auto msg = c.Incoming().pop_front().msg;

				switch (msg.header.id)
				{
				case CustomMsgTypes::ServerAccept:
				{
					// Server has responded to a ping request				
					std::cout << "Server Accepted Connection\n";
				}
				break;


				case CustomMsgTypes::ServerPing:
				{
					// Server has responded to a ping request
					std::chrono::system_clock::time_point timeNow = std::chrono::system_clock::now();
					std::chrono::system_clock::time_point timeThen;
					msg >> timeThen;
					std::cout << "Ping: " << std::chrono::duration<double>(timeNow - timeThen).count() << "\n";
				}
				break;

				case CustomMsgTypes::ServerMessage:
				{
					// Server has responded to a ping request	
					uint32_t clientID;
					msg >> clientID;
					std::cout << "Hello from [" << clientID << "]\n";
				}
				break;

				case CustomMsgTypes::RegisterRequest:
				{
					// server has responded to register request.
					char responseback[1024];
					msg >> responseback;
					std::cout << responseback << "\n";
				}
				break;

				}
			}
		}
		else
		{
			std::cout << "Server Down\n";
			bQuit = true;
		}

	}

	return 0;
}