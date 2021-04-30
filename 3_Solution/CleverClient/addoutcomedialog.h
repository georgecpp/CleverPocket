#pragma once

#include <QDialog>
#include "ui_addoutcomedialog.h"
#include <clever_Credentials.h>

class AddOutComeDialog : public QDialog, public Ui::AddOutComeDialog
{
	Q_OBJECT

public:
	AddOutComeDialog(QWidget* parent = Q_NULLPTR);
	AddOutComeDialog(std::map<std::string, clever::CardCredentialHandler>& map_cards, const std::string& username, QWidget* parent = Q_NULLPTR);

	~AddOutComeDialog();
	std::map<std::string, clever::CardCredentialHandler>& getCardMap();
private slots:
	void on_addPeriodicallyOutcomePushButton_clicked();
	void on_cancelAddOutcomePushButton_clicked();
	void on_cardsOutcomeComboBox_currentTextChanged(const QString& cardNameSelected);
private:
	std::string currUsernameLogged;
	QString currPATLogged;
	std::map<std::string, clever::CardCredentialHandler> map_cards;
};