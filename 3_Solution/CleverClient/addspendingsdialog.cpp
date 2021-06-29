#define _CRT_SECURE_NO_WARNINGS
#include "addspendingsdialog.h"
#include "Client.h"
#include <qmessagebox.h>
#include <qtimer.h>
#include <time.h>
#include <vector>

AddSpendingsDialog::AddSpendingsDialog(QWidget *parent)
	: QDialog(parent)
{
	setupUi(this);
	this->financeISOLineEdit->setReadOnly(true);
}

AddSpendingsDialog::AddSpendingsDialog(std::string& categoryName, std::pair<std::string, std::string>& cash_details, std::map<std::string, clever::CardCredentialHandler>& map_cards, const std::string& username, QWidget* parent)
	: AddSpendingsDialog(parent)
{
	this->cash_details = cash_details;
	this->map_cards = map_cards;
	this->currUsernameLogged = username;
	for (auto& it : map_cards)
	{
		this->categoryFinancePicker->addItem(it.second.getCardName());
	}
	this->categoryFinancePicker->addItem("Cash");
	this->categoryNameLineEdit->setText(categoryName.c_str());
	this->categoryFinancePicker->setCurrentIndex(0);
}

AddSpendingsDialog::~AddSpendingsDialog()
{
	
}

void AddSpendingsDialog::on_addSpendingsPushButton_clicked()
{
	QMessageBox* msgBox = new QMessageBox();
	std::vector<std::string> spending_details;
	QString categoryName = this->categoryNameLineEdit->text();
	spending_details.push_back(categoryName.toStdString());
	QString tranzValue = this->categoryTranzactionValueLineEdit->text();
	spending_details.push_back(tranzValue.toStdString());
	QString tranzDestination = this->categoryTranzactionDestinationLineEdit->text();
	spending_details.push_back(tranzDestination.toStdString());
	QString tranzDetails = this->categoryTranzactionDetailsLineEdit->text();
	spending_details.push_back(tranzDetails.toStdString());
	QString tranzCurrencyISO = this->financeISOLineEdit->text();
	spending_details.push_back(tranzCurrencyISO.toStdString());
	QString tranzFinanceType = this->categoryFinancePicker->currentText();
	spending_details.push_back(tranzFinanceType.toStdString());
	QString tranzTitle = this->categoryTranzactionTitleLineEdit->text();
	spending_details.push_back(tranzTitle.toStdString());
	
	//fill the boxes
	if (tranzValue == "" || tranzDestination == "" || tranzDetails == "" || tranzTitle == "")
	{
		msgBox->setText("Please fill all the boxes!");
		msgBox->show();
		QTimer::singleShot(2250, msgBox, SLOT(close()));
		return;
	}
	//value spend higher than the finance current sold
	if (tranzFinanceType == "Cash")
	{
		if (tranzValue.toFloat() > std::stof(cash_details.first))
		{
			msgBox->setText("The finance selected has not enough money! Please select another one!");
			msgBox->show();
			QTimer::singleShot(2250, msgBox, SLOT(close()));
			return;
		}
	}
	else 
	{
		if (tranzValue.toFloat() > this->map_cards[tranzFinanceType.toStdString()].getCardSold())
		{
			msgBox->setText("The finance selected has not enough money! Please select another one!");
			msgBox->show();
			QTimer::singleShot(2250, msgBox, SLOT(close()));
			return;
		}
	}

	bool stillConnectedWaitingAnswer = true;
	Client& c = Client::getInstance();
	c.Incoming().clear();
	c.UsernameAddSpendings(this->currUsernameLogged, spending_details);
	while (c.Incoming().empty())
	{
		if(!c.IsConnected())
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
		if (msg.header.id == clever::MessageType::ServerSpendingsResponse)
		{
			// server has responded back to add spendings request.
			char responseBack[1024];
			msg >> responseBack;
			if (strcmp(responseBack, "SuccessAddSpendings") == 0)
			{
				// ALL GOOD.
				msgBox->setText("Spendings added with success!");
				msgBox->show();
				QTimer::singleShot(2500, msgBox, SLOT(close()));
			}
			else
			{
				msgBox->setText("Couldn't add spendings! Server problem. Try again.");
				msgBox->show();
				QTimer::singleShot(2500, msgBox, SLOT(close()));
				return;
			}
			stillConnectedWaitingAnswer = false;
		}
	}


	this->done(QDialog::Accepted);
}

void AddSpendingsDialog::on_cancelAddSpendingsPushButton_clicked()
{
	this->done(QDialog::Rejected);
}

void AddSpendingsDialog::on_categoryFinancePicker_currentTextChanged(const QString& financeSelected)
{
	if (this->categoryFinancePicker->currentText() == "Cash")
	{
		this->financeISOLineEdit->setText(cash_details.second.c_str());
	}
	else
	{
		this->financeISOLineEdit->setText(this->map_cards[this->categoryFinancePicker->currentText().toStdString()].getCardCurrencyISO());
	}
}
