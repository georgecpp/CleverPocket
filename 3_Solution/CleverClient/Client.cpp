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

void Client::LogoutWithRememberMe(const std::string& PAT)
{
	clever::message<clever::MessageType> msg;
	msg.header.id = clever::MessageType::LogoutRememberedRequest;
	char l_pat[1024]; strcpy(l_pat, PAT.c_str());
	msg << l_pat;
	Send(msg);
}

void Client::UserPATAddCard(const std::string& PAT, const clever::CardCredentialHandler& cardCredHandler)
{
	clever::message<clever::MessageType> msg;
	msg.header.id = clever::MessageType::AddCardPATRequest;
	char l_cardName[1024]; strcpy(l_cardName, cardCredHandler.getCardName());
	char l_cardHolder[1024]; strcpy(l_cardHolder, cardCredHandler.getCardHolder());
	char l_cardNumber[1024]; strcpy(l_cardNumber, cardCredHandler.getCardNumber());
	char l_cardCurrencyISO[1024]; strcpy(l_cardCurrencyISO, cardCredHandler.getCardCurrencyISO());
	char l_cardValidUntil[1024]; strcpy(l_cardValidUntil, cardCredHandler.getCardValidUntil());
	char l_pat[1024]; strcpy(l_pat, PAT.c_str());

	msg << l_pat << l_cardValidUntil << l_cardCurrencyISO << l_cardNumber << l_cardHolder << l_cardName;
	Send(msg);
}

void Client::UsernameAddCard(const std::string& username, const clever::CardCredentialHandler& cardCredHandler)
{
	clever::message<clever::MessageType> msg;
	msg.header.id = clever::MessageType::AddCardUsernameRequest;
	char l_cardName[1024]; strcpy(l_cardName, cardCredHandler.getCardName());
	char l_cardHolder[1024]; strcpy(l_cardHolder, cardCredHandler.getCardHolder());
	char l_cardNumber[1024]; strcpy(l_cardNumber, cardCredHandler.getCardNumber());
	char l_cardCurrencyISO[1024]; strcpy(l_cardCurrencyISO, cardCredHandler.getCardCurrencyISO());
	char l_cardValidUntil[1024]; strcpy(l_cardValidUntil, cardCredHandler.getCardValidUntil());
	char l_username[1024]; strcpy(l_username, username.c_str());

	msg << l_username << l_cardValidUntil << l_cardCurrencyISO << l_cardNumber << l_cardHolder << l_cardName;
	Send(msg);
}

void Client::PATGetCardsDetails(const std::string& PAT)
{
	clever::message<clever::MessageType> msg;
	msg.header.id = clever::MessageType::PATGetCardRequest;

	char l_pat[1024]; strcpy(l_pat, PAT.c_str());
	msg << l_pat;
	Send(msg);
}

void Client::UsernameGetCardsDetails(const std::string& username)
{
	clever::message<clever::MessageType> msg;
	msg.header.id = clever::MessageType::UserGetCardRequest;

	char l_username[1024]; strcpy(l_username, username.c_str());
	msg << l_username;
	Send(msg);
}

void Client::UserPATAddCardFunds(const std::string& PAT, const std::string& currCardName, const std::string& fundsValue)
{
	clever::message<clever::MessageType> msg;
	msg.header.id = clever::MessageType::AddCardFundsPATRequest;
	char l_pat[1024]; strcpy(l_pat, PAT.c_str());
	char l_cardname[1024]; strcpy(l_cardname, currCardName.c_str());
	char l_fundsValue[1024]; strcpy(l_fundsValue, fundsValue.c_str());
	msg << l_fundsValue << l_cardname << l_pat;
	Send(msg);
}

void Client::UsernameAddCardFunds(const std::string& username, const std::string& currCardName, const std::string& fundsValue)
{
	clever::message<clever::MessageType> msg;
	msg.header.id = clever::MessageType::AddCardFundsUsernameRequest;
	char l_username[1024]; strcpy(l_username, username.c_str());
	char l_cardname[1024]; strcpy(l_cardname, currCardName.c_str());
	char l_fundsValue[1024]; strcpy(l_fundsValue, fundsValue.c_str());
	msg << l_fundsValue << l_cardname << l_username;
	Send(msg);
}

