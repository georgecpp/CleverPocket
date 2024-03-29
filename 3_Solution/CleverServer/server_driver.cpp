#define _CRT_SECURE_NO_WARNINGS
#include <iostream>
#include <clever_core.h>

class Server : public clever::server_interface<clever::MessageType>
{
public:
	Server(uint16_t port) : clever::server_interface<clever::MessageType>(port)
	{
		if (checkIfUptimeToSendDailyMails())
		{
			SendDailyNotification();
		}
		InitReccurentTransactions();
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
			char l_mailState[1024];
			char l_Picture[1024];
			char l_PhoneNumber[1024];
			char l_Country[1024];
			char l_Email[1024];
			char l_LastName[1024];
			char l_FirstName[1024];

			clever::CredentialHandler userInfo;


			std::string s_username="";
			std::string s_cashValue;
			std::string s_currencyISO;
			std::string s_mailState = "";
			try
			{
				OnGetCashPAT(l_pat, s_cashValue, s_currencyISO, s_username,userInfo, s_mailState);
				strcpy(responseBack, "SuccesGetCashDetails");
				strcpy(l_cashValue, s_cashValue.c_str());
				strcpy(l_currencyCashISO, s_currencyISO.c_str());
				strcpy(l_username, s_username.c_str());
				strcpy(l_mailState, s_mailState.c_str());
				strcpy(l_Picture, userInfo.getPassword()); 
				strcpy(l_PhoneNumber, userInfo.getPhoneNumber());
				strcpy(l_Country, userInfo.getCountryID());
				strcpy(l_Email, userInfo.getEmail());
				strcpy(l_LastName, userInfo.getLastName());
				strcpy(l_FirstName, userInfo.getFirstName());

			}
			catch (...)
			{
				strcpy(responseBack, "FailGetCashDetails");
			}
			msg.header.id = clever::MessageType::ServerGetCashResponse;
			msg << l_mailState << l_Picture << l_PhoneNumber << l_Country << l_Email << l_LastName << l_FirstName << l_username << l_cashValue << l_currencyCashISO<< responseBack;
			client->Send(msg);
		}
		break;

