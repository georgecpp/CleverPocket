#pragma once
#include <tchar.h>
#include "easendmailobj.tlh"
#include <ctime>
#include "clever_inclusions.h"
#include "clever_TSQueue.h"
#include "clever_Message.h"
#include "clever_Connection.h"
#include "clever_DBConnection.h"
#include "clever_Exceptions.h"
#include "clever_Credentials.h"
#include <map>

using namespace EASendMailObjLib;
enum class ConnectSMTPCodes
{
	ConnectNormal,
	ConnectSSLAuto,
	ConnectSTARTTls,
	ConnectDirectSSL,
	ConnectTryTLS
};


namespace clever
{
	template<typename T>
	class server_interface
	{
	public:
		// Create a server, ready to listen on specified port
		server_interface(uint16_t port)
			: m_asioAcceptor(m_asioContext, asio::ip::tcp::endpoint(asio::ip::address::from_string("0.0.0.0"), port))
		{
			didntSendTodayNotification = true;
		}

		virtual ~server_interface()
		{
			// May as well try and tidy up
			Stop();
		}

		// Starts the server!
		bool Start()
		{
			try
			{
				// Issue a task to the asio context - This is important
				// as it will prime the context with "work", and stop it
				// from exiting immediately. Since this is a server, we 
				// want it primed ready to handle clients trying to
				// connect.
				WaitForClientConnection();

				// Launch the asio context in its own thread
				m_threadContext = std::thread([this]() { m_asioContext.run(); });
			}
			catch (std::exception& e)
			{
				// Something prohibited the server from listening
				std::cerr << "[SERVER] Exception: " << e.what() << "\n";
				return false;
			}

			std::cout << "[SERVER] Started!\n";
			return true;
		}

		// Stops the server!
		void Stop()
		{
			// Request the context to close
			m_asioContext.stop();

			// Tidy up the context thread
			if (m_threadContext.joinable()) m_threadContext.join();

			// Inform someone, anybody, if they care...
			std::cout << "[SERVER] Stopped!\n";
		}

		void ConnectToDatabase()
		{
			if (m_dbconnector.OnAttemptToConnect())
			{
				std::cout << "[SERVER] Server is connected successfully to the SQL Server Database!\n";
			}
			else
			{
				throw DatabaseConnectionError("[SERVER] Couldn't connect to SQL Server Database\n");
			}
		}

		std::string GetQueryExecResult(const std::string& query)
		{
			ConnectToDatabase();
			return m_dbconnector.GetResultFromExecuteQuery(query);
		}
		std::string GetQueryExecRowsetResult(const std::string& query,int columnStart=1, int columnEnd=1)
		{
			ConnectToDatabase();
			return m_dbconnector.GetResultFromRowsetExecuteQuery(query,columnStart,columnEnd);
		}

		void ExecQuery(const std::string& query)
		{
			/*if (!m_dbconnector.isConnected())
			{*/
			ConnectToDatabase();
			std::cout << "\n[SERVER] Query Result:\n\n\n";
			m_dbconnector.ExecuteQuery(query); // throws Query exception!
			std::cout << "\n";
		}


		// ASYNC - Instruct asio to wait for connection
		void WaitForClientConnection()
		{
			// Prime context with an instruction to wait until a socket connects. This
			// is the purpose of an "acceptor" object. It will provide a unique socket
			// for each incoming connection attempt
			m_asioAcceptor.async_accept(
				[this](std::error_code ec, asio::ip::tcp::socket socket)
				{
					// Triggered by incoming connection request
					if (!ec)
					{
						// Display some useful(?) information
						std::cout << "[SERVER] New Connection: " << socket.remote_endpoint() << "\n";

						// Create a new connection to handle this client 
						std::shared_ptr<connection<T>> newconn =
							std::make_shared<connection<T>>(connection<T>::owner::server,
								m_asioContext, std::move(socket), m_qMessagesIn);



						// Give the user server a chance to deny connection
						if (OnClientConnect(newconn))
						{
							// Connection allowed, so add to container of new connections
							m_deqConnections.push_back(std::move(newconn));

							// And very important! Issue a task to the connection's
							// asio context to sit and wait for bytes to arrive!
							m_deqConnections.back()->ConnectToClient(this, nIDCounter++);

							std::cout << "[" << m_deqConnections.back()->GetID() << "] Connection Approved\n";
						}
						else
						{
							std::cout << "[-----] Connection Denied\n";

							// Connection will go out of scope with no pending tasks, so will
							// get destroyed automagically due to the wonder of smart pointers
						}
					}
					else
					{
						// Error has occurred during acceptance
						std::cout << "[SERVER] New Connection Error: " << ec.message() << "\n";
					}

					// Prime the asio context with more work - again simply wait for
					// another connection...
					WaitForClientConnection();
				});
		}

		// Send a message to a specific client
		void MessageClient(std::shared_ptr<connection<T>> client, const message<T>& msg)
		{
			// Check client is legitimate...
			if (client && client->IsConnected())
			{
				// ...and post the message via the connection
				client->Send(msg);
			}
			else
			{
				// If we cant communicate with client then we may as 
				// well remove the client - let the server know, it may
				// be tracking it somehow
				OnClientDisconnect(client);

				// Off you go now, bye bye!
				client.reset();

				// Then physically remove it from the container
				m_deqConnections.erase(
					std::remove(m_deqConnections.begin(), m_deqConnections.end(), client), m_deqConnections.end());
			}
		}

		// Send message to all clients
		void MessageAllClients(const message<T>& msg, std::shared_ptr<connection<T>> pIgnoreClient = nullptr)
		{
			bool bInvalidClientExists = false;

			// Iterate through all clients in container
			for (auto& client : m_deqConnections)
			{
				// Check client is connected...
				if (client && client->IsConnected())
				{
					// ..it is!
					if (client != pIgnoreClient)
						client->Send(msg);
				}
				else
				{
					// The client couldnt be contacted, so assume it has
					// disconnected.
					OnClientDisconnect(client);
					client.reset();

					// Set this flag to then remove dead clients from container
					bInvalidClientExists = true;
				}
			}

			// Remove dead clients, all in one go - this way, we dont invalidate the
			// container as we iterated through it.
			if (bInvalidClientExists)
				m_deqConnections.erase(
					std::remove(m_deqConnections.begin(), m_deqConnections.end(), nullptr), m_deqConnections.end());
		}

		// Force server to respond to incoming messages
		void Update(size_t nMaxMessages = -1, bool bWait = false)
		{
			if (bWait) m_qMessagesIn.wait();

			// Process as many messages as you can up to the value
			// specified
			size_t nMessageCount = 0;
			while (nMessageCount < nMaxMessages && !m_qMessagesIn.empty())
			{
				// Grab the front message
				auto msg = m_qMessagesIn.pop_front();

				// Pass to message handler
				OnMessage(msg.remote, msg.msg);

				nMessageCount++;
			}
		}

	public:
		// called when a client is validated

		virtual void OnClientValidated(std::shared_ptr<connection<T>> client)
		{

		}

	protected:
		// This server class should override thse functions to implement
		// customised functionality

		// Called when a client connects, you can veto the connection by returning false
		virtual bool OnClientConnect(std::shared_ptr<connection<T>> client)
		{
			return false;
		}

		// Called when a client appears to have disconnected
		virtual void OnClientDisconnect(std::shared_ptr<connection<T>> client)
		{

		}

