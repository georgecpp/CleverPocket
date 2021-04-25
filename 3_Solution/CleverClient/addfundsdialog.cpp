#define _CRT_SECURE_NO_WARNINGS
#include "addfundsdialog.h"
#include "Client.h"
#include <qmessagebox.h>
<<<<<<< Updated upstream
=======
#include <qtimer.h>
>>>>>>> Stashed changes

AddFundsDialog::AddFundsDialog(QWidget *parent)
	: QDialog(parent)
{
	setupUi(this);
}



AddFundsDialog::AddFundsDialog(const QString& cardName, const QString& PAT, QWidget* parent)
	: AddFundsDialog(parent)
{
	this->currCardName = cardName.toStdString();
<<<<<<< Updated upstream
	this->currPATLogged = PAT.toStdString();
	this->currUsernameLogged = "";
=======
	this->currPATLogged = PAT;
	this->currUsernameLogged = "";
	this->cardNameLineEdit->setText(cardName);
	this->loadCardCurrencyISO();

>>>>>>> Stashed changes
}

AddFundsDialog::AddFundsDialog(const QString& cardName, const std::string& username, QWidget* parent)
	: AddFundsDialog(parent)
{
	this->currCardName = cardName.toStdString();
	this->currUsernameLogged = username;
	this->currPATLogged = "";
<<<<<<< Updated upstream
=======
	this->cardNameLineEdit->setText(cardName);
	this->loadCardCurrencyISO();
>>>>>>> Stashed changes
}

AddFundsDialog::~AddFundsDialog()
{
}

<<<<<<< Updated upstream
=======
void AddFundsDialog::on_cancelAddFundsPushButton_clicked()
{
	this->done(QDialog::Accepted);
}

void AddFundsDialog::loadCardCurrencyISO()
{
	bool stillConnectedWaitingForAnswer = true;
	QMessageBox* msgBox = new QMessageBox;
	Client& c = Client::getInstance();
	c.Incoming().clear();
	if (this->currPATLogged == "") // it means user-login
	{
		c.UsernameGetSelectedCardCurrency(currUsernameLogged, currCardName);
	}
	else // else PAT-login
	{
		c.PATGetSelectedCardCurrency(currPATLogged.toStdString(), currCardName);
	}
	while (c.Incoming().empty())
	{
		if (!c.IsConnected())
		{
			msgBox->setText("Server down! Client disconnected!");
			msgBox->show();
			stillConnectedWaitingForAnswer = false;
			break;
		}
	}
	while (stillConnectedWaitingForAnswer)
	{
		if (!c.IsConnected())
		{
			msgBox->setText("Server down! Client disconnected!");
			msgBox->show();
			stillConnectedWaitingForAnswer = false;
			break;
		}
		if (!c.Incoming().empty())
		{
			auto msg = c.Incoming().pop_front().msg;
			if (msg.header.id == clever::MessageType::ServerGetCurrencyResponse)
			{
				char cardCurrencyISO[1024]; msg >> cardCurrencyISO;
				this->currencyLineEdit->setText(QString(cardCurrencyISO));

			}
		}
	}

}

>>>>>>> Stashed changes

void AddFundsDialog::on_addFundsPushButton_clicked()
{
	bool stillConnectedWaitingAnswer = true;
	QMessageBox* msgBox = new QMessageBox();
<<<<<<< Updated upstream
=======
	fundsValue = this->fundValueLineEdit->text();

	if (fundsValue == "")
	{
		msgBox->setText("Please fill the funds value boxe!");
		msgBox->show();
		QTimer::singleShot(2250, msgBox, SLOT(close()));
		return;
	}

	Client& c = Client::getInstance(); // handles connection too, if it is not connected.!!
	c.Incoming().clear();
	if (this->currUsernameLogged == "")
	{
		c.UserPATAddCardFunds(this->currPATLogged.toStdString(),this->currCardName, this->fundsValue.toStdString());
	}
	else
	{
		c.UsernameAddCardFunds(this->currUsernameLogged, this->currCardName, this->fundsValue.toStdString());
	}
	while (c.Incoming().empty())
	{
		if (!c.IsConnected())
		{
			msgBox->setText("Server down! Client disconnected!");
			msgBox->show();
			QTimer::singleShot(2500, msgBox, SLOT(close()));
			stillConnectedWaitingAnswer = false;
			break;
		}
	}

	if (!c.Incoming().empty() && stillConnectedWaitingAnswer)
	{
		auto msg = c.Incoming().pop_front().msg;
		if (msg.header.id == clever::MessageType::ServerAddFundsResponse)
		{
			// server has responded back to add card request.
			char responseBack[1024];
			msg >> responseBack;
			if (strcmp(responseBack, "SuccessAddFunds") == 0)
			{
				// ALL GOOD.
				msgBox->setText("Funds added with success!");
				msgBox->show();
				QTimer::singleShot(2500, msgBox, SLOT(close()));
			}
			else
			{
				msgBox->setText("Couldn't add funds to the card! Server problem. Try again.");
				msgBox->show();
				QTimer::singleShot(2500, msgBox, SLOT(close()));
				return;
			}
			stillConnectedWaitingAnswer = false;
		}
	}
	this->done(QDialog::Accepted);

>>>>>>> Stashed changes
}

