#pragma once
#include <clever_core.h>

class Client : public clever::client_interface<clever::MessageType>
{
private:
	static Client* instance;
	std::string ip_address_to;
	int port_to;
	Client() {};
	Client(const Client& src) = delete;
	~Client() {};

	void setIpPortTo();

public:
	static Client& getInstance();
	static void destroyInstance();


	void PingServer();
	void MessageAll();
	void Register(const std::string& username, const std::string& password, const std::string& email);
	void LoginUser(const std::string& username, const std::string& password);

public:
	std::string getIpAddressTo();
	int getPortTo();
};