		// Called when a message arrives
		virtual void OnMessage(std::shared_ptr<connection<T>> client, message<T>& msg)
		{

		}
		private:
			bool didntSendTodayNotification;
			std::string getToDaysDate()
			{
				std::string dateX = ""; // DD.MM.YYYY
				std::time_t t = std::time(0); // get time now.
				std::tm* now = std::localtime(&t);
				int currDay = now->tm_mday;
				std::string date = "";
				if (currDay < 10)
				{
					dateX += "0";
				}
				dateX += std::to_string(currDay);
				dateX += ".";
				int currMonth = (now->tm_mon + 1);
				if (currMonth < 10)
				{
					dateX += "0";
				}
				dateX += std::to_string(currMonth);
				dateX += ".";
				dateX += std::to_string(1900 + now->tm_year);
				
				return dateX;
			}
		public:
		void SendDailyNotification()
		{

			std::map<std::string, std::pair<std::string, std::string>> emailMaps; // email, {income,outcome}.
			std::string queryGetEmailNIncome = "SELECT U.Email, SUM(T.Valoare) AS TotalIncomeToday FROM DailyMails AS DM JOIN Users as U ON DM.UserID = U.UserID JOIN Tranzactions AS T ON DM.UserID = T.UserID WHERE T.TypeTranzaction = '1' AND CONVERT(date, T.Timestamp) = CONVERT(date, GETDATE()) GROUP BY T.UserID, U.Email";
			std::string resultsEmailIncome = GetQueryExecRowsetResult(queryGetEmailNIncome, 1, 2);
			std::stringstream ss(resultsEmailIncome);
			std::string row;
			while (std::getline(ss, row, '\n'))
			{
				std::stringstream ssfields(row);
				std::string emailTo;
				std::string income;

				std::getline(ssfields, emailTo, ';');
				std::getline(ssfields, income, ';');
				
				std::pair<std::string, std::string> income_outcome(income, "0");
				emailMaps.insert(std::pair<std::string, std::pair<std::string, std::string>>(emailTo, income_outcome));
			}


			std::string queryGetEmailNSpendings = "SELECT U.Email, SUM(T.Valoare) AS TotalSpendingToday FROM DailyMails AS DM JOIN Users as U ON DM.UserID = U.UserID JOIN Tranzactions AS T ON DM.UserID = T.UserID WHERE T.TypeTranzaction = '2' AND CONVERT(date, T.Timestamp) = CONVERT(date, GETDATE()) GROUP BY T.UserID, U.Email";
			std::string resultsEmailSpendings = GetQueryExecRowsetResult(queryGetEmailNSpendings, 1, 2);
			std::stringstream ss2(resultsEmailSpendings);
			while (std::getline(ss2, row, '\n'))
			{
				std::stringstream ssfields(row);
				std::string emailTo;
				std::string outcome;

				std::getline(ssfields, emailTo, ';');
				std::getline(ssfields, outcome, ';');

				if (emailMaps.find(emailTo) != emailMaps.end())
				{
					emailMaps[emailTo].second = outcome;
				}
				else
				{
					std::pair<std::string, std::string> income_outcome("0", outcome);
					emailMaps.insert(std::pair<std::string, std::pair<std::string, std::string>>(emailTo, income_outcome));
				}
			}
			// do send the email to those users.
			for (std::map<std::string, std::pair<std::string, std::string>>::iterator iter = emailMaps.begin(); iter != emailMaps.end(); iter++)
			{
				std::string msg = "Hello!\nHere's the daily summary for your tranzactions: \n";
				msg += "Income: "; msg += iter->second.first;
				msg += "\nSpendings: "; msg += iter->second.second;
				msg += "\n\nAll the best,\nCleverPocket developers";
				SendSMTPEmail(iter->first, msg);
			}
			FILE* fout = fopen("ServerSentEmails.txt", "w");
			std::string date = getToDaysDate();
			fputs(date.c_str(), fout);
			fclose(fout);
		}
		bool checkIfUptimeToSendDailyMails()
		{
			// 1. check if it's minimum 20:00:00
			std::time_t t = std::time(0); // get time now.
			std::tm* now = std::localtime(&t);
			if (now->tm_hour < 20)
			{
				return false;
			}
			if (didntSendTodayNotification)
			{
				int currDay = now->tm_mday;
				std::string date = "";
				if (currDay < 10)
				{
					date += "0";
				}
				date += std::to_string(currDay);
				date += ".";
				int currMonth = (now->tm_mon + 1);
				if (currMonth < 10)
				{
					date += "0";
				}
				date += std::to_string(currMonth);
				date += ".";
				date += std::to_string(1900 + now->tm_year);

				// 2. check if already sent that day.
				FILE* fin = fopen("ServerSentEmails.txt", "r");
				if (fin)
				{
					char buff[256]; fgets(buff, sizeof(buff), fin);
					if (strcmp(buff, date.c_str()) == 0)
					{
						std::cout << "[SERVER]: Already Sent Daily Emails to users that requested!\n";
						didntSendTodayNotification = false;
						return false;
					}
					fclose(fin);
				}
				return true;
			}
			return false;
		}
		void SendSMTPEmail(const std::string& emailTo, const std::string& body)
		{
			::CoInitialize(NULL); //?
			IMailPtr oSMTP = NULL;
			oSMTP.CreateInstance(__uuidof(EASendMailObjLib::Mail));
			oSMTP->LicenseCode = _T("TryIt");

			// set from gmail address
			oSMTP->FromAddr = _T("steelparrot.inc@gmail.com");

			// to gmail address.
			oSMTP->AddRecipientEx(_bstr_t(emailTo.c_str()), 0);

			// set email subject.
			oSMTP->Subject = _T("CleverPocket Daily Summary");

			// set email body.
			//std::string msg = "Hello!\nLooks like you requested us to update your password, since you forgot it. Here's the validation code: " + valid_code + "\n\nAll the best,\nCleverPocket developers";
			oSMTP->BodyText = _bstr_t(body.c_str()); // generate random code, save local to server in a file, and read it then delete.


			// gmail SMTP Server Address.
			oSMTP->ServerAddr = _T("smtp.gmail.com");

			// gmail user authentication should use your gmail address as username
			// extract secured from file or smt. TO DO!!
			oSMTP->UserName = _T("steelparrot.inc@gmail.com");
			oSMTP->Password = _T("papagaluu1");

			// set port 25 or 587.
			oSMTP->ServerPort = 587;

			// detect SSL/TLS automatically
			oSMTP->ConnectType = (long)ConnectSMTPCodes::ConnectSSLAuto;

			if (oSMTP->SendMail() == 0)
			{
				std::cout << "[SERVER]: email was sent successfully!\r\n";
			}
			else
			{
				std::cout << "[SERVER]: failed to send email with the following error:\r\n" << (const TCHAR*)oSMTP->GetLastErrDescription();
			}
		}
		void OnAddPreferencesUsername(char username[], char dailyMail[], char currencyISO[])
		{
			std::string l_username = convertToSqlVarcharFormat(username);
			std::string l_dailyMail = convertToSqlVarcharFormat(dailyMail);
			std::string l_currencyISO = convertToSqlVarcharFormat(currencyISO);
			std::string userIDQuery = "SELECT UserID FROM CleverPocket.dbo.Users WHERE Username = " + l_username;
			std::string resultID = GetQueryExecResult(userIDQuery);
			if (resultID == "")
			{
				throw UsernameInvalidLoginError("We couldn't find any user with this Username... operation down");
			}
			resultID = convertToSqlVarcharFormat(resultID.c_str());
			std::string query = "UPDATE [CleverPocket].[dbo].[Numerar] SET  CurrencyISO = " + l_currencyISO + "WHERE UserID = " + resultID;
			ExecQuery(query);
			//check mail state of this user
			if (strcmp(dailyMail, "True")==0)
			{
				query = "SELECT UserID FROM [CleverPocket].[dbo].[DailyMails]";
				std::string exists = GetQueryExecResult(query);
				if (exists != "")
				{
					throw AlreadyCheckedForDailyNotification("Already check for daily notification!");
				}
				query = "INSERT INTO  [CleverPocket].[dbo].[DailyMails](UserID, SendMail) VALUES( " + resultID + " , " + l_dailyMail + ")";
			}
			else
			{
				query = "DELETE FROM  [CleverPocket].[dbo].[DailyMails] WHERE UserID = " + resultID;
			}
			ExecQuery(query);
		}

