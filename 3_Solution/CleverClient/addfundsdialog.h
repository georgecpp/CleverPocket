#pragma once
#include <QDialog>
#include "ui_addfundsdialog.h"
class AddFundsDialog : public QDialog, public Ui::AddFundsDialog
{
	Q_OBJECT
public:
	AddFundsDialog(QWidget* parent = Q_NULLPTR);
	AddFundsDialog(const QString& cardName, const QString& currencyISO, const QString& PAT, QWidget* parent = Q_NULLPTR);
	AddFundsDialog(const QString& cardName, const QString& currencyISO, const std::string& username, QWidget* parent = Q_NULLPTR);
	~AddFundsDialog();

private slots:
	void on_addFundPushButton_clicked();
	void on_cancelAddFundsPushButton_clicked();
private:
	std::string currCardName;
	std::string currencyISO;
	QString fundsValue;
	std::string currUsernameLogged;
	QString currPATLogged;
};