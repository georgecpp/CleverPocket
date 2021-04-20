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
	}
	if (!instance->IsConnected())
	{
		// replace with credentials.
		instance->ConnectToServer();
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
	std::random_device engine;
	unsigned x = engine();
	return std::to_string(x);
}

bool Client::ClientIsConnected()
{
	return instance->IsConnected();
}

void Client::ConnectToServer()
{	
	instance->setIpPortTo();
	instance->Connect(instance->ip_address_to, instance->port_to);
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
void Client::Register(const clever::CredentialHandler& credentials)
{
	clever::message<clever::MessageType> msg;
	msg.header.id = clever::MessageType::RegisterRequest;
	char l_firstname[1024]; strcpy(l_firstname, credentials.getFirstName());
	char l_lastname[1024]; strcpy(l_lastname, credentials.getLastName());
	char l_username[1024]; strcpy(l_username, credentials.getUsername());
	char l_password[1024]; strcpy(l_password, credentials.getPassword());
	char l_email[1024]; strcpy(l_email, credentials.getEmail());
	char l_countryid[1024]; strcpy(l_countryid, credentials.getCountryID());
	char l_phoneNumber[1024]; strcpy(l_phoneNumber, credentials.getPhoneNumber());

	msg << l_phoneNumber << l_countryid << l_email << l_password << l_username << l_lastname << l_firstname;
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

void Client::SendEmailForgotPassword(const std::string& emailTo)
{
	clever::message<clever::MessageType> msg;
	msg.header.id = clever::MessageType::SendEmailForgotPasswordRequest;
	char l_email[1024]; strcpy(l_email, emailTo.c_str());
	msg << l_email;
	Send(msg);
}

void Client::ValidateMySixDigitCode(const std::string& validation_code)
{
	clever::message<clever::MessageType> msg;
	msg.header.id = clever::MessageType::VerifyCodeForgotPasswordRequest;
	char l_validCode[1024]; strcpy(l_validCode, validation_code.c_str());
	msg << l_validCode;
	Send(msg);
}

void Client::UpdatePasswordRequest(const std::string& newPassword, const std::string& userRequesting)
{
	clever::message<clever::MessageType> msg;
	msg.header.id = clever::MessageType::UpdatePasswordRequest;
	char l_email[1024]; strcpy(l_email, userRequesting.c_str());
	char l_newPassword[1024]; strcpy(l_newPassword, newPassword.c_str());

	msg << l_newPassword << l_email;
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
