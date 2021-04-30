#pragma once

#include <QDialog>
#include "ui_addspendingsdialog.h"

class AddSpendingsDialog : public QDialog, public Ui::AddSpendingsDialog
{
	Q_OBJECT

public:
	AddSpendingsDialog(QWidget *parent = Q_NULLPTR);
	~AddSpendingsDialog();
};
