#define _CRT_SECURE_NO_WARNINGS
#include "Client.h"
#include "startup.h"
#include "stdafx.h"
#include <qmessagebox.h>


Startup::Startup(QWidget *parent)
	: QWidget(parent)
{
	ui.setupUi(this);
	m_dshptr = new Dashboard();
	ui.stackedWidget->addWidget(m_dshptr); // add dashboard reference widget to stackedwidget.
	//check if pat.txt file can be opened.
	if (tryLoginRemembered())
	{
		ui.stackedWidget->setCurrentWidget(m_dshptr);
		// and resize for the dashboard.
	}
	else
	{
		 //if not, sure login form
		ui.stackedWidget->setCurrentWidget(ui.loginPage); // login page appears on construction of startup.
		 //SIZE FOR THE LOGIN PAGE
		this->resizeToLoginPage();
	}
}

Startup::~Startup()
{

}
//deletes all inputs that could remain while crossing pages
void Startup::clearGaps()
{
	ui.usernameLineEdit->clear();
	ui.passwordLineEdit->clear();
	ui.firstNameLineEdit->clear();
	ui.lastNameLineEdit->clear();
	ui.usernameRLineEdit->clear();
	ui.passwordRLineEdit->clear();
	ui.confirmPasswordRLineEdit->clear();
	ui.countryComboBox->setCurrentIndex(0);
	ui.phoneLineEdit->clear();
	ui.forgotPasswordEmailLineEdit->clear();
	ui.keeploggedCheckBox->setChecked(false);
	ui.termsAndConditonsCheckBox->setChecked(false);
}

void Startup::resizeToLoginPage()
{
	/*this->resize(QSize(800, 600));
	ui.stackedWidget->resize(QSize(800, 600));*/
	this->setFixedSize(QSize(800, 600));
	ui.stackedWidget->setFixedSize(QSize(800, 600)); //optional
	this->clearGaps();
}

void Startup::resizeToRegisterPage()
{
	/*this->resize(QSize(1360, 768));
	ui.stackedWidget->resize(QSize(1360, 768));*/
	this->setFixedSize(QSize(1360, 768));
	ui.stackedWidget->setFixedSize(QSize(1360, 768)); //optional
	this->clearGaps();
}

void Startup::resizeToForgotPasswordPage()
{
	/*this->resize(QSize(800, 600));
	ui.stackedWidget->resize(QSize(800, 600));*/
	this->setFixedSize(QSize(800, 600));
	ui.stackedWidget->setFixedSize(QSize(800, 600)); //optional
	this->clearGaps();
}

void Startup::resizeToTermsAndConditionsPage()
{
	/*this->resize(QSize(1360, 768));
	ui.stackedWidget->resize(QSize(1360, 768));*/
	this->setFixedSize(QSize(1360, 768));
	ui.stackedWidget->setFixedSize(QSize(1360, 768)); //optional
}

bool Startup::tryLoginRemembered()
{
	QMessageBox msgBox;
	FILE* fin = fopen("PAT.txt", "r");
	if (fin)
	{
		bool stillConnectedWaitingForAnswer = true;
		Client& c = Client::getInstance();
		if (!c.IsConnected())
		{
			c.Connect("6.tcp.ngrok.io", 12553);
		}
		while (c.Incoming().empty())
		{
			//do-wait.
			if (!c.IsConnected())
			{
				msgBox.setText("Server down! Client disconnected!");
				msgBox.exec();
				stillConnectedWaitingForAnswer = false;
				break;
			}
		}
		// if it does, try to login with pat and set currentWidget to dashboard.
		char buffer[21];
		fgets(buffer, sizeof(buffer), fin);
		std::string PAT = buffer;
		auto msg = c.Incoming().front().msg;
		c.Incoming().clear();
		c.LoginUserRemembered(PAT);
		while (c.Incoming().empty())
		{
			if (!c.IsConnected())
			{
				msgBox.setText("Server down! Client disconnected!");
				msgBox.exec();
				stillConnectedWaitingForAnswer = false;
				break;
			}
		}
		if (!c.Incoming().empty()&&stillConnectedWaitingForAnswer)
		{
			auto msg = c.Incoming().pop_front().msg;
			if (msg.header.id == clever::MessageType::LoginRememeberedRequest)
			{
				// server has responded to remembered-login request.
				char responseback[1024];
				msg >> responseback;
				if (strcmp(responseback, "SuccessRememberLogin") == 0)
				{
					// login successfully -- proceed to dashboard with this account logged in.
					fclose(fin);
					return true;
				}
				else
				{
					fclose(fin);
					return false;
				}
			}
		}
	}
	return false;
}

