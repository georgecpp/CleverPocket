#define _CRT_SECURE_NO_WARNINGS
#include <iostream>
#include <olc_net.h>

enum class CustomMsgTypes : uint32_t
{
	ServerAccept,
	ServerDeny,
	ServerPing,
	MessageAll,
	ServerMessage,
	RegisterRequest,
};



class CustomServer : public olc::net::server_interface<CustomMsgTypes>
{
public:
	CustomServer(uint16_t nPort) : olc::net::server_interface<CustomMsgTypes>(nPort)
	{

	}

protected:
	virtual bool OnClientConnect(std::shared_ptr<olc::net::connection<CustomMsgTypes>> client)
	{
		olc::net::message<CustomMsgTypes> msg;
		msg.header.id = CustomMsgTypes::ServerAccept;
		client->Send(msg);
		return true;
	}

	// Called when a client appears to have disconnected
	virtual void OnClientDisconnect(std::shared_ptr<olc::net::connection<CustomMsgTypes>> client)
	{
		std::cout << "Removing client [" << client->GetID() << "]\n";
	}

	// Called when a message arrives
	virtual void OnMessage(std::shared_ptr<olc::net::connection<CustomMsgTypes>> client, olc::net::message<CustomMsgTypes>& msg)
	{
		switch (msg.header.id)
		{
		case CustomMsgTypes::ServerPing:
		{
			std::cout << "[" << client->GetID() << "]: Server Ping\n";

			// Simply bounce message back to client
			client->Send(msg);
		}
		break;

		case CustomMsgTypes::MessageAll:
		{
			std::cout << "[" << client->GetID() << "]: Message All\n";

			// Construct a new message and send it to all clients
			olc::net::message<CustomMsgTypes> msg;
			msg.header.id = CustomMsgTypes::ServerMessage;
			msg << client->GetID();
			MessageAllClients(msg, client);

		}
		break;

		case CustomMsgTypes::RegisterRequest:
		{
			// the message contains the data for registration.
			std::cout << "[" << client->GetID() << "]: Register to DB request\n";
			char username[1024];
			char password[1024];
			char email[1024];

			msg >> email >> password >> username;
			char responseback[1024];

			if (RegisterUserToDatabase(username, password, email))
			{
				strcpy(responseback,"Successfully registered user to the database!");
			}
			else
			{
				// MORE EXPLICIT!!! WHY?? ALREADY THERE? ETC.
				strcpy(responseback, "Couldn't register user to the database");
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
	CustomServer server(60000);
	server.Start();
	//server.ConnectToDatabase();
	//server.ExecQuery("INSERT INTO [CleverPocket].[dbo].[Users] (Username, Password, Email, Role) VALUES ('test1', 'test123', 'testxd', 'user')");

	while (1)
	{
		server.Update(-1, true);
	}



	return 0;
}