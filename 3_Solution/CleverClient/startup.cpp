#include "Client.h"
#include "startup.h"
#include "stdafx.h"
#include <qmessagebox.h>


Startup::Startup(QWidget *parent)
	: QWidget(parent)
{
	ui.setupUi(this);
	ui.stackedWidget->setCurrentWidget(ui.loginPage); // login page appears on construction of startup.
	// SIZE FOR THE LOGIN PAGE
	this->resizeToLoginPage();
	
}

Startup::~Startup()
{

}

void Startup::resizeToLoginPage()
{
	this->resize(QSize(800, 600));
	ui.stackedWidget->resize(QSize(800, 600));
}

void Startup::resizeToRegisterPage()
{
	this->resize(QSize(1360, 768));
	ui.stackedWidget->resize(QSize(1360, 768));
}

void Startup::resizeToForgotPasswordPage()
{
	// TO BE DONE.
}

void Startup::on_alreadyRegisteredLinkButton_clicked()
{
	QObject::connect(ui.alreadyRegisteredLinkButton, SIGNAL(clicked()), this, SLOT(on_alreadyRegisteredLinkButton_clicked()));
	ui.stackedWidget->setCurrentWidget(ui.loginPage);
	this->resizeToLoginPage();
}

void Startup::on_registerPushButton_clicked()
{
	QMessageBox msgBox;
	bool stillConnectedWaitingForAnswer = true;
	Client& c = Client::getInstance(); // handles connection too, if it is not connected.!!

	QString username = ui.usernameRLineEdit->text();
	QString password = ui.passwordRLineEdit->text();
	QString email = ui.emailLineEdit->text();

	if (username == "" || password == "" || email == "")
	{
		msgBox.setText("Please fill all the boxes!");
		msgBox.exec();
		return;
	}
	c.Incoming().clear();
	c.Register(username.toStdString(), password.toStdString(), email.toStdString());
	while (c.Incoming().empty())
	{
		// do-wait.
		if (!c.IsConnected())
		{
			msgBox.setText("Server down! Client disconnected!");
			msgBox.exec();
			stillConnectedWaitingForAnswer = false;
			break;
		}
	}
	if (!c.Incoming().empty() && stillConnectedWaitingForAnswer)
	{
		auto msg = c.Incoming().pop_front().msg;
		if (msg.header.id == clever::MessageType::RegisterRequest)
		{
			// server has responded to register request.
			char responseback[1024];
			msg >> responseback;
			if (strcmp(responseback, "Success")==0)
			{
				ui.stackedWidget->setCurrentWidget(ui.loginPage);
				this->resizeToLoginPage();
			}
			if (strcmp(responseback, "AlreadyRegistered") == 0)
			{
				msgBox.setText("A user with these credentials is already registered!");
				msgBox.exec();
			}
		}
	}
}

void Startup::on_registerLinkButton_clicked()
{
	QObject::connect(ui.registerLinkButton, SIGNAL(clicked()), this, SLOT(on_registerLinkButton_clicked()));
	ui.stackedWidget->setCurrentWidget(ui.registerPage);

	// resize Startup 1360x768 and resize StackWidget parent of page too.
	this->resizeToRegisterPage();
}