void Startup::on_alreadyRegisteredLinkButton_clicked()
{
	QObject::connect(ui.alreadyRegisteredLinkButton, SIGNAL(clicked()), this, SLOT(on_alreadyRegisteredLinkButton_clicked()));
	ui.stackedWidget->setCurrentWidget(ui.loginPage);
	this->resizeToLoginPage();
}

void Startup::on_forgotPasswordLinkButton_clicked()
{
	QObject::connect(ui.forgotPasswordLinkButton, SIGNAL(clicked()), this, SLOT(on_forgotPasswordLinkButton_clicked()));
	ui.stackedWidget->setCurrentWidget(ui.forgotPasswordPage);
	this->resizeToForgotPasswordPage();
}

void Startup::on_termsAndConditionsLinkButton_clicked()
{
	QObject::connect(ui.termsAndConditionsLinkButton, SIGNAL(clicked()), this, SLOT(on_termsAndConditionsLinkButton_clicked()));
	ui.stackedWidget->setCurrentWidget(ui.termsAndConditionPage);
	this->resizeToTermsAndConditionsPage();
}

void Startup::on_backToRegisterLinkButton_clicked()
{
	QObject::connect(ui.backToRegisterLinkButton, SIGNAL(clicked()), this, SLOT(on_backToRegisterLinkButton_clicked()));
	ui.stackedWidget->setCurrentWidget(ui.registerPage);
	this->resizeToRegisterPage();
	
}

void Startup::on_registerNowLinkButton_clicked()
{
	QObject::connect(ui.registerNowLinkButton, SIGNAL(clicked()), this, SLOT(on_registerNowLinkButton_clicked()));
	ui.stackedWidget->setCurrentWidget(ui.registerPage);
	this->resizeToRegisterPage();
}

void Startup::on_backToLoginLinkButton_clicked()
{
	QObject::connect(ui.backToLoginLinkButton, SIGNAL(clicked()), this, SLOT(on_backToLoginLinkButton_clicked()));
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

void Startup::on_loginPushButton_clicked()
{
	// first perform check with these credentials.
	// if not, tell user what's wrong and return.
	bool stillConnectedWaitingForAnswer = true;
	QMessageBox msgBox;
	QString username = ui.usernameLineEdit->text();
	QString password = ui.passwordLineEdit->text();

	if (username == "" || password == "")
	{
		msgBox.setText("Please fill all the boxes!");
		msgBox.exec();
		return;
	}

	Client& c = Client::getInstance();
	c.Incoming().clear();
	c.LoginUser(username.toStdString(), password.toStdString());
	while (c.Incoming().empty())
	{
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
		if (msg.header.id == clever::MessageType::LoginRequest)
		{
			char responseback[1024];
			msg >> responseback;
			if (strcmp(responseback, "UsernameInvalidError") == 0)
			{
				msgBox.setText("Invalid Username! Please try again.");
				msgBox.exec();

				// optional, clear the username...
				// ui.usernameLineEdit->clear(); 
				return;
			}
			if (strcmp(responseback, "PasswordInvalidError") == 0)
			{
				msgBox.setText("Invalid Password! Please try again.");
				msgBox.exec();

				// optional, clear the password...
				//ui.passwordLineEdit->clear();
				return;
			}
			if (strcmp(responseback, "SuccessLogin") == 0)
			{
				// then check if "Keep Me Logged In" is checked.
				// TO DO
				if (ui.keeploggedCheckBox->isChecked())
				{
					FILE* fpat = fopen("PAT.txt", "w");
					std::string PAT = Client::generatePAT();
					fputs(PAT.c_str(), fpat);
					fclose(fpat);

					// request to update the database with this new PAT for this user.
					c.RememberMe(PAT, username.toStdString());
				}

				// finally, go to dashboard.
				ui.stackedWidget->setCurrentWidget(m_dshptr); 
				// and resize this to dashboard.
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