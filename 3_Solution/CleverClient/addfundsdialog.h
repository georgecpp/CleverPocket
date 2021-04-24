#pragma once

#include <QDialog>
#include "ui_addfundsdialog.h"

class AddFundsDialog : public QDialog, public Ui::AddFundsDialog
{
	Q_OBJECT

public:
	AddFundsDialog(QWidget *parent = Q_NULLPTR);
	AddFundsDialog(const QString& cardName, const QString& PAT, QWidget* parent = Q_NULLPTR);
	AddFundsDialog(const QString& cardName, const std::string& username, QWidget* parent = Q_NULLPTR);
	~AddFundsDialog();

private slots:
	void on_addFundsPushButton_clicked();



private:
	std::string currCardName;
	std::string currencyISO;
	std::string fundsValue;
	std::string currUsernameLogged;
	std::string currPATLogged;

	
};
