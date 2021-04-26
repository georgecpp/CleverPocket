#define _CRT_SECURE_NO_WARNINGS
#include "rechargecashdialog.h"
#include "Client.h"
#include <qmessagebox.h>
#include <qtimer.h>

RechargeCashDialog::RechargeCashDialog(QWidget* parent)
	: QDialog(parent)
{
	setupUi(this);
}

RechargeCashDialog::RechargeCashDialog(std::string userCurrencyISO, std::map<std::string, clever::CardCredentialHandler>& map_cards, const QString& PAT, QWidget* parent)
	: RechargeCashDialog(parent)
{
	this->currPATLogged = PAT;
	this->currUsernameLogged = "";
	for (auto& it : map_cards)
	{
		this->rechargeCashComboBox->addItem(it.second.getCardName());
	}
	this->userCurrencyISO = userCurrencyISO;
}

RechargeCashDialog::RechargeCashDialog(std::string userCurrencyISO, std::map<std::string, clever::CardCredentialHandler>& map_cards, const std::string& username, QWidget* parent)
	:RechargeCashDialog(parent)
{
	this->currUsernameLogged = username;
	this->currPATLogged = "";
	for (auto& it : map_cards)
	{
		this->rechargeCashComboBox->addItem(it.second.getCardName());
	}
	this->userCurrencyISO = userCurrencyISO;
}

RechargeCashDialog::~RechargeCashDialog()
{

}

void RechargeCashDialog::on_cancelRechargePushButton_clicked()
{
	this->done(QDialog::Rejected);
}

void RechargeCashDialog::on_rechargePushButton_clicked()
{	
	QMessageBox* msgBox = new QMessageBox;
	if (this->map_cards[this->rechargeCashComboBox->currentText().toStdString()].getCardCurrencyISO() != this->userCurrencyISO)
	{
		msgBox->setText("Please select a card with the same currency as the cash!");
		msgBox->show();
		QTimer::singleShot(2000, msgBox, SLOT(close()));
		return;
	}
	bool stillConnectedWaitingAnswer = true;
	cashValue = this->cashFundsLIneEdit->text();
	if (cashValue == "")
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
		c.UserPATAddCash(this->currPATLogged.toStdString(), this->cashValue.toStdString());
	}
	else
	{
		c.UsernameAddCash(this->currUsernameLogged, this->cashValue.toStdString());
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
		}//aici raman
	}
	if (!c.Incoming().empty() && stillConnectedWaitingAnswer)
	{
		auto msg = c.Incoming().pop_front().msg;
		if (msg.header.id == clever::MessageType::ServerAddCashResponse)
		{
			// server has responded back to add card request.
			char responseBack[1024];
			msg >> responseBack;
			if (strcmp(responseBack, "SuccessAddFCash") == 0)
			{
				// ALL GOOD.
				msgBox->setText("Cash added with success!");
				msgBox->show();
				QTimer::singleShot(2500, msgBox, SLOT(close()));
			}
			else
			{
				msgBox->setText("Couldn't add cash! Server problem. Try again.");
				msgBox->show();
				QTimer::singleShot(2500, msgBox, SLOT(close()));
				return;
			}
			stillConnectedWaitingAnswer = false;
		}
	}


	this->done(QDialog::Accepted);
}