		void OnAddPicturePAT(char pat[], char hexImg[])
		{
			std::string l_pat = convertToSqlVarcharFormat(pat);
			std::string l_hexImg = convertToSqlVarcharFormat(hexImg);
			std::string userIDQuery = "SELECT UserID FROM CleverPocket.dbo.Sessions WHERE PAT = " + l_pat;
			std::string resultID = GetQueryExecResult(userIDQuery);
			if (resultID == "")
			{
				throw InvalidPATLoginError("We couldn't find any user with this PAT... operation down");
			}
			resultID = convertToSqlVarcharFormat(resultID.c_str());
			std::string query = "UPDATE [CleverPocket].[dbo].[Users] SET Picture = " + l_hexImg + " WHERE UserID = " + resultID;
			ExecQuery(query);
		}

		void OnAddPictureUsername(char username[], char hexImg[])
		{
			std::string l_username = convertToSqlVarcharFormat(username);
			std::string l_hexImg = convertToSqlVarcharFormat(hexImg);
			std::string userIDQuery = "SELECT UserID FROM CleverPocket.dbo.Users WHERE Username = " + l_username;
			std::string resultID = GetQueryExecResult(userIDQuery);
			if (resultID == "")
			{
				throw UsernameInvalidLoginError("We couldn't find any user with this Username... operation down");
			}
			resultID = convertToSqlVarcharFormat(resultID.c_str());
			std::string query = "UPDATE [CleverPocket].[dbo].[Users] SET Picture = " + l_hexImg + " WHERE UserID = " + resultID;
			ExecQuery(query);
		}

		void OnAddPreferencesUsername(char username[], char dailyMail[], char currencyISO[])
		{
			std::string l_username = convertToSqlVarcharFormat(username);
			std::string l_dailyMail = convertToSqlVarcharFormat(dailyMail);
			std::string l_currencyISO = convertToSqlVarcharFormat(currencyISO);
			std::string userIDQuery = "SELECT UserID FROM CleverPocket.dbo.Sessions WHERE PAT = " + l_username;
			std::string resultID = GetQueryExecResult(userIDQuery);
			if (resultID == "")
			{
				throw InvalidPATLoginError("We couldn't find any user with this PAT... operation down");
			}
			resultID = convertToSqlVarcharFormat(resultID.c_str());
			std::string query = "UPDATE [CleverPocket].[dbo].[Numerar] SET  CurrencyISO = " + l_currencyISO + "WHERE UserID = " + resultID;
			ExecQuery(query);

			//check mail state of this user
			if (dailyMail == "True")
			{
				query = "INSERT INTO  [CleverPocket].[dbo].[DailyMails](UserID, SendMail) VALUES( " + resultID + " , " + l_dailyMail + ")";
			}
			else
			{
				query = "DELETE FROM  [CleverPocket].[dbo].[DailyMails] WHERE UserID = " + resultID;
			}
			ExecQuery(query);
		}

		void OnAddPicturePAT(char pat[], char hexImg[])
		{
			std::string l_pat = convertToSqlVarcharFormat(pat);
			std::string l_hexImg = convertToSqlVarcharFormat(hexImg);
			std::string userIDQuery = "SELECT UserID FROM CleverPocket.dbo.Sessions WHERE PAT = " + l_pat;
			std::string resultID = GetQueryExecResult(userIDQuery);
			if (resultID == "")
			{
				throw InvalidPATLoginError("We couldn't find any user with this PAT... operation down");
			}
			resultID = convertToSqlVarcharFormat(resultID.c_str());
			std::string query = "UPDATE [CleverPocket].[dbo].[Users] SET Picture = " + l_hexImg + " WHERE UserID = " + resultID;
			ExecQuery(query);
		}

		void OnAddPictureUsername(char username[], char hexImg[])
		{
			std::string l_username = convertToSqlVarcharFormat(username);
			std::string l_hexImg = convertToSqlVarcharFormat(hexImg);
			std::string userIDQuery = "SELECT UserID FROM CleverPocket.dbo.Sessions WHERE PAT = " + l_username;
			std::string resultID = GetQueryExecResult(userIDQuery);
			if (resultID == "")
			{
				throw InvalidPATLoginError("We couldn't find any user with this PAT... operation down");
			}
			resultID = convertToSqlVarcharFormat(resultID.c_str());
			std::string query = "UPDATE [CleverPocket].[dbo].[Users] SET Picture = " + l_hexImg + " WHERE UserID = " + resultID;
			ExecQuery(query);
		}

