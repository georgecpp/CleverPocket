#include "Client.h"
#include "startup.h"
#include "stdafx.h"
#include <qmessagebox.h>


Startup::Startup(QWidget *parent)
	: QWidget(parent)
{
	ui.setupUi(this);
}

Startup::~Startup()
{

}

void Startup::on_registerButton_clicked()
{
	bool stillConnectedWaitingForAnswer = true;
	Client& c = Client::getInstance(); // handles connection too, if it is not connected.


	QString username = ui.usernameLineEdit->text();
	QString password = ui.passwordLineEdit->text();
	QString email = ui.emailLineEdit->text();


	// handle if one box is missing text - TO DO
	if (username == "" && password == "" && email == "")
	{
		c.Register("rifflord", "rifflord123", "rifflord@gmail.com");
	}
	else
	{
		c.Register(username.toStdString(), password.toStdString(), email.toStdString());
	}

	// handle a delay before getting response from server -- EVENTUALLY A LOADING STATE.
	QMessageBox msgBox;
	if(!c.Incoming().empty() && stillConnectedWaitingForAnswer)
	{
		auto msg = c.Incoming().pop_front().msg;
		if (msg.header.id == clever::MessageType::RegisterRequest)
		{
			// server has responded to register request.
			char responseback[1024];
			msg >> responseback;
			msgBox.setText(responseback);
			msgBox.exec();
		}
	}
}	
