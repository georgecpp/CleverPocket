#pragma once

#include <QDialog>
#include "ui_addfundstosavingdialog.h"
#include <clever_Credentials.h>

class AddFundsToSavingDialog : public QDialog, public Ui::AddFundsToSavingDialog
{
	Q_OBJECT
private:
	std::map<std::string, clever::CardCredentialHandler> map_cards;
	std::string currUsername;
public:
	AddFundsToSavingDialog(const std::string& currUsername, std::map<std::string, clever::CardCredentialHandler>& map_cards,QWidget *parent = Q_NULLPTR);
	~AddFundsToSavingDialog();
	std::map<std::string, clever::CardCredentialHandler>& getMapCardsUpdated();
	
private slots:
	void on_financePicker_currentTextChanged(const QString& financeSelected);
	void on_addFundsPushButton_clicked();
};
