#define _CRT_SECURE_NO_WARNINGS
#include "Client.h"

Client* Client::instance = nullptr;

void Client::setIpPortTo()
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
	instance->ip_address_to = ip;
	instance->port_to = std::stoi(port);
	fclose(fin);
}

Client& Client::getInstance()
{
	if (!instance)
	{
		instance = new Client();
		//instance->setIpPortTo();
	}
	if (!instance->IsConnected())
	{
		// replace with credentials.
		instance->Connect("6.tcp.ngrok.io", 12553);
	}
	return *instance;
}

void Client::destroyInstance()
{
	if (instance)
	{
		delete instance;
		instance = nullptr;
	}
}

std::string Client::generatePAT()
{
	return std::string("12345");
}

bool Client::ClientIsConnected()
{
	return instance->IsConnected();
}

void Client::PingServer()
{
		clever::message<clever::MessageType> msg;
		msg.header.id = clever::MessageType::ServerPing;

		// caution.
		std::chrono::system_clock::time_point timeNow = std::chrono::system_clock::now();

		msg << timeNow;
		Send(msg);
}

void Client::MessageAll()
{
		clever::message<clever::MessageType> msg;
		msg.header.id = clever::MessageType::MessageAll;
		// simply send the message.
		Send(msg);
}

void Client::Register(const std::string& username, const std::string& password, const std::string& email)
{
		clever::message<clever::MessageType> msg;
		msg.header.id = clever::MessageType::RegisterRequest;
		char l_username[1024];
		char l_password[1024];
		char l_email[1024];

		if (username == "" && password == "" && email == "")
		{
			strcpy(l_username, username.c_str());
			strcpy(l_password, password.c_str());
			strcpy(l_email, email.c_str());
		}
		else
		{
			strcpy(l_username, username.c_str());
			strcpy(l_password, password.c_str());
			strcpy(l_email, email.c_str());
		}

		msg << l_username << l_password << l_email;
		Send(msg);
}

void Client::LoginUser(const std::string& username, const std::string& password)
{
	clever::message<clever::MessageType> msg;
	msg.header.id = clever::MessageType::LoginRequest;
	char l_username[1024]; strcpy(l_username, username.c_str());
	char l_password[1024]; strcpy(l_password, password.c_str());
	msg << l_username << l_password;
	Send(msg);
}

void Client::LoginUserRemembered(const std::string& PAT)
{
	// to do.
	clever::message<clever::MessageType> msg;
	msg.header.id = clever::MessageType::LoginRememeberedRequest;
	char l_pat[1024];
	strcpy(l_pat, PAT.c_str());
	msg << l_pat;
	Send(msg);
}

void Client::RememberMe(const std::string& PAT, const std::string& username)
{
	clever::message<clever::MessageType> msg;
	msg.header.id = clever::MessageType::RememberMeRequest;
	char l_pat[1024]; strcpy(l_pat, PAT.c_str());
	char l_username[1024]; strcpy(l_username, username.c_str());
	msg << l_username << l_pat;
	Send(msg);
}

std::string Client::getIpAddressTo()
{
	return instance->ip_address_to;
}

int Client::getPortTo()
{
	return instance->port_to;
}