		void OnAddCashPAT(char pat[], char cashValue[], char cardname[])
		{
			std::string l_pat = convertToSqlVarcharFormat(pat);
			std::string l_cashValue = convertToSqlVarcharFormat(cashValue);
			std::string l_cardname = convertToSqlVarcharFormat(cardname);
			std::string userIDQuery = "SELECT UserID FROM CleverPocket.dbo.Sessions WHERE PAT = " + l_pat;
			std::string resultID = GetQueryExecResult(userIDQuery);
			if (resultID == "")
			{
				throw InvalidPATLoginError("We couldn't find any user with this PAT... operation down");
			}
			resultID = convertToSqlVarcharFormat(resultID.c_str());
			std::string query = "UPDATE [CleverPocket].[dbo].[Numerar] SET SoldNumerar += " + l_cashValue + " WHERE UserID = " + resultID;
			ExecQuery(query);
			query = "UPDATE [CleverPocket].[dbo].[Cards] SET Sold -= " + l_cashValue + " WHERE UserID = " + resultID + " AND CardName = " + l_cardname;
			ExecQuery(query);
		}
		void OnAddCashUsername(char username[], char cashValue[], char cardname[])
		{
			std::string l_username = convertToSqlVarcharFormat(username);
			std::string l_cashValue = convertToSqlVarcharFormat(cashValue);
			std::string l_cardname = convertToSqlVarcharFormat(cardname);
			std::string userIDQuery = "SELECT UserID FROM CleverPocket.dbo.Users WHERE Username = " + l_username;
			std::string resultID = GetQueryExecResult(userIDQuery);
			if (resultID == "")
			{
				throw UsernameInvalidLoginError("We couldn't find any user with this username... operation down");
			}
			resultID = convertToSqlVarcharFormat(resultID.c_str());
			std::string query = "UPDATE [CleverPocket].[dbo].[Numerar] SET SoldNumerar += " + l_cashValue + " WHERE UserID = " + resultID;
			ExecQuery(query);
			query = "UPDATE [CleverPocket].[dbo].[Cards] SET Sold -= " + l_cashValue + " WHERE UserID = " + resultID + " AND CardName = " + l_cardname;
			ExecQuery(query);
		}
		void OnGetCashUsername(char username[], std::string& cashValue, std::string& cashCurrencyISO, clever::CredentialHandler& userInfo, std::string& mailState)
		{
			std::string l_username = convertToSqlVarcharFormat(username);
			std::string userIDQuery = "SELECT UserID FROM CleverPocket.dbo.Users WHERE Username = " + l_username;
			std::string resultID = GetQueryExecResult(userIDQuery);
			if (resultID == "")
			{
				throw UsernameInvalidLoginError("We couldn't find any user with this username... operation down");
			}
			std::string userID = convertToSqlVarcharFormat(resultID.c_str());

			std::string query = "SELECT SoldNumerar FROM CleverPocket.dbo.Numerar WHERE UserID = " + userID;
			cashValue = GetQueryExecResult(query);
			query = "SELECT CurrencyISO FROM CleverPocket.dbo.Numerar WHERE UserID = " + userID;
			cashCurrencyISO = GetQueryExecResult(query);
			std::string queryGetAllUserInformation = "SELECT [FirstName], [LastName], [Username], [Email], [Country], [PhoneNumber], [Picture] FROM CleverPocket.dbo.Users WHERE  UserID = " + userID;
			std::string resultQueryGetAllUserInfoCards = GetQueryExecRowsetResult(queryGetAllUserInformation, 1, 7);

			std::stringstream ss(resultQueryGetAllUserInfoCards);

			std::string firstName;
			std::string lastName;
			std::string userName;
			std::string email;
			std::string country;
			std::string phoneNumber;
			std::string picture;
			std::getline(ss, firstName, ';'); userInfo.setFirstName(firstName);
			std::getline(ss, lastName, ';'); userInfo.setLastName(lastName);
			std::getline(ss, userName, ';'); userInfo.setUsername(userName);
			std::getline(ss, email, ';'); userInfo.setEmail(email);
			std::getline(ss, country, ';'); userInfo.setCountryID(country);
			std::getline(ss, phoneNumber, ';'); userInfo.setPhoneNumber(phoneNumber);
			std::getline(ss, picture, ';'); userInfo.setPassword(picture);


			std::string queryMail = "SELECT [SendMail] FROM CleverPocket.dbo.DailyMails WHERE UserID = " + resultID;
			std::string resultQueryMail = GetQueryExecResult(queryMail);
			if (resultQueryMail == "")
			{
				mailState = "False";
			}
			else
			{
				mailState = "True";
			}
		}
		void OnGetCashPAT(char pat[], std::string& cashValue, std::string& cashCurrencyISO, std::string& username, clever::CredentialHandler& userInfo, std::string& mailState)
		{
			std::string l_pat = convertToSqlVarcharFormat(pat);
			std::string userIDQuery = "SELECT UserID FROM CleverPocket.dbo.Sessions WHERE PAT = " + l_pat;
			std::string resultID = GetQueryExecResult(userIDQuery);
			if (resultID == "")
			{
				throw InvalidPATLoginError("We couldn't find any user with this PAT... operation down");
			}
			std::string userID = convertToSqlVarcharFormat(resultID.c_str());
			std::string query = "SELECT SoldNumerar FROM CleverPocket.dbo.Numerar WHERE UserID = " + userID;
			cashValue = GetQueryExecResult(query);
			query = "SELECT CurrencyISO FROM CleverPocket.dbo.Numerar WHERE UserID = " + userID;
			cashCurrencyISO = GetQueryExecResult(query);
			query = "SELECT Username FROM CleverPocket.dbo.Users WHERE UserID = " + userID;
			username = GetQueryExecResult(query);

			std::string queryGetAllUserInformation = "SELECT [FirstName], [LastName], [Username], [Email], [Country], [PhoneNumber], [Picture] FROM CleverPocket.dbo.Users WHERE  UserID = " + userID;
			std::string resultQueryGetAllUserInfoCards = GetQueryExecRowsetResult(queryGetAllUserInformation, 1, 7);

			std::stringstream ss(resultQueryGetAllUserInfoCards);

			std::string firstName;
			std::string lastName;
			std::string userName;
			std::string email;
			std::string country;
			std::string phoneNumber;
			std::string picture;
			std::getline(ss, firstName, ';'); userInfo.setFirstName(firstName);
			std::getline(ss, lastName, ';'); userInfo.setLastName(lastName);
			std::getline(ss, userName, ';'); userInfo.setUsername(userName);
			std::getline(ss, email, ';'); userInfo.setEmail(email);
			std::getline(ss, country, ';'); userInfo.setCountryID(country);
			std::getline(ss, phoneNumber, ';'); userInfo.setPhoneNumber(phoneNumber);
			std::getline(ss, picture, ';'); userInfo.setPassword(picture);


			std::string queryMail = "SELECT [SendMail] FROM CleverPocket.dbo.DailyMails WHERE UserID = " + resultID;
			std::string resultQueryMail = GetQueryExecResult(queryMail);
			if (resultQueryMail == "")
			{
				mailState = "False";
			}
			else
			{
				mailState = "True";
			}
		}
		void OnAddCardFundsPAT(char pat[], char cardFunds[], char cardName[])
		{
			std::string l_pat = convertToSqlVarcharFormat(pat);
			std::string l_cardFunds = convertToSqlVarcharFormat(cardFunds);
			std::string l_cardName = convertToSqlVarcharFormat(cardName);
			std::string userIDQuery = "SELECT UserID FROM CleverPocket.dbo.Sessions WHERE PAT = " + l_pat;
			std::string resultID = GetQueryExecResult(userIDQuery);
			if (resultID == "")
			{
				throw InvalidPATLoginError("We couldn't find any user with this PAT... operation down");
			}
			resultID = convertToSqlVarcharFormat(resultID.c_str());
			std::string query = "UPDATE [CleverPocket].[dbo].[Cards] SET Sold += " + l_cardFunds + " WHERE CardName = " + l_cardName+" AND UserID = "+resultID;
			ExecQuery(query);
		}

