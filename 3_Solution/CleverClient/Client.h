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
	static std::string generatePAT();
	static bool ClientIsConnected();

	void ConnectToServer();
	void PingServer();
	void MessageAll();
	void Register(const clever::CredentialHandler& credentials);
	void LoginUser(const std::string& username, const std::string& password);
	void LoginUserRemembered(const std::string& PAT);
	void RememberMe(const std::string& PAT, const std::string& username);
	void SendEmailForgotPassword(const std::string& emailTo);
	void ValidateMySixDigitCode(const std::string& validation_code);
	void UpdatePasswordRequest(const std::string& newPassword, const std::string& userRequesting);
<<<<<<< Updated upstream

=======
	void LogoutWithRememberMe(const std::string& PAT);
	void PATGetSelectedCardCurrency(const std::string& PAT, const std::string& cardName);
	void UsernameGetSelectedCardCurrency(const std::string& username, const std::string& cardName);
<<<<<<< Updated upstream
>>>>>>> Stashed changes
=======
	void UserPATAddCardFunds(const std::string& PAT, const std::string& currCardName, const std::string& fundsValue);
	void UsernameAddCardFunds(const std::string& username, const std::string& currCardName, const std::string& fundsValue);
>>>>>>> Stashed changes
public:
	std::string getIpAddressTo();
	int getPortTo();
};
