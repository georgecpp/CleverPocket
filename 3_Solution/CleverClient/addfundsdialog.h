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
<<<<<<< Updated upstream


=======
	void on_cancelAddFundsPushButton_clicked();
	// care - cancel - push button -ul de la addfundsdialog

private:
	void loadCardCurrencyISO();
>>>>>>> Stashed changes

private:
	std::string currCardName;
	std::string currencyISO;
<<<<<<< Updated upstream
	std::string fundsValue;
	std::string currUsernameLogged;
	std::string currPATLogged;
=======
	QString fundsValue;
	std::string currUsernameLogged;
	QString currPATLogged;
>>>>>>> Stashed changes

	
};