void Client::UserPATEditCard(const std::string& PAT, const clever::CardCredentialHandler& cardCredHandler, const std::string& oldcardname)
{
	clever::message<clever::MessageType> msg;
	msg.header.id = clever::MessageType::EditCardPATRequest;
	char l_cardName[1024]; strcpy(l_cardName, cardCredHandler.getCardName());
	char l_cardHolder[1024]; strcpy(l_cardHolder, cardCredHandler.getCardHolder());
	char l_cardNumber[1024]; strcpy(l_cardNumber, cardCredHandler.getCardNumber());
	char l_cardCurrencyISO[1024]; strcpy(l_cardCurrencyISO, cardCredHandler.getCardCurrencyISO());
	char l_cardValidUntil[1024]; strcpy(l_cardValidUntil, cardCredHandler.getCardValidUntil());
	char l_pat[1024]; strcpy(l_pat, PAT.c_str());
	char l_oldcardname[1024]; strcpy(l_oldcardname, oldcardname.c_str());

	msg << l_oldcardname<< l_pat << l_cardValidUntil << l_cardCurrencyISO << l_cardNumber << l_cardHolder << l_cardName;
	Send(msg);
}

void Client::UsernameEditCard(const std::string& username, const clever::CardCredentialHandler& cardCredHandler, const std::string& oldcardname)
{
	clever::message<clever::MessageType> msg;
	msg.header.id = clever::MessageType::EditCardUsernameRequest;
	char l_cardName[1024]; strcpy(l_cardName, cardCredHandler.getCardName());
	char l_cardHolder[1024]; strcpy(l_cardHolder, cardCredHandler.getCardHolder());
	char l_cardNumber[1024]; strcpy(l_cardNumber, cardCredHandler.getCardNumber());
	char l_cardCurrencyISO[1024]; strcpy(l_cardCurrencyISO, cardCredHandler.getCardCurrencyISO());
	char l_cardValidUntil[1024]; strcpy(l_cardValidUntil, cardCredHandler.getCardValidUntil());
	char l_username[1024]; strcpy(l_username, username.c_str());
	char l_oldcardname[1024]; strcpy(l_oldcardname, oldcardname.c_str());

	msg << l_oldcardname << l_username << l_cardValidUntil << l_cardCurrencyISO << l_cardNumber << l_cardHolder << l_cardName;
	Send(msg);
}

void Client::PATGetCashDetails(const std::string& PAT)
{
	clever::message<clever::MessageType> msg;
	msg.header.id = clever::MessageType::PATGetCashRequest;

	char l_pat[1024]; strcpy(l_pat, PAT.c_str());
	msg << l_pat;
	Send(msg);
}

void Client::UsernameGetCashDetails(const std::string& username)
{
	clever::message<clever::MessageType> msg;
	msg.header.id = clever::MessageType::UsernameGetCashRequest;

	char l_username[1024]; strcpy(l_username, username.c_str());
	msg << l_username;
	Send(msg);
}

void Client::UserPATAddCash(const std::string& PAT, const std::string& cashValue, const std::string& fromCardName)
{
	clever::message<clever::MessageType> msg;
	msg.header.id = clever::MessageType::AddCashPATRequest;
	char l_pat[1024]; strcpy(l_pat, PAT.c_str());
	char l_cashValue[1024]; strcpy(l_cashValue, cashValue.c_str());
	char l_cardname[1024]; strcpy(l_cardname, fromCardName.c_str());
	msg << l_cardname << l_cashValue << l_pat;
	Send(msg);
}

void Client::UsernameAddCash(const std::string& username, const std::string& cashValue, const std::string& fromCardName)
{
	clever::message<clever::MessageType> msg;
	msg.header.id = clever::MessageType::AddCashUsernameRequest;
	char l_username[1024]; strcpy(l_username, username.c_str());
	char l_cashValue[1024]; strcpy(l_cashValue, cashValue.c_str());
	char l_cardname[1024]; strcpy(l_cardname, fromCardName.c_str());
	msg << l_cardname << l_cashValue << l_username;
	Send(msg);
}

void Client::UserGetTranzactions(const std::string& username)
{
	clever::message<clever::MessageType> msg;
	msg.header.id = clever::MessageType::GetTranzactionsRequest;
	char l_username[1024]; strcpy(l_username, username.c_str());
	msg << l_username;
	Send(msg);
}

