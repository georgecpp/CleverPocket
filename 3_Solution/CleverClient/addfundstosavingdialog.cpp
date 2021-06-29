#include "addfundstosavingdialog.h"
#include <qmessagebox.h>
#include <qtimer.h>
#include <Client.h>
AddFundsToSavingDialog::AddFundsToSavingDialog(const std::string& currUsername, std::map<std::string, clever::CardCredentialHandler>& map_cards, QWidget *parent)
	: QDialog(parent)
{
	setupUi(this);
	this->currUsername = currUsername;
	this->map_cards = map_cards;
	for (auto it : map_cards)
	{
		this->financePicker->addItem(it.first.c_str());
	}
}

AddFundsToSavingDialog::~AddFundsToSavingDialog()
{

}

std::map<std::string, clever::CardCredentialHandler>& AddFundsToSavingDialog::getMapCardsUpdated()
{
	return this->map_cards;
}

void AddFundsToSavingDialog::on_addFundsPushButton_clicked()
{
	bool stillConnectedWaitingAnswer = true;
	QMessageBox* msgBox = new QMessageBox();
	if (this->valueLineEdit->text() == "")
	{
		msgBox->setText("Please fill the funds value box!");
		msgBox->show();
		QTimer::singleShot(2250, msgBox, SLOT(close()));
		return;
	}
	float fundsValue = this->valueLineEdit->text().toFloat();
	if (fundsValue > this->map_cards[this->financePicker->currentText().toStdString()].getCardSold())
	{
		msgBox->setText("Not enough funds on this card!");
		msgBox->show();
		QTimer::singleShot(2000, msgBox, SLOT(close()));
		return;
	}
	float diff = this->map_cards[this->financePicker->currentText().toStdString()].getCardSold() - fundsValue;
	this->map_cards[this->financePicker->currentText().toStdString()].setCardSold(diff);
	std::string fromCardName = this->financePicker->currentText().toStdString();
	Client& c = Client::getInstance();
	c.Incoming().clear();
	c.UsernameAddFundsToSaving(this->currUsername, this->valueLineEdit->text().toStdString(), fromCardName, this->savingTitle->text().toStdString());
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
		if (msg.header.id == clever::MessageType::ServerAddFundsToSavingResponse)
		{
			// server has responded back to add funds to saving request.
			char responseBack[1024];
			msg >> responseBack;
			if (strcmp(responseBack, "SuccessAddFundsToSaving") == 0)
			{
				// ALL GOOD.
				msgBox->setText("Funds added with success!");
				msgBox->show();
				QTimer::singleShot(2500, msgBox, SLOT(close()));
			}
			else
			{
				msgBox->setText("Couldn't add funds to saving! Server problem. Try again.");
				msgBox->show();
				QTimer::singleShot(2500, msgBox, SLOT(close()));
				return;
			}
			stillConnectedWaitingAnswer = false;
		}
	}
	this->done(QDialog::Accepted);
}

void AddFundsToSavingDialog::on_financePicker_currentTextChanged(const QString& financeSelected)
{
	this->currencyISOLabel->setText(this->map_cards[financeSelected.toStdString()].getCardCurrencyISO());
}