		case clever::MessageType::UsernameGetCashRequest:
		{
			std::cout << "[" << client->GetID() << "]: Get Cash (Username) request\n";
			char l_username[1024]; msg >> l_username;

			char l_cashValue[1024];
			char l_currencyCashISO[1024];
			char responseBack[1024];
			char l_mailState[1024];
			char l_Picture[1024];
			char l_PhoneNumber[1024];
			char l_Country[1024];
			char l_Email[1024];
			char l_LastName[1024];
			char l_FirstName[1024];

			clever::CredentialHandler userInfo;

			std::string s_cashValue;
			std::string s_currencyISO;
			std::string s_mailState = "";
			try
			{
				OnGetCashUsername(l_username, s_cashValue, s_currencyISO, userInfo, s_mailState);
				strcpy(responseBack, "SuccesGetCashDetails");
				strcpy(l_cashValue, s_cashValue.c_str());
				strcpy(l_currencyCashISO, s_currencyISO.c_str());
				strcpy(l_mailState, s_mailState.c_str());
				strcpy(l_Picture, userInfo.getPassword());
				strcpy(l_PhoneNumber, userInfo.getPhoneNumber());
				strcpy(l_Country, userInfo.getCountryID());
				strcpy(l_Email, userInfo.getEmail());
				strcpy(l_LastName, userInfo.getLastName());
				strcpy(l_FirstName, userInfo.getFirstName());
			}
			catch (...)
			{
				strcpy(responseBack, "FailGetCashDetails");
			}
			msg.header.id = clever::MessageType::ServerGetCashResponse;
			msg << l_mailState << l_Picture << l_PhoneNumber << l_Country << l_Email << l_LastName << l_FirstName << l_username << l_cashValue << l_currencyCashISO << responseBack;
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

		case clever::MessageType::AddSpendingUsernameRequest:
		{
			std::cout << "[" << client->GetID() << "]: Add spendings (username) request\n";
			char l_tranzTitle[1024]; msg >> l_tranzTitle;
			char l_tranzFinanceType[1024]; msg >> l_tranzFinanceType;
			char l_tranzCurrencyISO[1024]; msg >> l_tranzCurrencyISO;
			char l_tranzDetails[1024]; msg >> l_tranzDetails;
			char l_tranzDestination[1024]; msg >> l_tranzDestination;
			char l_tranzValue[1024]; msg >> l_tranzValue;
			char l_tranzCategoryName[1024]; msg >> l_tranzCategoryName;
			char l_username[1024]; msg >> l_username;

			std::vector<std::string> spending_details = { l_tranzCategoryName, 
													      l_tranzValue,
														  l_tranzDestination,
														  l_tranzDetails,
														  l_tranzCurrencyISO, 
														  l_tranzFinanceType,
														  l_tranzTitle};
			char responseBack[1024];
			try
			{
				OnAddSpendingsUsername(l_username, spending_details);
				strcpy(responseBack, "SuccessAddSpendings");
			}
			catch (...)
			{
				strcpy(responseBack, "FailAddSpending");
			}
			msg.header.id = clever::MessageType::ServerSpendingsResponse;
			msg << responseBack;
			client->Send(msg);
		}
		break;

		case clever::MessageType::AddPicturePATRequest:
		{
			std::cout << "[" << client->GetID() << "]: Add picture (PAT) request\n";
			char l_hexImg[1024]; msg >> l_hexImg;
			char l_pat[1024]; msg >> l_pat;
			char responseBack[1024];
			try
			{
				OnAddPicturePAT(l_pat, l_hexImg);
				strcpy(responseBack, "SuccessAddPicture");
			}
			catch (...)
			{
				strcpy(responseBack, "FailAddPicture");
			}
			msg.header.id = clever::MessageType::ServerAddPictureResponse;
			msg << responseBack;
			client->Send(msg);
		}
		break;

		case clever::MessageType::AddPreferencesOptionRequest:
		{
			std::cout << "[" << client->GetID() << "]: Add preferences option (Username) request\n";
			char l_currencyISO[1024]; msg >> l_currencyISO;
			char l_dailyMail[1024]; msg >> l_dailyMail;
			char l_username[1024]; msg >> l_username;
			char responseBack[1024];
			try
			{
				OnAddPreferencesUsername(l_username, l_dailyMail, l_currencyISO);
				strcpy(responseBack, "SuccessAddPreferencesOption");
			}
			catch (clever::AlreadyCheckedForDailyNotification)
			{
				strcpy(responseBack, "AlreadyCheckedDailyNotification");
			}
			catch (...)
			{
				strcpy(responseBack, "FailAddPreferencesOption");
			}
			msg.header.id = clever::MessageType::ServerAddPreferencesOptionResponse;
			msg << responseBack;
			client->Send(msg);
		}
		break;

		case clever::MessageType::AddPictureUsernameRequest:
		{
			std::cout << "[" << client->GetID() << "]: Add picture (Username) request\n";
			char l_hexImg[1024]; msg >> l_hexImg;
			char l_username[1024]; msg >> l_username;
			char responseBack[1024];
			try
			{
				OnAddPictureUsername(l_username, l_hexImg);
				strcpy(responseBack, "SuccessAddPicture");
			}
			catch (...)
			{
				strcpy(responseBack, "FailAddPicture");
			}
			msg.header.id = clever::MessageType::ServerAddPictureResponse;
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

		case clever::MessageType::UserGetRecurenciesRequest:
		{
			std::cout << "[" << client->GetID() << "]: Get Recurencies (Username) request\n";
			char l_username[1024]; msg >> l_username;
			std::vector<clever::FinanceTypeCredentialHandler> incomes;
			msg.header.id = clever::MessageType::ServerGetRecurenciesResponse;
			try
			{
				OnGetRecurenciesUsername(l_username, incomes);
				// now incomes is filled.
				int incomesToCome = (int)incomes.size();
				msg << incomesToCome;
				client->Send(msg);
				for (std::vector<clever::FinanceTypeCredentialHandler>::iterator iter = incomes.begin(); iter != incomes.end(); iter++)
				{
					char recurenciesName[1024]; strcpy(recurenciesName, iter->getFinanceTypeName());
					char recurenciesReceiver[1024]; strcpy(recurenciesReceiver, iter->getFinanceTypeSource());
					char recurenciesCard[1024]; strcpy(recurenciesCard, iter->getFinanceTypeToCard());
					char recurenciesISO[1024]; strcpy(recurenciesISO, iter->getFinanceTypeCurrencyISO());
					char dayOfMonth[1024]; strcpy(dayOfMonth, iter->getDayOfFinanceType());
					char recurenciesTypeID[1024]; strcpy(recurenciesTypeID, iter->getFinanceTypeRecurencies());
					float recurenciesValue = iter->getFinanceTypeValue();
					msg<< recurenciesValue << recurenciesTypeID << dayOfMonth << recurenciesISO << recurenciesCard << recurenciesReceiver << recurenciesName;
					client->Send(msg);
				}
			}
			catch (...)
			{
				char responseBack[1024];
				strcpy(responseBack, "FailGetIncomes");
				msg << responseBack;
				client->Send(msg);
			}
		}
		break;

		case clever::MessageType::AddIncomeUsernameRequest:
		{
			std::cout << "[" << client->GetID() << "]: Add income (username) request\n";

			char l_incomeValue[1024]; msg >> l_incomeValue;
			char l_incomeToCard[1024]; msg >> l_incomeToCard;
			char l_incomeSource[1024]; msg >> l_incomeSource;
			char l_dayOfIncome[1024]; msg >> l_dayOfIncome;
			char l_incomeCurrencyISO[1024]; msg >> l_incomeCurrencyISO;
			char l_incomeName[1024]; msg >> l_incomeName;
			char l_username[1024]; msg >> l_username;
			clever::FinanceTypeCredentialHandler incomeCredHandler(l_incomeName, l_incomeSource, l_incomeCurrencyISO, l_dayOfIncome, l_incomeToCard, std::atof(l_incomeValue));
			char responseBack[1024];
			try
			{
				OnAddIncomeUsername(l_username, incomeCredHandler);
				strcpy(responseBack, "SuccessAddIncome");
			}
			catch (...)
			{
				strcpy(responseBack, "FailAddIncomeCard");
			}
			msg.header.id = clever::MessageType::ServerIncomeResponse;
			msg << responseBack;
			client->Send(msg);
		}
		break;

		case clever::MessageType::AddOutcomeUsernameRequest:
		{
			std::cout << "[" << client->GetID() << "]: Add outcome (username) request\n";

			char l_outcomeValue[1024]; msg >> l_outcomeValue;
			char l_outcomeToCard[1024]; msg >> l_outcomeToCard;
			char l_outcomeSource[1024]; msg >> l_outcomeSource;
			char l_dayOfOutcome[1024]; msg >> l_dayOfOutcome;
			char l_outcomeCurrencyISO[1024]; msg >> l_outcomeCurrencyISO;
			char l_outcomeName[1024]; msg >> l_outcomeName;
			char l_username[1024]; msg >> l_username;
			clever::FinanceTypeCredentialHandler outcomeCredHandler(l_outcomeName, l_outcomeSource, l_outcomeCurrencyISO, l_dayOfOutcome, l_outcomeToCard, std::atof(l_outcomeValue));
			char responseBack[1024];
			try
			{
				OnAddOutcomeUsername(l_username, outcomeCredHandler);
				strcpy(responseBack, "SuccessAddOutcome");
			}
			catch (...)
			{
				strcpy(responseBack, "FailAddOutcomeCard");
			}
			msg.header.id = clever::MessageType::ServerOutcomeResponse;
			msg << responseBack;
			client->Send(msg);
		}
		break;

		case clever::MessageType::GetSavingsUsernameRequest:
		{
			std::cout << "[" << client->GetID() << "]: Get Savings (username) request\n";

			char l_username[1024]; msg >> l_username;
			std::vector<clever::SavingHandler> savings;
			msg.header.id = clever::MessageType::ServerGetSavingsResponse;
			try
			{
				OnGetSavingsUsername(l_username, savings);
				int savingsToCome = (int)savings.size();
				msg << savingsToCome;
				client->Send(msg);
				for (std::vector<clever::SavingHandler>::iterator iter = savings.begin(); iter != savings.end(); iter++)
				{
					char savingTitle[1024]; strcpy(savingTitle, iter->getSavingTitle());
					float savingGoal = iter->getSavingGoal();
					float savingCurrMoney = iter->getSavingCurrMoney();
					char savingCurrencyISO[1024]; strcpy(savingCurrencyISO, iter->getSavingCurrencyISO());
					char savingInitialDate[1024]; strcpy(savingInitialDate, iter->getSavingInitialDate());
					msg << savingInitialDate << savingCurrencyISO << savingCurrMoney << savingGoal << savingTitle;
					client->Send(msg);
				}
			}
			catch (...)
			{
				char responseBack[1024];
				strcpy(responseBack, "FailGetIncomes");
				msg << responseBack;
				client->Send(msg);
			}
		}
		break;

		case clever::MessageType::AddFundsToSavingUsernameRequest:
		{
			std::cout << "[" << client->GetID() << "]: Add Funds to saving (username) request\n";
			char l_username[1024];
			char l_value[1024];
			char l_fromCardName[1024]; 
			char l_toSaving[1024]; 
			msg >> l_toSaving;
			msg >> l_fromCardName;
			msg >> l_value;
			msg >> l_username;

			char responseBack[1024];
			msg.header.id = clever::MessageType::ServerAddFundsToSavingResponse;
			try
			{
				OnAddFundsToSavingUsername(l_username, l_fromCardName, l_value, l_toSaving);
				strcpy(responseBack, "SuccessAddFundsToSaving");
			}
			catch (...)
			{
				strcpy(responseBack, "FailAddFundsToSaving");
			}
			msg << responseBack;
			client->Send(msg);
		}
		break;

		case clever::MessageType::AddSavingUsernameRequest:
		{
			std::cout << "[" << client->GetID() << "]: Add new Saving (username) request\n";
			char l_username[1024];
			char l_title[1024];
			float l_goal;
			float l_currMoney;
			char l_currencyISO[1024];
			char l_initialDate[1024];

			msg >> l_initialDate;
			msg >> l_currencyISO;
			msg >> l_currMoney;
			msg >> l_goal;
			msg >> l_title;
			msg >> l_username;

			char responseBack[1024];
			msg.header.id = clever::MessageType::ServerAddSavingResponse;
			try
			{
				clever::SavingHandler saving(l_title, l_goal, l_currencyISO, l_initialDate, l_currMoney);
				OnAddSavingUsername(l_username, saving);
				strcpy(responseBack, "SuccessAddSaving");
			}
			catch (...)
			{
				strcpy(responseBack, "FailAddSaving");
			}
			msg << responseBack;
			client->Send(msg);
		}

		break;

		case clever::MessageType::GetBudgetRequest:
		{
			std::cout << "[" << client->GetID() << "]: Get Budget (username) request\n";
			char l_username[1024];

			msg >> l_username;

			char responseBack[1024];
			clever::BudgetHandler budget;
			msg.header.id = clever::MessageType::ServerGetBudgetResponse;
			try
			{
				OnGetBudgetUsername(l_username, budget);
				char budgetStartDate[1024]; strcpy(budgetStartDate, budget.getBudgetStartDate());
				char budgetEndDate[1024]; strcpy(budgetEndDate, budget.getBudgetEndDate());
				float budgetValue = budget.getBudgetValue();
				strcpy(responseBack, "SuccessGetBudget");
				msg << budgetValue << budgetEndDate << budgetStartDate << responseBack;
			}
			catch (...)
			{
				strcpy(responseBack, "FailGetBudget");
				msg << responseBack;
			}

			client->Send(msg);
		}
		break;

		case clever::MessageType::AddBudgetRequest:
		{
			std::cout << "[" << client->GetID() << "]: Add Budget (username) request\n";
			char l_username[1024];
			char l_startDate[1024];
			char l_endDate[1024];
			float l_value;

			msg >> l_value;
			msg >> l_endDate;
			msg >> l_startDate;
			msg >> l_username;

			char responseBack[1024];
			msg.header.id = clever::MessageType::ServerAddBudgetResponse;
			clever::BudgetHandler budget(l_startDate, l_endDate, l_value);
			try
			{
				OnAddBudget(l_username, budget);
				strcpy(responseBack, "SuccessAddBudget");
			}
			catch (...)
			{
				strcpy(responseBack, "FailAddBudget");
			}
			msg << responseBack;
			client->Send(msg);
		}
		break;

		case clever::MessageType::DeleteBudgetRequest:
		{
			std::cout << "[" << client->GetID() << "]: Delete Budget (username) request\n";
			char l_username[1024];
			msg >> l_username;

			char responseBack[1024];
			msg.header.id = clever::MessageType::ServerDeleteBudgetResponse;
			try
			{
				OnDeleteBudget(l_username);
				strcpy(responseBack, "SuccessDeleteBudget");
			}
			catch (...)
			{
				strcpy(responseBack, "FailDeleteBudget");
			}
			msg << responseBack;
			client->Send(msg);
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
		if (server.checkIfUptimeToSendDailyMails())
		{
			server.SendDailyNotification();
		}
		if (server.checkIfToVerifyBudgets())
		{
			server.UpdateAndCheckBudgets();
		}
		server.Update(-1, true);
	}
	return 0;
}