void Client::AddUsernamePicture(const std::string& username, const std::string& hexImg)
{
	clever::message<clever::MessageType> msg;
	msg.header.id = clever::MessageType::AddPictureUsernameRequest;
	char l_username[1024]; strcpy(l_username, username.c_str());
	char l_hexImg[1024]; strcpy(l_hexImg, hexImg.c_str());
	msg << l_username << l_hexImg;
	Send(msg);
}

void Client::AddPATPicture(const std::string& PAT, const std::string& hexImg)
{
	clever::message<clever::MessageType> msg;
	msg.header.id = clever::MessageType::AddPicturePATRequest;
	char l_pat[MAX_IMG_LEN]; strcpy(l_pat, PAT.c_str());
	char l_hexImg[MAX_IMG_LEN]; strcpy(l_hexImg, hexImg.c_str());
	msg << l_pat << l_hexImg;
	Send(msg);
}

void Client::AddPreferences(const std::string& username, const std::string& dailyMailState, const std::string& cashCurrencyISO)
{
	clever::message<clever::MessageType> msg;
	msg.header.id = clever::MessageType::AddPreferencesOptionRequest;
	char l_username[1024]; strcpy(l_username, username.c_str());
	char l_dailyMailState[1024]; strcpy(l_dailyMailState, dailyMailState.c_str());
	char l_cashCurrencyISO[1024]; strcpy(l_cashCurrencyISO, cashCurrencyISO.c_str());
	msg << l_username << l_dailyMailState << l_cashCurrencyISO;
	Send(msg);
}

void Client::UsernameAddIncome(const std::string& username, const clever::FinanceTypeCredentialHandler& incomeCredHandler)
{
	clever::message<clever::MessageType> msg;
	msg.header.id = clever::MessageType::AddIncomeUsernameRequest;
	char l_username[1024]; strcpy(l_username, username.c_str());
	char l_incomeName[1024]; strcpy(l_incomeName, incomeCredHandler.getFinanceTypeName());
	char l_incomeCurrencyISO[1024]; strcpy(l_incomeCurrencyISO, incomeCredHandler.getFinanceTypeCurrencyISO());
	char l_dayOfIncome[1024]; strcpy(l_dayOfIncome, incomeCredHandler.getDayOfFinanceType());
	char l_incomeSource[1024]; strcpy(l_incomeSource, incomeCredHandler.getFinanceTypeSource());
	char l_incomeToCard[1024]; strcpy(l_incomeToCard, incomeCredHandler.getFinanceTypeToCard());
	char l_incomeValue[1024]; strcpy(l_incomeValue, std::to_string(incomeCredHandler.getFinanceTypeValue()).c_str());

	msg << l_username << l_incomeName << l_incomeCurrencyISO << l_dayOfIncome << l_incomeSource << l_incomeToCard << l_incomeValue;
	Send(msg);
}

void Client::UsernameGetRecurenciesDetails(const std::string& username)
{
	clever::message<clever::MessageType> msg;
	msg.header.id = clever::MessageType::UserGetRecurenciesRequest;

	char l_username[1024]; strcpy(l_username, username.c_str());
	msg << l_username;
	Send(msg);
}

void Client::UsernameAddOutcome(const std::string& username, const clever::FinanceTypeCredentialHandler& outcomeCredHandler)
{
	clever::message<clever::MessageType> msg;
	msg.header.id = clever::MessageType::AddOutcomeUsernameRequest;
	char l_username[1024]; strcpy(l_username, username.c_str());
	char l_outcomeName[1024]; strcpy(l_outcomeName, outcomeCredHandler.getFinanceTypeName());
	char l_outcomeCurrencyISO[1024]; strcpy(l_outcomeCurrencyISO, outcomeCredHandler.getFinanceTypeCurrencyISO());
	char l_dayOfOutcome[1024]; strcpy(l_dayOfOutcome, outcomeCredHandler.getDayOfFinanceType());
	char l_outcomeSource[1024]; strcpy(l_outcomeSource, outcomeCredHandler.getFinanceTypeSource());
	char l_outcomeToCard[1024]; strcpy(l_outcomeToCard, outcomeCredHandler.getFinanceTypeToCard());
	char l_outcomeValue[1024]; strcpy(l_outcomeValue, std::to_string(outcomeCredHandler.getFinanceTypeValue()).c_str());

	msg << l_username << l_outcomeName << l_outcomeCurrencyISO << l_dayOfOutcome << l_outcomeSource << l_outcomeToCard << l_outcomeValue;
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
