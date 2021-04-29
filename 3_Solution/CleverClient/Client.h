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
	void LogoutWithRememberMe(const std::string& PAT);
	void UserPATAddCard(const std::string& PAT, const clever::CardCredentialHandler& cardCredHandler);
	void UsernameAddCard(const std::string& username, const clever::CardCredentialHandler& cardCredHandler);
	void PATGetCardsDetails(const std::string& PAT);
	void UsernameGetCardsDetails(const std::string& username);
	void UserPATAddCardFunds(const std::string& PAT, const std::string& currCardName, const std::string& fundsValue);
	void UsernameAddCardFunds(const std::string& username, const std::string& currCardName, const std::string& fundsValue);
	void UserPATEditCard(const std::string& PAT, const clever::CardCredentialHandler& cardCredHandler, const std::string& oldcardname);
	void UsernameEditCard(const std::string& username, const clever::CardCredentialHandler& cardCredHandler, const std::string& oldcardname);
	void PATGetCashDetails(const std::string& PAT);
	void UsernameGetCashDetails(const std::string& username);
	void UserPATAddCash(const std::string& PAT, const std::string& cashValue, const std::string& fromCardName);
	void UsernameAddCash(const std::string& username, const std::string& cashValue, const std::string& fromCardName);
	void UserGetTranzactions(const std::string& username);
	void AddUsernamePicture(const std::string& username, const std::string& hexImg);
	void AddPATPicture(const std::string& PAT, const std::string& hexImg);
	void AddPreferences(const std::string& username, const std::string& dailyMailState, const std::string& cashCurrencyISO);
	void UsernameAddIncome(const std::string& username, const clever::FinanceTypeCredentialHandler& incomeCredHandler);
	void UsernameGetRecurenciesDetails(const std::string& username);
public:
	std::string getIpAddressTo();
	int getPortTo();
};
