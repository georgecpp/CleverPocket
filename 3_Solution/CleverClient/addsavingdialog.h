#pragma once

#include <QDialog>
#include "ui_addsavingdialog.h"

class AddSavingDialog : public QDialog, public Ui::AddSavingDialog
{
	Q_OBJECT

public:
	AddSavingDialog(const std::string& dateToday, const std::string& usernameLogged, QWidget *parent = Q_NULLPTR);
	~AddSavingDialog();
private slots:
	void on_addSavingPushButton_clicked();
private:
	std::string username;
	std::string date;
};
