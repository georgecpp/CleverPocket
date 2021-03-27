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
				strcpy(responseback, "Successfully registered user to the database!");
			}
			catch (clever::EmailValidationError)
			{
				strcpy(responseback, "The email you entered is not a valid format! ");
			}
			catch (clever::UserAlreadyRegisteredError)
			{
				strcpy(responseback, "User with these credentials is already registered!");
			}
			catch (clever::DatabaseQueryError)
			{
				strcpy(responseback, "Couldn't register user to the database!");
			}
			catch (clever::DatabaseConnectionError)
			{
				strcpy(responseback, "Server couldn't connect to SQL Server Database!");
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