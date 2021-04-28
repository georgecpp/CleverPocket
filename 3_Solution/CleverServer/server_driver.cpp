#define _CRT_SECURE_NO_WARNINGS
#include <iostream>
#include <clever_core.h>

class Server : public clever::server_interface<clever::MessageType>
{
public:
	Server(uint16_t port) : clever::server_interface<clever::MessageType>(port)
	{

	}
protected:
	virtual bool OnClientConnect(std::shared_ptr<clever::connection<clever::MessageType>> client)
	{
		clever::message<clever::MessageType> msg;
		msg.header.id = clever::MessageType::ServerAccept;
		client->Send(msg);
		return true;
	}

	// Called when a client appears to have disconnected
	virtual void OnClientDisconnect(std::shared_ptr<clever::connection<clever::MessageType>> client)
	{
		std::cout << "Removing client [" << client->GetID() << "]\n";
	}

	// Called when a message arrives
	virtual void OnMessage(std::shared_ptr<clever::connection<clever::MessageType>> client, clever::message<clever::MessageType>& msg)
	{
		switch (msg.header.id)
		{
		case clever::MessageType::ServerPing:
		{
			std::cout << "[" << client->GetID() << "]: Server Ping\n";

			// Simply bounce message back to client
			client->Send(msg);
		}
		break;

		case clever::MessageType::MessageAll:
		{
			std::cout << "[" << client->GetID() << "]: Message All\n";

			// Construct a new message and send it to all clients
			clever::message<clever::MessageType> msg;
			msg.header.id = clever::MessageType::ServerMessage;
			msg << client->GetID();
			MessageAllClients(msg, client);

		}
		break;

		case clever::MessageType::RegisterRequest:
		{
			// the message contains the data for registration.
			std::cout << "[" << client->GetID() << "]: Register to DB request\n";
			char firstname[1024];
			char lastname[1024];
			char username[1024];
			char password[1024];
			char email[1024];
			char countryID[1024];
			char phonenumber[1024];

			msg >> firstname >> lastname >> username >> password >> email >> countryID >> phonenumber;
			clever::CredentialHandler credentials;
			credentials.setFirstName(firstname);
			credentials.setLastName(lastname);
			credentials.setUsername(username);
			credentials.setPassword(password);
			credentials.setEmail(email);
			credentials.setCountryID(countryID);
			credentials.setPhoneNumber(phonenumber);
			char responseback[1024];

			try
			{
				RegisterUserToDatabase(credentials);
				strcpy(responseback, "Success");
			}
			catch (clever::EmailValidationError)
			{
				strcpy(responseback, "EmailInvalid");
			}
			catch (clever::UserAlreadyRegisteredError)
			{
				strcpy(responseback, "AlreadyRegistered");
			}
			catch (clever::DatabaseQueryError)
			{
				strcpy(responseback, "ServerToDatabaseQueryError");
			}
			catch (clever::DatabaseConnectionError)
			{
				strcpy(responseback, "ServerToDatabaseConnectionError");
			}

			msg << responseback;
			client->Send(msg); // send message back to client.
		}
		break;

		case clever::MessageType::LoginRequest:
		{
			std::cout << "[" << client->GetID() << "]: Login to Server request\n";
			char username[1024];
			char password[1024];

			msg >> password >> username;
			char responseback[1024];
			try
			{
				OnLoginUserToServer(username, password);
				strcpy(responseback, "SuccessLogin");
			}
			catch (clever::UsernameInvalidLoginError)
			{
				strcpy(responseback, "UsernameInvalidError");
			}
			catch (clever::PasswordInvalidLoginError)
			{
				strcpy(responseback, "PasswordInvalidError");
			}

			msg << responseback;
			client->Send(msg); // send message back to client.
		}
		break;

		case clever::MessageType::RememberMeRequest:
		{
			std::cout << "[" << client->GetID() << "]: 'Keep Me Logged In' request\n";
			char PAT[1024];
			char username[1024];
			msg >> PAT >> username;
			char responseback[1024];
			try
			{
				OnRememberUserLoggedIn(username, PAT);
				strcpy(responseback, "SuccessRemember");
			}
			catch (clever::UsernameInvalidLoginError)
			{
				strcpy(responseback, "UsernameInvalidError");
				msg << responseback;
				client->Send(msg);
			}
		}
		break;
		
		case clever::MessageType::LoginRememeberedRequest:
		{
			std::cout << "[" << client->GetID() << "]: Remembered-Login request\n";
			char PAT[1024];
			msg >> PAT;
			char responseback[1024];
			try
			{
				if (OnLoginUserPAT(PAT) == -1)
				{
					strcpy(responseback, "InvalidPATError");
				}
				else
				{
					strcpy(responseback, "SuccessRememberLogin");
				}
			}
			catch (clever::InvalidPATLoginError)
			{
				strcpy(responseback, "InvalidPATError");
			}

			msg << responseback;
			client->Send(msg);
		}
		break;

		case clever::MessageType::SendEmailForgotPasswordRequest:
		{
			std::cout << "[" << client->GetID() << "]: Forgot Password Send Email Request\n";
			char emailTo[1024];
			msg >> emailTo;
			char responseback[1024];
			try
			{
				OnSendEmailForgotPassword(emailTo);
				strcpy(responseback, "SendEmailForgotPasswordSuccess");
			}
			catch (clever::EmailValidationError)
			{
				strcpy(responseback, "InvalidFormatEmailForgotPassword");
			}
			catch (clever::EmailInvalidForgotPasswordError)
			{
				strcpy(responseback, "InvalidEmailForgotPassword");
			}

			msg << responseback;
			client->Send(msg);
		}
		break;

		case clever::MessageType::VerifyCodeForgotPasswordRequest:
		{
			std::cout << "[" << client->GetID() << "]: Validate 6-digit update password request\n";
			char validationCode[1024];
			msg >> validationCode;
			char responseBack[1024];
			try
			{
				OnValidateSixDigitCode(validationCode);
				strcpy(responseBack, "SuccessValidationCode");
			}
			catch (clever::SixDigitCodeInvalidError)
			{
				strcpy(responseBack, "InvalidValidationCode");
			}
			
			// respond back to client, thus moving to update password form.
			msg << responseBack;
			client->Send(msg);
		}
		break;

		case clever::MessageType::UpdatePasswordRequest:
		{
			std::cout << "[" << client->GetID() << "]: Update Password request\n";
			char emailTo[1024];
			char newPassword[1024];

			msg >> emailTo >> newPassword;
			char responseBack[1024];
			try
			{
				OnUpdatePassword(newPassword, emailTo);
				strcpy(responseBack, "SuccessUpdatePassword");
			}
			catch (clever::EmailInvalidForgotPasswordError)
			{
				strcpy(responseBack, "InvalidEmailUpdatePassword");
			}
			// respond back to client, thus moving back to login form.
			msg << responseBack;
			client->Send(msg);
		}
		break;
		
		case clever::MessageType::LogoutRememberedRequest:
		{
			std::cout << "[" << client->GetID() << "]: Logout remembered request\n";
			char PAT[1024];
			msg >> PAT;
			char responseBack[1024];
			try
			{
				OnLogoutRemembered(PAT);
				strcpy(responseBack, "LogoutRememberedSuccess");
			}
			catch (clever::InvalidPATLogoutError)
			{
				strcpy(responseBack, "InvalidPATLogout");
			}

			msg << responseBack;
			client->Send(msg);
		}
		break;

		case clever::MessageType::AddCardUsernameRequest:
		{
			std::cout << "[" << client->GetID() << "]: Add card (username) request\n";
			char l_cardName[1024]; msg >> l_cardName;
			char l_cardHolder[1024]; msg >> l_cardHolder;
			char l_cardNumber[1024]; msg >> l_cardNumber;
			char l_cardCurrencyISO[1024];  msg >> l_cardCurrencyISO;
			char l_cardValidUntil[1024]; msg >> l_cardValidUntil;
			char l_username[1024]; msg >> l_username;
			clever::CardCredentialHandler cardCredHandler(l_cardName, l_cardHolder, l_cardNumber, l_cardCurrencyISO, l_cardValidUntil);
			char responseBack[1024];
			try
			{
				OnAddCardUsername(l_username, cardCredHandler);
				strcpy(responseBack, "SuccessAddCard");
			}
			catch (...)
			{
				strcpy(responseBack, "FailAddCard");
			}
			msg.header.id = clever::MessageType::ServerAddCardResponse;
			msg << responseBack;
			client->Send(msg);
		}
		break;

		case clever::MessageType::AddCardPATRequest:
		{
			std::cout << "[" << client->GetID() << "]: Add card (PAT) request\n";
			char l_cardName[1024]; msg >> l_cardName;
			char l_cardHolder[1024]; msg >> l_cardHolder;
			char l_cardNumber[1024]; msg >> l_cardNumber;
			char l_cardCurrencyISO[1024];  msg >> l_cardCurrencyISO;
			char l_cardValidUntil[1024]; msg >> l_cardValidUntil;
			char l_pat[1024]; msg >> l_pat;
			clever::CardCredentialHandler cardCredHandler(l_cardName, l_cardHolder, l_cardNumber, l_cardCurrencyISO, l_cardValidUntil);
			char responseBack[1024];
			try
			{
				OnAddCardPAT(l_pat, cardCredHandler);
				strcpy(responseBack, "SuccessAddCard");
			}
			catch (...)
			{
				strcpy(responseBack, "FailAddCard");
			}
			msg.header.id = clever::MessageType::ServerAddCardResponse;
			msg << responseBack;
			client->Send(msg);
		}
		break;

		case clever::MessageType::EditCardUsernameRequest:
		{
			std::cout << "[" << client->GetID() << "]: Edit card (username) request\n";
			char l_cardName[1024]; msg >> l_cardName;
			char l_cardHolder[1024]; msg >> l_cardHolder;
			char l_cardNumber[1024]; msg >> l_cardNumber;
			char l_cardCurrencyISO[1024];  msg >> l_cardCurrencyISO;
			char l_cardValidUntil[1024]; msg >> l_cardValidUntil;
			char l_username[1024]; msg >> l_username;
			char l_oldcardname[1024]; msg >> l_oldcardname;
			clever::CardCredentialHandler cardCredHandler(l_cardName, l_cardHolder, l_cardNumber, l_cardCurrencyISO, l_cardValidUntil);
			char responseBack[1024];
			try
			{
				OnEditCardUsername(l_username, cardCredHandler, l_oldcardname);
				strcpy(responseBack, "SuccessEditCard");
			}
			catch (...)
			{
				strcpy(responseBack, "FailEditCard");
			}
			msg.header.id = clever::MessageType::ServerEditCardResponse;
			msg << responseBack;
			client->Send(msg);
		}
		break;

		case clever::MessageType::EditCardPATRequest:
		{
			std::cout << "[" << client->GetID() << "]: Edit card (PAT) request\n";
			char l_cardName[1024]; msg >> l_cardName;
			char l_cardHolder[1024]; msg >> l_cardHolder;
			char l_cardNumber[1024]; msg >> l_cardNumber;
			char l_cardCurrencyISO[1024];  msg >> l_cardCurrencyISO;
			char l_cardValidUntil[1024]; msg >> l_cardValidUntil;
			char l_pat[1024]; msg >> l_pat;
			char l_oldcardname[1024]; msg >> l_oldcardname;
			clever::CardCredentialHandler cardCredHandler(l_cardName, l_cardHolder, l_cardNumber, l_cardCurrencyISO, l_cardValidUntil);
			char responseBack[1024];
			try
			{
				OnEditCardPAT(l_pat, cardCredHandler, l_oldcardname);
				strcpy(responseBack, "SuccessEditCard");
			}
			catch (...)
			{
				strcpy(responseBack, "FailEditCard");
			}
			msg.header.id = clever::MessageType::ServerEditCardResponse;
			msg << responseBack;
			client->Send(msg);
		}
		break;

		
		case clever::MessageType::PATGetCardRequest:
		{
			std::cout << "[" << client->GetID() << "]: Get Cards (PAT) request\n";
			char l_pat[1024]; msg >> l_pat;
			std::vector<clever::CardCredentialHandler> cards;
			msg.header.id = clever::MessageType::ServerGetCardsResponse;
			try
			{
				OnGetCardsPAT(l_pat, cards);
				// now cards is filled.
				int cardsToCome = cards.size();
				msg << cardsToCome;
				client->Send(msg);

				for (std::vector<clever::CardCredentialHandler>::iterator iter = cards.begin(); iter != cards.end(); iter++)
				{
					char cardName[1024]; strcpy(cardName, iter->getCardName());
					char cardHolder[1024]; strcpy(cardHolder, iter->getCardHolder());
					char cardNumber[1024]; strcpy(cardNumber, iter->getCardNumber());
					char cardValidUntil[1024]; strcpy(cardValidUntil, iter->getCardValidUntil());
					char cardCurrencyISO[1024]; strcpy(cardCurrencyISO, iter->getCardCurrencyISO());
					float cardSold = iter->getCardSold();
					msg << cardSold<<cardCurrencyISO << cardValidUntil << cardNumber << cardHolder << cardName;
					client->Send(msg);
				}
			}
			catch (...)
			{
				char responseBack[1024];
				strcpy(responseBack, "FailGetCards");
				msg << responseBack;
				client->Send(msg);
			}
		}
		break;

		case clever::MessageType::UserGetCardRequest:
		{
			std::cout << "[" << client->GetID() << "]: Get Cards (Username) request\n";
			char l_username[1024]; msg >> l_username;
			std::vector<clever::CardCredentialHandler> cards;
			msg.header.id = clever::MessageType::ServerGetCardsResponse;
			try
			{
				OnGetCardsUsername(l_username, cards);
				// now cards is filled.
				int cardsToCome = cards.size();
				msg << cardsToCome;
				client->Send(msg);

				for (std::vector<clever::CardCredentialHandler>::iterator iter = cards.begin(); iter != cards.end(); iter++)
				{
					char cardName[1024]; strcpy(cardName, iter->getCardName());
					char cardHolder[1024]; strcpy(cardHolder, iter->getCardHolder());
					char cardNumber[1024]; strcpy(cardNumber, iter->getCardNumber());
					char cardValidUntil[1024]; strcpy(cardValidUntil, iter->getCardValidUntil());
					char cardCurrencyISO[1024]; strcpy(cardCurrencyISO, iter->getCardCurrencyISO());
					float cardSold = iter->getCardSold();
					msg << cardSold<<cardCurrencyISO << cardValidUntil << cardNumber << cardHolder << cardName;
					client->Send(msg);
				}
			}
			catch (...)
			{
				char responseBack[1024];
				strcpy(responseBack, "FailGetCards");
				msg << responseBack;
				client->Send(msg);
			}
		}
		break;

		case clever::MessageType::AddCardFundsPATRequest:
		{
			std::cout << "[" << client->GetID() << "]: Add funds card request\n";
			char l_pat[1024]; msg >> l_pat;
			char l_cardname[1024]; msg >> l_cardname;
			char l_fundValue[1024]; msg >> l_fundValue;
			char responseBack[1024];
			try
			{

				OnAddCardFundsPAT(l_pat, l_fundValue, l_cardname);
				strcpy(responseBack, "SuccessAddFunds");
			}
			catch (...)
			{
				strcpy(responseBack, "FailAddFunds");
			}
			msg.header.id = clever::MessageType::ServerAddFundsResponse;
			msg << responseBack;
			client->Send(msg);
		}
		break;

		case clever::MessageType::AddCardFundsUsernameRequest:
		{
			std::cout << "[" << client->GetID() << "]: Add funds card request\n";
			char l_username[1024]; msg >> l_username;
			char l_cardname[1024]; msg >> l_cardname;
			char l_fundValue[1024]; msg >> l_fundValue;;
			char responseBack[1024];
			try
			{
				OnAddCardFundsUsername(l_username, l_fundValue, l_cardname);
				strcpy(responseBack, "SuccessAddFunds");
			}
			catch (...)
			{
				strcpy(responseBack, "FailAddFunds");
			}
			msg.header.id = clever::MessageType::ServerAddFundsResponse;
			msg << responseBack;
			client->Send(msg);
		}
		break;

		case clever::MessageType::PATGetCashRequest:
		{
			std::cout << "[" << client->GetID() << "]: Get Cash (PAT) request\n";
			char l_pat[1024]; msg >> l_pat;
			char l_cashValue[1024];
			char l_currencyCashISO[1024];
			char responseBack[1024];
			char l_username[1024];

			std::string s_username="";
			std::string s_cashValue;
			std::string s_currencyISO;
			try
			{
				OnGetCashPAT(l_pat, s_cashValue, s_currencyISO, s_username);
				strcpy(responseBack, "SuccesGetCashDetails");
				strcpy(l_cashValue, s_cashValue.c_str());
				strcpy(l_currencyCashISO, s_currencyISO.c_str());
				strcpy(l_username, s_username.c_str());
			}
			catch (...)
			{
				strcpy(responseBack, "FailGetCashDetails");
			}
			msg.header.id = clever::MessageType::ServerGetCashResponse;
			msg << l_username << l_cashValue << l_currencyCashISO<< responseBack;
			client->Send(msg);
		}
		break;

		case clever::MessageType::UsernameGetCashRequest:
		{
			std::cout << "[" << client->GetID() << "]: Get Cash (Username) request\n";
			char l_pat[1024]; msg >> l_pat;
			char l_cashValue[1024];
			char l_currencyCashISO[1024];
			char responseBack[1024];
			std::string s_cashValue;
			std::string s_currencyISO;
			try
			{
				OnGetCashUsername(l_pat, s_cashValue, s_currencyISO);
				strcpy(responseBack, "SuccesGetCashDetails");
				strcpy(l_cashValue, s_cashValue.c_str());
				strcpy(l_currencyCashISO, s_currencyISO.c_str());
			}
			catch (...)
			{
				strcpy(responseBack, "FailGetCashDetails");
			}
			msg.header.id = clever::MessageType::ServerGetCashResponse;
			msg << l_cashValue << l_currencyCashISO << responseBack;
			client->Send(msg);
		}
		break;

		case clever::MessageType::AddCashPATRequest:
		{
			std::cout << "[" << client->GetID() << "]: Add cash (PAT) request\n";
			char l_pat[1024]; msg >> l_pat;
			char l_cashValue[1024]; msg >> l_cashValue;
			char l_cardname[1024]; msg >> l_cardname;
			char responseBack[1024];
			try
			{
				OnAddCashPAT(l_pat, l_cashValue, l_cardname);
				strcpy(responseBack, "SuccessAddCash");
			}
			catch (...)
			{
				strcpy(responseBack, "FailAddCash");
			}
			msg.header.id = clever::MessageType::ServerAddCashResponse;
			msg << responseBack;
			client->Send(msg);
		}
		break;

		case clever::MessageType::AddCashUsernameRequest:
		{
			std::cout << "[" << client->GetID() << "]: Add cash (username) request\n";
			char l_username[1024]; msg >> l_username;
			char l_cashValue[1024]; msg >> l_cashValue;
			char l_cardname[1024]; msg >> l_cardname;
			char responseBack[1024];
			try
			{

				OnAddCashUsername(l_username, l_cashValue,l_cardname);
				strcpy(responseBack, "SuccessAddCash");
			}
			catch (...)
			{
				strcpy(responseBack, "FailAddCash");
			}
			msg.header.id = clever::MessageType::ServerAddCashResponse;
			msg << responseBack;
			client->Send(msg);
		}
		break;

		case clever::MessageType::GetTranzactionsRequest:
		{
			std::cout << "[" << client->GetID() << "]: Get Tranzactions (username) request\n";
			char l_username[1024]; msg >> l_username;
			std::vector<clever::TranzactionHandler> tranzactions;
			msg.header.id = clever::MessageType::ServerGetTranzactionsResponse;
			try
			{
				OnGetTranzactionsUsername(l_username, tranzactions);
				int tranzactionsToCome = (int)tranzactions.size();
				msg << tranzactionsToCome;
				client->Send(msg);

				for (std::vector<clever::TranzactionHandler>::iterator iter = tranzactions.begin(); iter != tranzactions.end(); iter++)
				{
					char tranzactionTitle[1024]; strcpy(tranzactionTitle, iter->getTranzactionTitle());
					char tranzactionSource[1024]; strcpy(tranzactionSource, iter->getTranzactionSource());
					char tranzactionDestination[1024]; strcpy(tranzactionDestination, iter->getTranzactionDestination());
					char tranzactionTimestamp[1024]; strcpy(tranzactionTimestamp, iter->getTranzactionTimestamp());
					char tranzactionFinanceName[1024]; strcpy(tranzactionFinanceName, iter->getTranzactionFinanceName());
					unsigned int tranzactionType = (unsigned int)iter->getTranzactionType();
					float tranzactionValue = iter->getTranzactionValue();
					char tranzactionCurrencyISO[1024]; strcpy(tranzactionCurrencyISO, iter->getTranzactionCurrencyISO());
					char tranzactionDescription[1024]; strcpy(tranzactionDescription, iter->getTranzactionDescription());
					char tranzactionCategoryName[1024]; strcpy(tranzactionCategoryName, iter->getTranzactionCategoryName());

					msg << tranzactionCategoryName << tranzactionDescription << tranzactionCurrencyISO << tranzactionValue << tranzactionType << tranzactionFinanceName;
					msg << tranzactionTimestamp << tranzactionDestination << tranzactionSource << tranzactionTitle;
					
					client->Send(msg);
				}
			}
			catch (...)
			{
				char responseBack[1024];
				strcpy(responseBack, "FailGetTranzactions");
				msg << responseBack;
				client->Send(msg);
			}
		}

		break;
		}
	}
};

int main()
{
	Server server(60000);
	server.Start();
	
	while (1) {
		server.Update(-1, true);
	}
	return 0;
}