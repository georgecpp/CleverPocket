#define _CRT_SECURE_NO_WARNINGS
#include "addfundsdialog.h"
#include "Client.h"
#include <qmessagebox.h>
#include <qtimer.h>


AddFundsDialog::AddFundsDialog(QWidget *parent)
	: QDialog(parent)
{
	setupUi(this);
	this->cardNameLineEdit->setReadOnly(true);
	this->currencyLineEdit->setReadOnly(true);
}


AddFundsDialog::AddFundsDialog(const QString& cardName, const QString& currencyISO, const QString& PAT, QWidget* parent)
	: AddFundsDialog(parent)
{
	this->currCardName = cardName.toStdString();
	this->currencyISO = currencyISO.toStdString();
		this->currPATLogged = PAT;
	this->currUsernameLogged = "";
	this->cardNameLineEdit->setText(cardName);
	this->currencyLineEdit->setText(currencyISO);
}

AddFundsDialog::AddFundsDialog(const QString& cardName, const QString& currencyISO, const std::string& username, QWidget* parent)
	: AddFundsDialog(parent)
{
	this->currCardName = cardName.toStdString();
	this->currencyISO = currencyISO.toStdString();
	this->currUsernameLogged = username;
	this->currPATLogged = "";
	this->cardNameLineEdit->setText(cardName);
	this->currencyLineEdit->setText(currencyISO);
}

AddFundsDialog::~AddFundsDialog()
{

}

void AddFundsDialog::on_addFundPushButton_clicked()
{
	bool stillConnectedWaitingAnswer = true;
	QMessageBox* msgBox = new QMessageBox();
	fundsValue = this->fundValueLineEdit->text();

	if (fundsValue == "")
	{
		msgBox->setText("Please fill the funds value box!");
		msgBox->show();
		QTimer::singleShot(2250, msgBox, SLOT(close()));
		return;
	}
	Client& c = Client::getInstance(); // handles connection too, if it is not connected.!!
	c.Incoming().clear();
	if (this->currUsernameLogged == "")
	{
		c.UserPATAddCardFunds(this->currPATLogged.toStdString(), this->currCardName, this->fundsValue.toStdString());
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
}

void AddFundsDialog::on_cancelAddFundsPushButton_clicked()
{
	this->done(QDialog::Rejected);
}
