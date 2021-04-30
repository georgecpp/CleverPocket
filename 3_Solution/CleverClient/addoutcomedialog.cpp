#define _CRT_SECURE_NO_WARNINGS
#include "addoutcomedialog.h"
#include "Client.h"
#include <qmessagebox.h>
#include <qtimer.h>
#include <time.h>

AddOutComeDialog::AddOutComeDialog(QWidget *parent)
	: QDialog(parent)
{
	setupUi(this);
}

AddOutComeDialog::AddOutComeDialog(std::map<std::string, clever::CardCredentialHandler>& map_cards, const QString& PAT, QWidget* parent)
	: AddOutComeDialog(parent)
{
	this->map_cards = map_cards;
	this->currPATLogged = PAT;
	this->currUsernameLogged = "";
	for (auto& it : map_cards)
	{
		this->cardsOutcomeComboBox->addItem(it.second.getCardName());
	}
}

AddOutComeDialog::AddOutComeDialog(std::map<std::string, clever::CardCredentialHandler>& map_cards, const std::string& username, QWidget* parent)
	: AddOutComeDialog(parent)
{
	this->map_cards = map_cards;
	this->currPATLogged = "";
	this->currUsernameLogged = username;
	for (auto& it : map_cards)
	{
		this->cardsOutcomeComboBox->addItem(it.second.getCardName());
	}
}

AddOutComeDialog::~AddOutComeDialog()
{
}

std::map<std::string, clever::CardCredentialHandler>& AddOutComeDialog::getCardMap()
{
	// TODO: insert return statement here
	return this->map_cards;
}

void AddOutComeDialog::on_cancelAddOutcomePushButton_clicked()
{
	this->done(QDialog::Rejected);
}

void AddOutComeDialog::on_addPeriodicallyOutcomePushButton_clicked()
{
	bool stillConnectedWaitingAnswer = true;
	QMessageBox* msgBox = new QMessageBox();

	QString outcomeName = this->OutcomeNameLineEdit->text();
	QString valueOutcome = this->outcomeValue->text();
	QString cardSelected = this->cardsOutcomeComboBox->currentText();
	QString dayOutcome = this->dayOfMonthComboBox->currentText();
	QString OutcomeSource = this->OutcomeDestinationLineEdit->text();
	QString outcomeCurrencyISO = this->cardCurrencyISO->text();
	clever::FinanceTypeCredentialHandler outcomeCredHandler(outcomeName.toStdString(), OutcomeSource.toStdString(), outcomeCurrencyISO.toStdString(), dayOutcome.toStdString(), cardSelected.toStdString(), valueOutcome.toFloat());
	
	//all boxes are filled

	if (outcomeName == "" || valueOutcome == "")
	{
		msgBox->setText("Please fill all the boxes!");
		msgBox->show();
		QTimer::singleShot(2250, msgBox, SLOT(close()));
		return;
	}
	Client& c = Client::getInstance();
	c.Incoming().clear();
	c.UsernameAddOutcome(this->currUsernameLogged, outcomeCredHandler);
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
		if (msg.header.id == clever::MessageType::ServerOutcomeResponse)
		{
			// server has responded back to add card request.
			char responseBack[1024];
			msg >> responseBack;
			if (strcmp(responseBack, "SuccessAddOutcome") == 0)
			{
				// ALL GOOD.
				msgBox->setText("Outcome added with success!");
				msgBox->show();
				QTimer::singleShot(2500, msgBox, SLOT(close()));
			}
			else
			{
				msgBox->setText("Couldn't add outcome! Server problem. Try again.");
				msgBox->show();
				QTimer::singleShot(2500, msgBox, SLOT(close()));
				return;
			}
			stillConnectedWaitingAnswer = false;
		}
	}

	this->done(QDialog::Accepted);
}
