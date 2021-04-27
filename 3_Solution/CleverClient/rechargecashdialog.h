#pragma once

#include <QDialog>
#include "ui_rechargecashdialog.h"
#include <clever_Credentials.h>
#include<qmessagebox.h>

class RechargeCashDialog : public QDialog, public Ui::RechargeCashDialog
{
	Q_OBJECT

public:
	RechargeCashDialog(QWidget* parent = Q_NULLPTR);
	RechargeCashDialog(std::string userCurrencyISO, std::map<std::string, clever::CardCredentialHandler>& map_cards, const QString& PAT, QWidget* parent = Q_NULLPTR);
	RechargeCashDialog(std::string userCurrencyISO, std::map<std::string, clever::CardCredentialHandler>& map_cards, const std::string& username, QWidget* parent = Q_NULLPTR);
	~RechargeCashDialog();
	std::map<std::string, clever::CardCredentialHandler>& getCardMap();
private slots:
	void on_rechargePushButton_clicked();
	void on_cancelRechargePushButton_clicked();

private:
	std::string userCurrencyISO;
	QString cashValue;
	std::string currUsernameLogged;
	QString currPATLogged;
	std::map<std::string, clever::CardCredentialHandler> map_cards;
};