		void OnAddCardFundsUsername(char username[], char cardFunds[], char cardName[])
		{
			std::string l_username = convertToSqlVarcharFormat(username);
			std::string l_cardFunds = convertToSqlVarcharFormat(cardFunds);
			std::string l_cardName = convertToSqlVarcharFormat(cardName);
			std::string userIDQuery = "SELECT UserID FROM CleverPocket.dbo.Users WHERE Username = " + l_username;
			std::string resultID = GetQueryExecResult(userIDQuery);
			if (resultID == "")
			{
				throw UsernameInvalidLoginError("We couldn't find any user with this username... operation down");
			}
			resultID = convertToSqlVarcharFormat(resultID.c_str());
			std::string query = "UPDATE [CleverPocket].[dbo].[Cards] SET Sold += " + l_cardFunds + " WHERE CardName = " + l_cardName + " AND UserID = " + resultID;
			ExecQuery(query);
		}
		void OnGetTranzactionsUsername(char username[], std::vector<clever::TranzactionHandler>& tranzactions)
		{
			std::string l_username = convertToSqlVarcharFormat(username);
			std::string userIDQuery = "SELECT UserID FROM CleverPocket.dbo.Users WHERE Username = " + l_username;
			std::string resultUserID = GetQueryExecResult(userIDQuery);
			if (resultUserID == "")
			{
				throw UsernameInvalidLoginError("We couldn't find any user with this username... operation down");
			}

			std::string queryGetAllTranzactionsUser = "SELECT [Source], [Destination], [Timestamp], [FinanceName], [TypeTranzaction], [Valoare], [CurrencyISO], [DescriereTranzactie], [CategoryName], [TranzactionTitle] FROM [CleverPocket].[dbo].[Tranzactions] WHERE UserID = " + resultUserID;
			std::string resultQueryGetTranzactions = GetQueryExecRowsetResult(queryGetAllTranzactionsUser, 1, 10);
			std::cout << resultQueryGetTranzactions;

			std::stringstream ss(resultQueryGetTranzactions);
			std::string row;
			while (std::getline(ss, row, '\n'))
			{
				std::stringstream ssfields(row);
				std::string tranzactionSource;
				std::string tranzactionDestination;
				std::string tranzactionTimestamp;
				std::string tranzactionFinanceName;
				std::string tranzactionType; // clever::TranzactionsTypes -- unsigned int
				std::string tranzactionValoare; // float
				std::string tranzactionCurrencyISO;
				std::string tranzactionDescriere;
				std::string tranzactionCategoryName;
				std::string tranzactionTitle;

				std::getline(ssfields, tranzactionSource, ';');
				std::getline(ssfields, tranzactionDestination, ';');
				std::getline(ssfields, tranzactionTimestamp, ';');
				std::getline(ssfields, tranzactionFinanceName, ';');
				std::getline(ssfields, tranzactionType, ';');
				std::getline(ssfields, tranzactionValoare, ';');
				std::getline(ssfields, tranzactionCurrencyISO, ';');
				std::getline(ssfields, tranzactionDescriere, ';');
				std::getline(ssfields, tranzactionCategoryName, ';');
				std::getline(ssfields, tranzactionTitle, ';');

				TranzactionType trType = (tranzactionType == "1") ? TranzactionType::Income : TranzactionType::Spending;
				tranzactions.push_back(clever::TranzactionHandler(tranzactionTitle,
					tranzactionSource,
					tranzactionDestination,
					tranzactionTimestamp,
					tranzactionFinanceName,
					trType,
					std::stof(tranzactionValoare),
					tranzactionCurrencyISO,
					tranzactionDescriere,
					tranzactionCategoryName
				));
			}
		}
		void OnGetRecurenciesUsername(char username[], std::vector<clever::FinanceTypeCredentialHandler>& incomes)
		{
			std::string l_username = convertToSqlVarcharFormat(username);
			std::string userIDQuery = "SELECT UserID FROM CleverPocket.dbo.Users WHERE Username = " + l_username;
			std::string resultUserID = GetQueryExecResult(userIDQuery);
			if (resultUserID == "")
			{
				throw UsernameInvalidLoginError("We couldn't find any user with this username... operation down");
			}

			std::string queryGetAllCardsUser = "SELECT [RecurenciesName], [RecurenciesReceiver], [RecurenciesValue], [RecurenciesCard], [RecurenciesISO], [DayOfMonth], [RecurenciesTypeID] FROM [CleverPocket].[dbo].[Recurencies] WHERE dbo.Cards.UserID = " + resultUserID;
			// will contain all data separated each row with \n.
			std::string resultQueryGetCards = GetQueryExecRowsetResult(queryGetAllCardsUser, 1, 7);
			//std::cout << resultQueryGetCards;

			std::stringstream ss(resultQueryGetCards);
			std::string row;
			while (std::getline(ss, row, '\n'))
			{
				std::stringstream ssfields(row);
				std::string recurenciesName;
				std::string recurenciesReceiver;
				std::string recurenciesValue;
				std::string recurenciesCard;
				std::string recurenciesISO;
				std::string dayOfMonth;
				std::string recurenciesTypeID;

				std::getline(ssfields, recurenciesName, ';');
				std::getline(ssfields, recurenciesReceiver, ';');
				std::getline(ssfields, recurenciesValue, ';');
				std::getline(ssfields, recurenciesCard, ';');
				std::getline(ssfields, recurenciesISO, ';');
				std::getline(ssfields, dayOfMonth, ';');
				std::getline(ssfields, recurenciesTypeID, ';');


				incomes.push_back(clever::FinanceTypeCredentialHandler(recurenciesName, recurenciesReceiver, recurenciesISO, dayOfMonth, recurenciesCard, std::stof(recurenciesValue), recurenciesTypeID));
			}
		}
		void OnGetCardsUsername(char username[], std::vector<clever::CardCredentialHandler>& cards)
		{
			std::string l_username = convertToSqlVarcharFormat(username);
			std::string userIDQuery = "SELECT UserID FROM CleverPocket.dbo.Users WHERE Username = " + l_username;
			std::string resultUserID = GetQueryExecResult(userIDQuery);
			if (resultUserID == "")
			{
				throw UsernameInvalidLoginError("We couldn't find any user with this username... operation down");
			}

			std::string queryGetAllCardsUser = "SELECT [CardName],[CardHolder],[CardNumber],[ValidUntil],[CurrencyISO],[Sold] FROM [CleverPocket].[dbo].[Cards] WHERE dbo.Cards.UserID = " + resultUserID;
			// will contain all data separated each row with \n.
			std::string resultQueryGetCards = GetQueryExecRowsetResult(queryGetAllCardsUser, 1, 6);
			//std::cout << resultQueryGetCards;

			std::stringstream ss(resultQueryGetCards);
			std::string row;
			while (std::getline(ss, row, '\n'))
			{
				std::stringstream ssfields(row);
				std::string cardName;
				std::string cardHolder;
				std::string cardNumber;
				std::string cardValidUntil;
				std::string cardCurrencyISO;
				std::string cardSold;

				std::getline(ssfields, cardName, ';');
				std::getline(ssfields, cardHolder, ';');
				std::getline(ssfields, cardNumber, ';');
				std::getline(ssfields, cardValidUntil, ';');
				std::getline(ssfields, cardCurrencyISO, ';');
				std::getline(ssfields, cardSold, ';');

				cards.push_back(clever::CardCredentialHandler(cardName, cardHolder, cardNumber, cardCurrencyISO, cardValidUntil, std::stof(cardSold)));
			}
		}
		void OnGetCardsPAT(char pat[], std::vector<clever::CardCredentialHandler>& cards)
		{
			std::string l_pat = convertToSqlVarcharFormat(pat);
			std::string userIDQuery = "SELECT UserID FROM CleverPocket.dbo.Sessions WHERE PAT = " + l_pat;
			std::string resultUserID = GetQueryExecResult(userIDQuery);
			if (resultUserID == "")
			{
				throw InvalidPATLoginError("We couldn't find any user with this PAT... operation down");
			}

			std::string queryGetAllCardsUser = "SELECT [CardName],[CardHolder],[CardNumber],[ValidUntil],[CurrencyISO],[Sold] FROM [CleverPocket].[dbo].[Cards] WHERE dbo.Cards.UserID = " + resultUserID;
			// will contain all data separated each row with \n.
			std::string resultQueryGetCards = GetQueryExecRowsetResult(queryGetAllCardsUser,1,6); 
			//std::cout << resultQueryGetCards;

			std::stringstream ss(resultQueryGetCards);
			std::string row;
			while (std::getline(ss, row, '\n'))
			{
				std::stringstream ssfields(row);
				std::string cardName;
				std::string cardHolder;
				std::string cardNumber;
				std::string cardValidUntil;
				std::string cardCurrencyISO;
				std::string cardSold;

				std::getline(ssfields, cardName, ';');
				std::getline(ssfields, cardHolder, ';');
				std::getline(ssfields, cardNumber, ';');
				std::getline(ssfields, cardValidUntil, ';');
				std::getline(ssfields, cardCurrencyISO, ';');
				std::getline(ssfields, cardSold, ';');

				cards.push_back(clever::CardCredentialHandler(cardName, cardHolder, cardNumber, cardCurrencyISO, cardValidUntil,std::stof(cardSold)));
			}
		}
		void OnAddIncomeUsername(char username[], const clever::FinanceTypeCredentialHandler& incomeCredHandler)
		{
			// obtain the user ID from this username and insert in Cards with this user ID.
			std::string l_username = convertToSqlVarcharFormat(username);
			std::string userIDQuery = "SELECT UserID FROM CleverPocket.dbo.Users WHERE Username = " + l_username;
			std::string resultID = GetQueryExecResult(userIDQuery);
			if (resultID == "")
			{
				throw UsernameInvalidLoginError("We couldn't find any user with this username... operation down");
			}
			std::string userID = convertToSqlVarcharFormat(resultID.c_str());
			std::string incomeName = convertToSqlVarcharFormat(incomeCredHandler.getFinanceTypeName());
			std::string incomeSource = convertToSqlVarcharFormat(incomeCredHandler.getFinanceTypeSource());
			std::string incomeCurrencyISO = convertToSqlVarcharFormat(incomeCredHandler.getFinanceTypeCurrencyISO());
			std::string dayOfIncome = convertToSqlVarcharFormat(incomeCredHandler.getDayOfFinanceType());
			std::string incomeToCard = convertToSqlVarcharFormat(incomeCredHandler.getFinanceTypeToCard());
			std::string incomeValue = convertToSqlVarcharFormat(std::string(incomeCredHandler.getFinanceTypeValue()).c_str());

			std::string query = "INSERT INTO [CleverPocket].[dbo].[Recurencies](UserId, RecurenciesName, RecurenciesReceiver, RecurenciesValue, RecurenciesCard, RecurenciesISO, DayOfMonth, RecurenciesTypeID) VALUES (" + userID + "," + incomeName + "," + incomeSource + "," + incomeValue + "," + incomeToCard + "," + incomeCurrencyISO + "," + dayOfIncome + ", 1)";
			ExecQuery(query);
		}
		void OnAddOutcomeUsername(char username[], const clever::FinanceTypeCredentialHandler& outcomeCredHandler)
		{
			// obtain the user ID from this username and insert in Cards with this user ID.
			std::string l_username = convertToSqlVarcharFormat(username);
			std::string userIDQuery = "SELECT UserID FROM CleverPocket.dbo.Users WHERE Username = " + l_username;
			std::string resultID = GetQueryExecResult(userIDQuery);
			if (resultID == "")
			{
				throw UsernameInvalidLoginError("We couldn't find any user with this username... operation down");
			}
			std::string userID = convertToSqlVarcharFormat(resultID.c_str());
			std::string oucomeName = convertToSqlVarcharFormat(outcomeCredHandler.getFinanceTypeName());
			std::string outcomeSource = convertToSqlVarcharFormat(outcomeCredHandler.getFinanceTypeSource());
			std::string outcomeCurrencyISO = convertToSqlVarcharFormat(outcomeCredHandler.getFinanceTypeCurrencyISO());
			std::string dayOfOutcome = convertToSqlVarcharFormat(outcomeCredHandler.getDayOfFinanceType());
			std::string outcomeToCard = convertToSqlVarcharFormat(outcomeCredHandler.getFinanceTypeToCard());
			std::string outcomeValue = convertToSqlVarcharFormat(std::string(outcomeCredHandler.getFinanceTypeValue()).c_str());

			std::string query = "INSERT INTO [CleverPocket].[dbo].[Recurencies](UserId, RecurenciesName, RecurenciesReceiver, RecurenciesValue, RecurenciesCard, RecurenciesISO, DayOfMonth, RecurenciesTypeID) VALUES (" + userID + "," + incomeName + "," + incomeSource + "," + incomeValue + "," + incomeToCard + "," + incomeCurrencyISO + "," + dayOfIncome + ", 2)";
			ExecQuery(query);
		}
		void OnAddCardUsername(char username[], const clever::CardCredentialHandler& cardCredHandler)
		{
			// obtain the user ID from this username and insert in Cards with this user ID.
			std::string l_username = convertToSqlVarcharFormat(username);
			std::string userIDQuery = "SELECT UserID FROM CleverPocket.dbo.Users WHERE Username = " + l_username;
			std::string resultID = GetQueryExecResult(userIDQuery);
			if (resultID == "")
			{
				throw UsernameInvalidLoginError("We couldn't find any user with this username... operation down");
			}
			std::string userID = convertToSqlVarcharFormat(resultID.c_str());
			std::string cardName = convertToSqlVarcharFormat(cardCredHandler.getCardName());
			std::string cardHolder = convertToSqlVarcharFormat(cardCredHandler.getCardHolder());
			std::string cardNumber = convertToSqlVarcharFormat(cardCredHandler.getCardNumber());
			std::string cardCurrencyISO = convertToSqlVarcharFormat(cardCredHandler.getCardCurrencyISO());
			std::string cardValidUntil = convertToSqlVarcharFormat(cardCredHandler.getCardValidUntil());

			std::string query = "INSERT INTO [CleverPocket].[dbo].[Cards] (UserID, CardName, CardHolder, CardNumber, ValidUntil, CurrencyISO) VALUES(" + userID + ", " + cardName + ", " + cardHolder + ", " + cardNumber + ", " + cardValidUntil + ", " + cardCurrencyISO + ")";
			ExecQuery(query);
		}
		void OnAddCardPAT(char pat[], const clever::CardCredentialHandler& cardCredHandler)
		{
			// obtain the user ID from this PAT and insert in Cards table with this user ID.
			std::string l_pat = convertToSqlVarcharFormat(pat);
			std::string userIDQuery = "SELECT UserID FROM CleverPocket.dbo.Sessions WHERE PAT = " + l_pat;
			std::string resultID = GetQueryExecResult(userIDQuery);
			if (resultID == "")
			{
				throw InvalidPATLoginError("We couldn't find any user with this PAT... operation down");
			}
			std::string userID = convertToSqlVarcharFormat(resultID.c_str());
			std::string cardName = convertToSqlVarcharFormat(cardCredHandler.getCardName());
			std::string cardHolder = convertToSqlVarcharFormat(cardCredHandler.getCardHolder());
			std::string cardNumber = convertToSqlVarcharFormat(cardCredHandler.getCardNumber());
			std::string cardCurrencyISO = convertToSqlVarcharFormat(cardCredHandler.getCardCurrencyISO());
			std::string cardValidUntil = convertToSqlVarcharFormat(cardCredHandler.getCardValidUntil());

			std::string query = "INSERT INTO [CleverPocket].[dbo].[Cards] (UserID, CardName, CardHolder, CardNumber, ValidUntil, CurrencyISO) VALUES(" + userID + ", " + cardName + ", " + cardHolder + ", " + cardNumber + ", " + cardValidUntil + ", " + cardCurrencyISO + ")";
			ExecQuery(query);
		}

