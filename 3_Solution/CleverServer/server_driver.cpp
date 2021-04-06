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