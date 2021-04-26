#include "rechargecashdialog.h"
#include <qtimer.h>


RechargeCashDialog::RechargeCashDialog(QWidget *parent)
	: QDialog(parent)
{
	setupUi(this);
}

RechargeCashDialog::RechargeCashDialog(std::string userCurrencyISO, std::map<std::string, clever::CardCredentialHandler>& map_cards, const QString& PAT, QWidget* parent)
	:RechargeCashDialog(parent)
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
<<<<<<< Updated upstream
=======
	//
	//insert funds cash
	// 
	//
>>>>>>> Stashed changes
	this->done(QDialog::Accepted);
}