		void OnEditCardUsername(char username[], const clever::CardCredentialHandler& cardCredHandler, char oldcardname[])
		{
			// obtain the user ID from this username and insert in Cards with this user ID.
			std::string l_username = convertToSqlVarcharFormat(username);
			std::string userIDQuery = "SELECT UserID FROM CleverPocket.dbo.Users WHERE Username = " + l_username;
			std::string resultID = GetQueryExecResult(userIDQuery);
			if (resultID == "")
			{
				throw UsernameInvalidLoginError("We couldn't find any user with this username... operation down");
			}
			std::string userID = convertToSqlVarcharFormat(resultID.c_str());
			std::string cardName = convertToSqlVarcharFormat(cardCredHandler.getCardName());
			std::string cardHolder = convertToSqlVarcharFormat(cardCredHandler.getCardHolder());
			std::string cardNumber = convertToSqlVarcharFormat(cardCredHandler.getCardNumber());
			std::string cardCurrencyISO = convertToSqlVarcharFormat(cardCredHandler.getCardCurrencyISO());
			std::string cardValidUntil = convertToSqlVarcharFormat(cardCredHandler.getCardValidUntil());
			std::string oldcardname2 = convertToSqlVarcharFormat(oldcardname);

			std::string query = "UPDATE [CleverPocket].[dbo].[Cards] SET CardName = " + cardName + ", CardHolder = " + cardHolder + ", CardNumber = " + cardNumber + ", ValidUntil = " + cardValidUntil + ",CurrencyISO = " + cardCurrencyISO + " WHERE UserID = " + userID + " AND CardName = "+oldcardname2;
			ExecQuery(query);
		}

