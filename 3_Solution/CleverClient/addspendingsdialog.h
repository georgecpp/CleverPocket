#pragma once

#include <QDialog>
#include "ui_addspendingsdialog.h"
#include <clever_Credentials.h>

class AddSpendingsDialog : public QDialog, public Ui::AddSpendingsDialog
{
	Q_OBJECT

public:
	AddSpendingsDialog(QWidget *parent = Q_NULLPTR);
	AddSpendingsDialog(std::string& categoryName,std::pair<std::string, std::string>& cash_details, std::map<std::string, clever::CardCredentialHandler>& map_cards, const std::string& username, QWidget* parent = Q_NULLPTR);
	~AddSpendingsDialog();

private slots:
	void on_categoryFinancePicker_currentTextChanged(const QString& financeSelected);
	void on_addSpendingsPushButton_clicked();
	void on_cancelAddSpendingsPushButton_clicked();
private:
	std::string currUsernameLogged;
	std::map<std::string, clever::CardCredentialHandler> map_cards;
	std::pair<std::string, std::string> cash_details;
};
