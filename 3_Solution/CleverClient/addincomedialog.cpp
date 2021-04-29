#define _CRT_SECURE_NO_WARNINGS
#include "addincomedialog.h"
#include "Client.h"
#include <qmessagebox.h>
#include <qtimer.h>
#include <time.h>

AddIncomeDialog::AddIncomeDialog(QWidget* parent)
	: QDialog(parent)
{
	setupUi(this);
}

AddIncomeDialog::AddIncomeDialog(std::map<std::string, clever::CardCredentialHandler>& map_cards, const QString& PAT, QWidget* parent)
	: AddIncomeDialog(parent)
{
	this->map_cards = map_cards;
	this->currPATLogged = PAT;
	this->currUsernameLogged = "";
	for (auto& it : map_cards)
	{
		this->cardsIncomeComboBox->addItem(it.second.getCardName());
	}
}

AddIncomeDialog::AddIncomeDialog(std::map<std::string, clever::CardCredentialHandler>& map_cards, const std::string& username, QWidget* parent)
	: AddIncomeDialog(parent)
{
	this->map_cards = map_cards;
	this->currPATLogged = "";
	this->currUsernameLogged = username;
	for (auto& it : map_cards)
	{
		this->cardsIncomeComboBox->addItem(it.second.getCardName());
		
	}
}

AddIncomeDialog::~AddIncomeDialog()
{
}

std::map<std::string, clever::CardCredentialHandler>& AddIncomeDialog::getCardMap()
{
	// TODO: insert return statement here
	return this->map_cards;
}

void AddIncomeDialog::on_cancelAddIncomePushButton_clicked()
{
	this->done(QDialog::Rejected);
}

void AddIncomeDialog::on_addPeriodicallyIncomePushButton_clicked()
{
	bool stillConnectedWaitingAnswer = true;
	QMessageBox* msgBox = new QMessageBox();
	
	QString incomeName = this->incomeNameLineEdit->text();
	QString valueIncome = this->incomeValue->text();
	QString cardSelected = this->cardsIncomeComboBox->currentText();
	QString dayIncome = this->dayOfMonthComboBox->currentText();
	QString incomeSource = this->incomeSourceLineEdit->text();
	QString incomeCurrencyISO = this->cardCurrencyISO->text();
	clever::FinanceTypeCredentialHandler incomeCredHandler(incomeName.toStdString(), incomeSource.toStdString(), incomeCurrencyISO.toStdString(), dayIncome.toStdString(), cardSelected.toStdString(), valueIncome.toFloat());
	//all boxes are filled
	if (incomeName == "" || valueIncome == "")
	{
		msgBox->setText("Please fill all the boxes!");
		msgBox->show();
		QTimer::singleShot(2250, msgBox, SLOT(close()));
		return;
	}
	Client& c = Client::getInstance();
	c.Incoming().clear();
	c.UsernameAddIncome(this->currUsernameLogged, incomeCredHandler);
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
		if (msg.header.id == clever::MessageType::ServerIncomeResponse)
		{
			// server has responded back to add card request.
			char responseBack[1024];
			msg >> responseBack;
			if (strcmp(responseBack, "SuccessAddIncome") == 0)
			{
				// ALL GOOD.
				msgBox->setText("Income added with success!");
				msgBox->show();
				QTimer::singleShot(2500, msgBox, SLOT(close()));
			}
			else
			{
				msgBox->setText("Couldn't add income! Server problem. Try again.");
				msgBox->show();
				QTimer::singleShot(2500, msgBox, SLOT(close()));
				return;
			}
			stillConnectedWaitingAnswer = false;
		}
	}

	this->done(QDialog::Accepted);
}
