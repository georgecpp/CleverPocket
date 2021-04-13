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
			char username[1024];
			char password[1024];
			char email[1024];

			msg >> email >> password >> username;
			char responseback[1024];

			try
			{
				RegisterUserToDatabase(username, password, email);
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