		void OnEditCardPAT(char pat[], const clever::CardCredentialHandler& cardCredHandler, char oldcardname[])
		{
			// obtain the user ID from this PAT and insert in Cards table with this user ID.
			std::string l_pat = convertToSqlVarcharFormat(pat);
			std::string userIDQuery = "SELECT UserID FROM CleverPocket.dbo.Sessions WHERE PAT = " + l_pat;
			std::string resultID = GetQueryExecResult(userIDQuery);
			if (resultID == "")
			{
				throw InvalidPATLoginError("We couldn't find any user with this PAT... operation down");
			}
			std::string userID = convertToSqlVarcharFormat(resultID.c_str());
			std::string cardName = convertToSqlVarcharFormat(cardCredHandler.getCardName());
			std::string cardHolder = convertToSqlVarcharFormat(cardCredHandler.getCardHolder());
			std::string cardNumber = convertToSqlVarcharFormat(cardCredHandler.getCardNumber());
			std::string cardCurrencyISO = convertToSqlVarcharFormat(cardCredHandler.getCardCurrencyISO());
			std::string cardValidUntil = convertToSqlVarcharFormat(cardCredHandler.getCardValidUntil());
			std::string oldcardname2 = convertToSqlVarcharFormat(oldcardname);

			std::string query = "UPDATE [CleverPocket].[dbo].[Cards] SET CardName = " + cardName + ", CardHolder = " + cardHolder + ", CardNumber = " + cardNumber + ", ValidUntil = " + cardValidUntil + ",CurrencyISO = " + cardCurrencyISO + " WHERE UserID = " + userID +" AND CardName = "+oldcardname2;
			ExecQuery(query);
		}

		void OnLogoutRemembered(char pat[])
		{
			std::string PAT = convertToSqlVarcharFormat(pat);
			std::string testPATExistenceQuery = "IF EXISTS(SELECT 1 FROM CleverPocket.dbo.Sessions WHERE PAT = " + PAT + ") SELECT 1 ELSE SELECT 0";
			std::string result = GetQueryExecResult(testPATExistenceQuery);
			if (result == "0")
			{
				throw InvalidPATLogoutError("PAT does not correspond to any user in the database.");
			}
			std::cout << PAT;
			std::string query = "DELETE FROM CleverPocket.dbo.Sessions WHERE PAT = "+PAT;
			ExecQuery(query);
		}
		void OnUpdatePassword(char newPassword[], char emailTo[])
		{
			std::string l_newPassword = convertToSqlVarcharFormat(newPassword);
			std::string l_email = convertToSqlVarcharFormat(emailTo);
			std::string testEmailExistQuery = "IF EXISTS(SELECT 1 FROM CleverPocket.dbo.Users WHERE Email = " + l_email + ") SELECT 1 ELSE SELECT 0";
			std::string result = GetQueryExecResult(testEmailExistQuery);
			if (result == "0")
			{
				throw EmailInvalidForgotPasswordError("Email does not exist in the database! -- Tried to perform update password");
			}
			std::string query = "UPDATE CleverPocket.dbo.Users SET Password = " + l_newPassword + " WHERE Email = " + l_email;
			ExecQuery(query);
		}
		void OnValidateSixDigitCode(char validation_code[])
		{
			// look up for files named like that, and try open it.
			FILE* f = fopen(validation_code, "r");
			if (!f)
			{
				throw SixDigitCodeInvalidError("Invalid 6 digit validation code!");
			}
			char buffer[256];
			fgets(buffer, sizeof(buffer), f);
			if (strcmp(buffer, validation_code) != 0)
			{
				throw SixDigitCodeInvalidError("Invalid 6 digit validation code!");
			}
			std::cout << "[SERVER]: 6 digit Validation Code -- Reset Password -- passed.\n";
			fclose(f);
			std::remove(validation_code);
		}
		void OnSendEmailForgotPassword(char emailTo[])
		{
			if (!checkEmailFormat(emailTo))
			{
				throw EmailValidationError("Email Validation Failed. Format is not good!");
			}
			std::string l_email = convertToSqlVarcharFormat(emailTo);
			std::string query = "IF EXISTS (SELECT Email FROM CleverPocket.dbo.Users WHERE Email = " + l_email+") SELECT 1 ELSE SELECT 0";
			std::string resultBool = GetQueryExecResult(query);
			if (resultBool == "0")
			{
				//return -1;
				throw EmailInvalidForgotPasswordError("Email is not present in the database!");
			}

			// now handle the SMTP.
			std::string strEmailTo = emailTo;
			SendSMTPEmailValidationCodeTo(strEmailTo);
		}
		int OnLoginUserPAT(char PAT[])
		{
			std::string l_pat = convertToSqlVarcharFormat(PAT);
			std::string query = "SELECT UserID FROM [CleverPocket].[dbo].[Sessions] WHERE PAT = " + l_pat;
			std::string resultID = GetQueryExecResult(query);
			if (resultID == "")
			{
				std::cout << "-1";
				return -1;
				//throw InvalidPATLoginError("PAT is invalid, we couldn't get any user with this PAT. Try to login again!");
			}
			// now that a user exists
			// BASICALLY RETURN THE USER INFOS IN USER ACCOUNT SECTION.
			return std::stoi(resultID);
		}
		void OnRememberUserLoggedIn(char username[], char PAT[])
		{
			// first get the ID of user that is registered with this username
			std::string l_username = convertToSqlVarcharFormat(username);
			std::string query = "SELECT UserID FROM CleverPocket.dbo.Users WHERE Username = " + l_username;
			std::string resultID = GetQueryExecResult(query);
			if (resultID == "")
			{
				throw UsernameInvalidLoginError("Username is not valid! No user registered with this credential!");
			}
			else
			{
				// then go insert into Sessions userID and PAT.
				char resultIDChar[1024]; strcpy(resultIDChar, resultID.c_str());
				resultID = convertToSqlVarcharFormat(resultIDChar);
				std::string l_PAT = convertToSqlVarcharFormat(PAT);

				query = "INSERT INTO [CleverPocket].[dbo].[Sessions] (UserID, PAT) VALUES ( " + resultID + ", " + l_PAT+")";
				ExecQuery(query);
			}
		}
		void OnLoginUserToServer(char username[], char password[])
		{
			std::string l_username = convertToSqlVarcharFormat(username);
			std::string query = "IF EXISTS(SELECT 1 FROM CleverPocket.dbo.Users WHERE Username = " + l_username + ") " + "SELECT 1 ELSE SELECT 0";
			std::string result = GetQueryExecResult(query);
			if (result == "0")
			{
				throw UsernameInvalidLoginError("Username is not valid! No user registered with this credential!");
			}

			std::string l_password = convertToSqlVarcharFormat(password);
			query = "IF EXISTS(SELECT 1 FROM CleverPocket.dbo.Users WHERE Password = " + l_password + " AND Username = "+l_username+") " + "SELECT 1 ELSE SELECT 0";
			result = GetQueryExecResult(query);
			if (result == "0")
			{
				throw PasswordInvalidLoginError("Password is incorrect for this user!");
			}
		}

