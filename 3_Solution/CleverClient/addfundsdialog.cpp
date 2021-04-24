#define _CRT_SECURE_NO_WARNINGS
#include "addfundsdialog.h"
#include "Client.h"
#include <qmessagebox.h>

AddFundsDialog::AddFundsDialog(QWidget *parent)
	: QDialog(parent)
{
	setupUi(this);
}



AddFundsDialog::AddFundsDialog(const QString& cardName, const QString& PAT, QWidget* parent)
	: AddFundsDialog(parent)
{
	this->currCardName = cardName.toStdString();
	this->currPATLogged = PAT.toStdString();
	this->currUsernameLogged = "";
}

AddFundsDialog::AddFundsDialog(const QString& cardName, const std::string& username, QWidget* parent)
	: AddFundsDialog(parent)
{
	this->currCardName = cardName.toStdString();
	this->currUsernameLogged = username;
	this->currPATLogged = "";
}

AddFundsDialog::~AddFundsDialog()
{
}


void AddFundsDialog::on_addFundsPushButton_clicked()
{
	bool stillConnectedWaitingAnswer = true;
	QMessageBox* msgBox = new QMessageBox();
}

