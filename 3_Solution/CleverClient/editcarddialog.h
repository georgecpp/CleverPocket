#pragma once

#include <QDialog>
#include "ui_editcarddialog.h"
#include <vector>

class EditCardDialog : public QDialog, public Ui::EditCardDialog
{
	Q_OBJECT

public:
	EditCardDialog(QWidget *parent = Q_NULLPTR);
	EditCardDialog(const QString& PAT, QWidget* parent = Q_NULLPTR);
	EditCardDialog(const std::string& username, QWidget* parent = Q_NULLPTR);
	void setOldCardName(std::string oldcardname);
	~EditCardDialog();

private slots:
	void on_editCardPushButton_clicked();
	void on_cancelEditCardPushButton_clicked();
	void on_validDateEdited();
	void on_cardNumberEdited();
private:
	void loadCurrencyISOS();
private:
	std::vector<std::string> currencyISOS;
	std::string currUsernameLogged;
	std::string currPATLogged;
	std::string oldCardName;
	QRegExp reg;
	QRegExpValidator val;
};