		void RegisterUserToDatabase(const CredentialHandler& credentials)
		{
			// build query with those credentials.
			if (!checkEmailFormat(credentials.getEmail()))
			{
				throw EmailValidationError("Email Validation Failed. Format is not good!");
			}

			if (checkUserAlreadyRegistered(credentials.getUsername(),credentials.getEmail()))
			{
				throw UserAlreadyRegisteredError("A user with this username is already registered to the database!");
			}
			std::string l_firstname = convertToSqlVarcharFormat(credentials.getFirstName());
			std::string l_lastname = convertToSqlVarcharFormat(credentials.getLastName());
			std::string l_username = convertToSqlVarcharFormat(credentials.getUsername());
			std::string l_password = convertToSqlVarcharFormat(credentials.getPassword());
			std::string l_email = convertToSqlVarcharFormat(credentials.getEmail());
			std::string l_countryid = convertToSqlVarcharFormat(credentials.getCountryID());
			std::string l_phoneNumber = convertToSqlVarcharFormat(credentials.getPhoneNumber());
			std::string query = "INSERT INTO [CleverPocket].[dbo].[Users] (FirstName, LastName, Username, Password, Email, Country, PhoneNumber) VALUES ( " + l_firstname + ", " + l_lastname + ", " + l_username + ", " + l_password + ", " + l_email + ", " + l_countryid+", "+l_phoneNumber+")";
			ExecQuery(query);

			//INITIALIZING NUMERAR TABLE
			query = "SELECT UserID FROM CleverPocket.dbo.Users WHERE Username = " + l_username;
			std::string resultID = GetQueryExecResult(query);
			std::string userID = convertToSqlVarcharFormat(resultID.c_str());
			query = "INSERT INTO[CleverPocket].[dbo].[Numerar] (UserID, SoldNumerar, CurrencyISO) VALUES (" + userID + ", 0 , '0')";
			ExecQuery(query);
		}
	private:
		std::string convertToSqlVarcharFormat(const char* field)
		{
			std::string str;
			str = "'";
			str += field;
			str += "'";
			return str;
		}
		bool checkUserAlreadyRegistered(const char* username, const char* email)
		{
			// first check if the username is already there.
			std::string l_username = convertToSqlVarcharFormat(username);
			std::string query = "IF EXISTS(SELECT 1 FROM CleverPocket.dbo.Users WHERE Username = " + l_username + ") " + "SELECT 1 ELSE SELECT 0";
			std::string result = GetQueryExecResult(query);
			if (result == "1")
			{
				return true;
			}

			// then check if email is already there.
			std::string l_email = convertToSqlVarcharFormat(email);
			query = "IF EXISTS(SELECT 1 FROM CleverPocket.dbo.Users WHERE Email = " + l_email + ") " + "SELECT 1 ELSE SELECT 0";
			result = GetQueryExecResult(query);
			if (result == "1")
			{
				return true;
			}

			return false;
		}
		bool checkEmailFormat(const char* email)
		{
			std::string str_email = email;
			// handle REGEX.
			const std::regex pattern("(\\w+)(\\.|_)?(\\w*)@(\\w+)(\\.(\\w+))+");
			return std::regex_match(str_email, pattern);
		}
		void SendSMTPEmailValidationCodeTo(const std::string& emailTo)
		{
			::CoInitialize(NULL); //?
			IMailPtr oSMTP = NULL;
			oSMTP.CreateInstance(__uuidof(EASendMailObjLib::Mail));
			oSMTP->LicenseCode = _T("TryIt");

			// set from gmail address
			oSMTP->FromAddr = _T("steelparrot.inc@gmail.com");

			// to gmail address.
			oSMTP->AddRecipientEx(_bstr_t(emailTo.c_str()), 0);

			// set email subject.
			oSMTP->Subject = _T("CleverPocket Forgot Password");

			// set email body.
			srand(time(NULL));
			std::string valid_code = "";
			for (int i = 0; i < 6; i++)
			{
				valid_code += std::to_string(rand() % 9);
			}
			FILE* fout = fopen(valid_code.c_str(), "w");
			fputs(valid_code.c_str(), fout);
			fclose(fout); // remove file from disk at validation.
			std::string msg = "Hello!\nLooks like you requested us to update your password, since you forgot it. Here's the validation code: " + valid_code+"\n\nAll the best,\nCleverPocket developers";
			oSMTP->BodyText = _bstr_t(msg.c_str()); // generate random code, save local to server in a file, and read it then delete.


			// gmail SMTP Server Address.
			oSMTP->ServerAddr = _T("smtp.gmail.com");

			// gmail user authentication should use your gmail address as username
			// extract secured from file or smt. TO DO!!
			oSMTP->UserName = _T("steelparrot.inc@gmail.com");
			oSMTP->Password = _T("papagaluu1"); 

			// set port 25 or 587.
			oSMTP->ServerPort = 587;

			// detect SSL/TLS automatically
			oSMTP->ConnectType = (long)ConnectSMTPCodes::ConnectSSLAuto;

			if (oSMTP->SendMail() == 0)
			{
				std::cout << "[SERVER]: email was sent successfully!\r\n";
			}
			else
			{
				std::cout<<"[SERVER]: failed to send email with the following error:\r\n"<< (const TCHAR*)oSMTP->GetLastErrDescription();
			}
		}
	protected:
		// Thread Safe Queue for incoming message packets
		tsqueue<owned_message<T>> m_qMessagesIn;

		// Container of active validated connections
		std::deque<std::shared_ptr<connection<T>>> m_deqConnections;

		// Order of declaration is important - it is also the order of initialisation
		asio::io_context m_asioContext;
		std::thread m_threadContext;

		// These things need an asio context
		asio::ip::tcp::acceptor m_asioAcceptor; // Handles new incoming connection attempts...

		// Clients will be identified in the "wider system" via an ID
		uint32_t nIDCounter = 10000;
		
		// dbconnection class.
		dbconnection<T> m_dbconnector;
	};
}