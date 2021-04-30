#define _CRT_SECURE_NO_WARNINGS
#include "addcarddialog.h"
#include "Client.h"
#include <qmessagebox.h>
#include <qtimer.h>
#include <time.h>


AddCardDialog::AddCardDialog(QWidget *parent)
	: QDialog(parent), reg("[A-Z- ]{1,50}"), val(reg,0)
{
	setupUi(this);
	loadCurrencyISOS();
	// only capital letters to card holder line edit.
	this->cardHolderLineEdit->setValidator(&val);
	this->cardNumberLineEdit->setMaxLength(19);
	this->validUntilLineEdit->setMaxLength(5);
	this->validUntilLineEdit->setCursorPosition(0);
	this->validUntilLineEdit->insert("/");
	connect(this->validUntilLineEdit, SIGNAL(textEdited(QString)), this, SLOT(on_validDateEdited()));
	connect(this->cardNumberLineEdit, SIGNAL(textEdited(QString)), this, SLOT(on_cardNumberEdited()));
}

AddCardDialog::AddCardDialog(const QString& PAT, QWidget* parent)
	:AddCardDialog(parent)
{
	this->currPATLogged = PAT.toStdString();
	this->currUsernameLogged = "";
}

AddCardDialog::AddCardDialog(const std::string& username, QWidget* parent)
	:AddCardDialog(parent)
{
	this->currUsernameLogged = username;
	this->currPATLogged = "";
}

AddCardDialog::~AddCardDialog()
{

}

void AddCardDialog::on_validDateEdited()
{
	/*if (this->validUntilLineEdit->cursorPosition() == 2)
	{
		this->validUntilLineEdit->insert("/");
	}*/
}

void AddCardDialog::on_cardNumberEdited()
{
	if ((this->cardNumberLineEdit->cursorPosition() + 1) % 5 == 0)
	{
		this->cardNumberLineEdit->insert(" ");
	}
}

void AddCardDialog::loadCurrencyISOS()
{
	// load CurrencyISO from resource text file into combobox.
	FILE* fin = fopen("currencyISO.txt", "r");
	if (fin)
	{
		char buffer[256];
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
			this->currencyISOS.push_back(line);
		}
		fclose(fin);
		this->isoCurrencyComboBox->clear();
		for (std::vector<std::string>::iterator it = currencyISOS.begin(); it != currencyISOS.end(); it++)
		{
			this->isoCurrencyComboBox->addItem(it->c_str());
		}
	}
}

void AddCardDialog::on_addCardPushButton_clicked()
{
	bool stillConnectedWaitingAnswer = true;
	QMessageBox* msgBox = new QMessageBox();

	QString cardName = this->cardNameLineEdit->text();
	QString cardHolder = this->cardHolderLineEdit->text();
	QString cardNumber = this->cardNumberLineEdit->text();
	QString currencyISO = this->isoCurrencyComboBox->currentText();
	QString validUntil = this->validUntilLineEdit->text();

	// validation stage.

	// 1. all boxes are filled.
	if (cardName == "" || cardHolder == "" || cardNumber == "" || currencyISO == "" || validUntil == "")
	{
		msgBox->setText("Please fill all the boxes!");
		msgBox->show();
		QTimer::singleShot(2250, msgBox, SLOT(close()));
		return;
	}

	// 2. Card Number has exactly 16 chars. (16 chars and 3 space bars -- 19 total chars.)
	if (cardNumber.length() != 19)
	{
		msgBox->setText("Invalid card number!");
		msgBox->show();
		QTimer::singleShot(2250, msgBox, SLOT(close()));
		return;
	}

	// 3. check card is expired. -- year < actual year and if not, month < actual month.
	time_t now = time(0);
	tm* ltm = localtime(&now);
	int year = ltm->tm_year + 1900;
	int month = ltm->tm_mon + 1;
	std::string dateFilled = validUntil.toStdString();
	if (std::stoi(dateFilled.substr(dateFilled.find('/')+1)) < (year-2000))
	{
		msgBox->setText("Card is expired! Year out of date.");
		msgBox->show();
		QTimer::singleShot(2250, msgBox, SLOT(close()));
		return;
	}
	else
	{
		if (std::stoi(dateFilled.substr(dateFilled.find('/') + 1)) == (year - 2000))
		{
			if (std::stoi(dateFilled.substr(0, 2)) < month)
			{
				msgBox->setText("Card is expired! Month out of date");
				msgBox->show();
				QTimer::singleShot(2250, msgBox, SLOT(close()));
				return;
			}
		}
	}

	// request to server to register this card belonging to this user currently logged in.
	clever::CardCredentialHandler cardCredHandler(cardName.toStdString(), cardHolder.toStdString(), cardNumber.toStdString(),currencyISO.toStdString(),validUntil.toStdString(),0.0f);
	Client& c = Client::getInstance();
	c.Incoming().clear();
	if (this->currUsernameLogged == "")
	{
		c.UserPATAddCard(this->currPATLogged, cardCredHandler);
	}
	else
	{
		c.UsernameAddCard(this->currUsernameLogged, cardCredHandler);
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
		if (msg.header.id == clever::MessageType::ServerAddCardResponse)
		{
			// server has responded back to add card request.
			char responseBack[1024];
			msg >> responseBack;
			if (strcmp(responseBack, "SuccessAddCard") == 0)
			{
				// ALL GOOD.
				msgBox->setText("Card added with success!");
				msgBox->show();
				QTimer::singleShot(2500, msgBox, SLOT(close()));
			}
			else
			{
				msgBox->setText("Couldn't add the card! Server problem. Try again.");
				msgBox->show();
				QTimer::singleShot(2500, msgBox, SLOT(close()));
				return;
			}
			stillConnectedWaitingAnswer = false;
		}
	}
	this->done(QDialog::Accepted);
}
