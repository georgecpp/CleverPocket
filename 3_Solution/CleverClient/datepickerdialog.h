#pragma once

#include <QDialog>
#include "ui_datepickerdialog.h"

class DatePickerDialog : public QDialog, public Ui::DatePickerDialog
{
	Q_OBJECT

public:
	DatePickerDialog(QWidget *parent = Q_NULLPTR);
	~DatePickerDialog();

private slots:
	void on_okPushButton_clicked();
};
