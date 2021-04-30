#pragma once

#include <QDialog>
#include "ui_addincomedialog.h"
#include <clever_Credentials.h>

class AddIncomeDialog : public QDialog, public Ui::AddIncomeDialog
{
	Q_OBJECT

public:
	AddIncomeDialog(QWidget* parent = Q_NULLPTR);
	AddIncomeDialog(std::map<std::string, clever::CardCredentialHandler>& map_cards, const QString& PAT, QWidget* parent = Q_NULLPTR);
	AddIncomeDialog(std::map<std::string, clever::CardCredentialHandler>& map_cards, const std::string& username, QWidget* parent = Q_NULLPTR);

	~AddIncomeDialog();
	std::map<std::string, clever::CardCredentialHandler>& getCardMap();
private slots:
	void on_addPeriodicallyIncomePushButton_clicked();
	void on_cancelAddIncomePushButton_clicked();
private:
	std::string currUsernameLogged;
	QString currPATLogged;
	std::map<std::string, clever::CardCredentialHandler> map_cards;
};
