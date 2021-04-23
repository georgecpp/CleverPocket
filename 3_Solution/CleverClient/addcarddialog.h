#pragma once

#include <QDialog>
#include "ui_addcarddialog.h"
#include <vector>

class AddCardDialog : public QDialog, public Ui::AddCardDialog
{
	Q_OBJECT

public:
	AddCardDialog(QWidget *parent = Q_NULLPTR);
	AddCardDialog(const QString& PAT, QWidget* parent = Q_NULLPTR);
	AddCardDialog(const std::string& username, QWidget* parent = Q_NULLPTR);

	~AddCardDialog();

private slots:
	void on_addCardPushButton_clicked();
	void on_validDateEdited();
	void on_cardNumberEdited();
private:
	void loadCurrencyISOS();
private:
	std::vector<std::string> currencyISOS;
	std::string currUsernameLogged;
	std::string currPATLogged;
	QRegExp reg;
	QRegExpValidator val;
};
