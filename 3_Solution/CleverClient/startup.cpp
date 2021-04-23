#define _CRT_SECURE_NO_WARNINGS
#include "Client.h"
#include "startup.h"
#include "stdafx.h"
#include <qmessagebox.h>

Startup::Startup(QWidget *parent)
	: QWidget(parent)
{
	this->currPAT = "";
	this->currEmail = "";
	ui.setupUi(this);
	//m_dshptr = new Dashboard(ui.stackedWidget);
	connect(ui.countryComboBox, SIGNAL(activated(int)), this, SLOT(on_countrySelected(int)));
	connect(ui.phoneLineEdit, SIGNAL(textEdited(QString)), this, SLOT(on_phoneNumberEdited()));
	//ui.stackedWidget->addWidget(m_dshptr); // add dashboard reference widget to stackedwidget.
	ui.validationCodeLineEdit->setVisible(false); ui.validatePushButton->setVisible(false);
	//check if pat.txt file can be opened.
	if (tryLoginRemembered())
	{
		m_dshptr = new Dashboard(QString(this->currPAT.c_str()),ui.stackedWidget);
		ui.stackedWidget->addWidget(m_dshptr); // add dashboard reference widget to stackedwidget.
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
	this->currEmail = "";
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

void Startup::fillCountries()
{
	if (ui.countryComboBox->count()>1)
	{
		return;
	}
	FILE* fin = fopen("Countries.txt", "r");
	if (fin)
	{
		char buffer[512];
		while (fgets(buffer, sizeof(buffer), fin))
		{
			std::string line;
			if (buffer[strlen(buffer) - 1] != '\n')
			{
				line = buffer;
			}
			else
			{
				line.assign(buffer, strlen(buffer) - 1);
			}
			std::stringstream ss(line);
			std::string countryName;
			std::string phoneCode;
			std::getline(ss, countryName, ',');
			std::getline(ss, phoneCode, ',');
			this->countries.insert(std::pair<std::string, std::string>(countryName, phoneCode));
		}
		fclose(fin);

		ui.countryComboBox->clear();
		for (std::map<std::string, std::string>::iterator it = countries.begin(); it != countries.end(); it++)
		{
			ui.countryComboBox->addItem(QString(it->first.c_str()));
		}
	}
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
	//this->clearGaps();
}

void Startup::resizeToForgotPasswordPage()
{
	/*this->resize(QSize(800, 600));
	ui.stackedWidget->resize(QSize(800, 600));*/
	this->setFixedSize(QSize(800, 600));
	ui.stackedWidget->setFixedSize(QSize(800, 600)); //optional
	this->clearGaps();
}

void Startup::resizeToUpdatePasswordPage()
{
	this->setFixedSize(QSize(800, 600));
	ui.stackedWidget->setFixedSize(QSize(800, 600)); //optional
	this->clearGaps();
}

void Startup::resizeToDashboard()
{
	this->setFixedSize(QSize(1360, 768));
	ui.stackedWidget->setFixedSize(QSize(1360, 768));
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
			c.ConnectToServer();
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
					this->currPAT = PAT;
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
	fillCountries();
	QObject::connect(ui.registerNowLinkButton, SIGNAL(clicked()), this, SLOT(on_registerNowLinkButton_clicked()));
	ui.stackedWidget->setCurrentWidget(ui.registerPage);
	this->resizeToRegisterPage();
}

void Startup::on_forgotPasswordSendPushButton_clicked()
{
	QString emailTo = ui.forgotPasswordEmailLineEdit->text();
	QMessageBox* msgBox = new QMessageBox;
	if (emailTo == "")
	{
		msgBox->setText("Please fill the email to send you the validation code!");
		msgBox->exec();
		return;
	}
	bool hasReceivedAnswer = true;
	Client& c = Client::getInstance();
	c.Incoming().clear();
	c.SendEmailForgotPassword(emailTo.toStdString());
	//while (c.Incoming().front().msg.header.id != clever::MessageType::SendEmailForgotPasswordRequest)
	while(c.Incoming().empty())
	{
		// do-wait.
		if (!c.IsConnected())
		{
			msgBox->setText("Server down! Client disconnected!");
			msgBox->show();
			hasReceivedAnswer = false;
			break;
		}
	}
	if (hasReceivedAnswer)
	{
		if (!c.Incoming().empty())
		{
			auto msg = c.Incoming().pop_front().msg;
			if (msg.header.id == clever::MessageType::SendEmailForgotPasswordRequest)
			{
				char responseBack[1024];
				msg >> responseBack;
				if (strcmp(responseBack, "SendEmailForgotPasswordSuccess") == 0)
				{
					// email
					msgBox->setText("An email has been sent to this email address, open it and enter the validation code we sent to you!");
					msgBox->show();
					QTimer::singleShot(2500, msgBox, SLOT(close()));
					ui.validationCodeLineEdit->setVisible(true); 
					ui.validatePushButton->setVisible(true);
					this->currEmail = emailTo.toStdString();
				}
				if (strcmp(responseBack, "InvalidEmailForgotPassword") == 0)
				{
					msgBox->setText("Invalid email! You are not registered to the database!");
					msgBox->show();
					QTimer::singleShot(2250, msgBox, SLOT(close()));
				}
				if (strcmp(responseBack, "InvalidFormatEmailForgotPassword") == 0)
				{
					msgBox->setText("Wrong Format Email! Re-enter again the email!");
					msgBox->show();
					QTimer::singleShot(2250, msgBox, SLOT(close()));
				}
				hasReceivedAnswer = false;
			}
		}
	}
	//delete msgBox; -- delete automatically on close!!
}

void Startup::on_validatePushButton_clicked()
{
	// TO DO - TAKE ME TO THE NEXT FORM WHERE I ENTER AND RE-ENTER NEW PASSWORD
	// THUS UPDATING THE DATABASE WITH NEW PASSWORD.
	QString validationCode = ui.validationCodeLineEdit->text();
	bool stillHasConnectedForAnswer = true;
	Client& c = Client::getInstance();

	QMessageBox* msgBox = new QMessageBox;
	if (validationCode == "")
	{
		msgBox->setText("Please fill the 6 digit code we sent over email.");
		msgBox->show();
		QTimer::singleShot(2250, msgBox, SLOT(close()));
		return;
	}
	if (validationCode.length() != 6)
	{
		msgBox->setText("Enter a 6-digit code!");
		msgBox->show();
		QTimer::singleShot(2250, msgBox, SLOT(close()));
		return;
	}

	c.Incoming().clear();
	c.ValidateMySixDigitCode(validationCode.toStdString());
	while (c.Incoming().empty())
	{
		// do-wait.
		if (!c.IsConnected())
		{
			msgBox->setText("Server down! Client disconnected!");
			msgBox->show();
			stillHasConnectedForAnswer = false;
			break;
		}
	}

	if (stillHasConnectedForAnswer)
	{
		if (!c.Incoming().empty())
		{
			auto msg = c.Incoming().pop_front().msg;
			if (msg.header.id == clever::MessageType::VerifyCodeForgotPasswordRequest)
			{
				char responseBack[1024];
				msg >> responseBack;

				if (strcmp(responseBack, "SuccessValidationCode") == 0)
				{
					ui.stackedWidget->setCurrentWidget(ui.newPasswordPage);
					this->resizeToUpdatePasswordPage();
				}
				if (strcmp(responseBack, "InvalidValidationCode") == 0)
				{
					msgBox->setText("Wrong 6-Digit Validation Code! Try again.");
					msgBox->show();
					QTimer::singleShot(2250, msgBox, SLOT(close()));
				}
				stillHasConnectedForAnswer = false;
			}
		}
	}
}

void Startup::on_updatePasswordPushButton_clicked()
{
	bool stillHasConnectedForAnswer = true;
	QMessageBox* msgBox = new QMessageBox;

	// check if boxes are filled
	QString newPassword = ui.newPasswordLineEdit->text();
	QString confirmNewPassword = ui.confirmNewPasswordLineEdit->text();

	if (newPassword == "" || confirmNewPassword == "")
	{
		msgBox->setText("Please fill all the boxes!");
		msgBox->show();
		QTimer::singleShot(2250, msgBox, SLOT(close()));
		return;
	}

	// check if password are the same

	if (newPassword != confirmNewPassword)
	{
		msgBox->setText("Passwords do not match! Re-enter again.");
		msgBox->show();
		QTimer::singleShot(2250, msgBox, SLOT(close()));
		return;
	}

	// perform update password request to server.
	Client& c = Client::getInstance();
	c.Incoming().clear();
	c.UpdatePasswordRequest(newPassword.toStdString(), this->currEmail);
	while (c.Incoming().empty())
	{
		// do-wait.
		if (!c.IsConnected())
		{
			msgBox->setText("Server down! Client disconnected!");
			msgBox->show();
			stillHasConnectedForAnswer = false;
			break;
		}
	}

	if (stillHasConnectedForAnswer)
	{
		if (!c.Incoming().empty())
		{
			auto msg = c.Incoming().pop_front().msg;
			if (msg.header.id == clever::MessageType::UpdatePasswordRequest)
			{
				char responseBack[1024];
				msg >> responseBack;

				if (strcmp(responseBack, "SuccessUpdatePassword") == 0)
				{
					msgBox->setText("Successfully updated password for this account! Redirecting to login page...");
					msgBox->show();
					QTimer::singleShot(2500, msgBox, SLOT(close()));
					ui.stackedWidget->setCurrentWidget(ui.loginPage);
					this->resizeToLoginPage();
				}
				if (strcmp(responseBack, "InvalidEmailUpdatePassword") == 0)
				{
					msgBox->setText("We couldn't update password for account with this email. Try again!");
					msgBox->show();
					QTimer::singleShot(2500, msgBox, SLOT(close()));
				}
				stillHasConnectedForAnswer = false;
			}
		}
	}

}

void Startup::on_countrySelected(int index)
{
	std::string t = std::to_string(index);
	QString countryName = ui.countryComboBox->currentText();

	// match the phone code from the map.
	std::string phoneCode = "(+";
	phoneCode += this->countries[countryName.toStdString()];
	phoneCode += ") ";
	ui.phoneLineEdit->setText(QString(phoneCode.c_str()));
}

void Startup::on_phoneNumberEdited()
{
	std::string currText = ui.phoneLineEdit->text().toStdString();
	if (currText[ui.phoneLineEdit->cursorPosition()-1] == ')')
	{
		ui.phoneLineEdit->undo();
	}
}

void Startup::on_backToLoginLinkButton_clicked()
{
	QObject::connect(ui.backToLoginLinkButton, SIGNAL(clicked()), this, SLOT(on_backToLoginLinkButton_clicked()));
	//
	ui.validationCodeLineEdit->clear();
	ui.validationCodeLineEdit->setVisible(false);
	ui.validatePushButton->setVisible(false);
	//
	
	ui.stackedWidget->setCurrentWidget(ui.loginPage);
	this->resizeToLoginPage();
}

void Startup::on_registerPushButton_clicked()
{
	QMessageBox msgBox;
	bool stillConnectedWaitingForAnswer = true;
	Client& c = Client::getInstance(); // handles connection too, if it is not connected.!!
	
	QString firstname = ui.firstNameLineEdit->text();
	QString lastname = ui.lastNameLineEdit->text();
	QString username = ui.usernameRLineEdit->text();
	QString password = ui.passwordRLineEdit->text();
	QString email = ui.emailLineEdit->text();
	QString country = ui.countryComboBox->currentText();
	QString phoneNumber = ui.phoneLineEdit->text();
	
	if (username == "" || password == "" || email == "" || firstname == "" || lastname =="" || country=="")
	{
		msgBox.setText("Please fill all the boxes!");
		msgBox.exec();
		return;
	}
	if (password != ui.confirmPasswordRLineEdit->text())
	{
		msgBox.setText("Passwords do not match!!");
		msgBox.exec();
		return;
	}

	// check if cursor has been modified for phone number. + TO DO: REGEX VALIDATION
	int phoneNumberSize = ui.phoneLineEdit->text().size() - (this->countries[country.toStdString()].size() + 2);
	if (phoneNumberSize < 4)
	{
		msgBox.setText("Phone number size is too small!");
		msgBox.exec();
		return;
	}

	// check if agreed terms and conditions.
	clever::CredentialHandler credentials;
	credentials.setFirstName(firstname.toStdString());
	credentials.setLastName(lastname.toStdString());
	credentials.setUsername(username.toStdString());
	credentials.setPassword(password.toStdString());
	credentials.setEmail(email.toStdString());
	credentials.setCountryID(country.toStdString()); // ID == Country name, since it is unique...
	credentials.setPhoneNumber(phoneNumber.toStdString());
	c.Incoming().clear();
	c.Register(credentials);
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
					FILE* fin = fopen("PAT.txt", "r");
					if (!fin)
					{
						FILE* fpat = fopen("PAT.txt", "w");
						std::string PAT = Client::generatePAT();
						fputs(PAT.c_str(), fpat);
						fclose(fpat);
						// request to update the database with this new PAT for this user.
						c.RememberMe(PAT, username.toStdString());
					}
					// else, make sure to delete session from DB -- request to server, get rid of PAT.txt too!
				
				}

				// finally, go to dashboard.
				if (m_dshptr==Q_NULLPTR)
				{
					m_dshptr = new Dashboard(username,ui.stackedWidget);
					ui.stackedWidget->addWidget(m_dshptr);
				}
				ui.stackedWidget->setCurrentWidget(m_dshptr);
				this->resizeToDashboard();
				m_dshptr = Q_NULLPTR; // reset pointer to dashboard for further log in attempt after sign out!!
				// and resize this to dashboard.
			}
		}
	}
}

void Startup::on_registerLinkButton_clicked()
{
	fillCountries();
	QObject::connect(ui.registerLinkButton, SIGNAL(clicked()), this, SLOT(on_registerLinkButton_clicked()));
	ui.stackedWidget->setCurrentWidget(ui.registerPage);

	// resize Startup 1360x768 and resize StackWidget parent of page too.
	this->resizeToRegisterPage();
}