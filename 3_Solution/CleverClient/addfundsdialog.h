#pragma once

#include <QDialog>
#include "ui_addfundsdialog.h"

class AddFundsDialog : public QDialog, public Ui::AddFundsDialog
{
	Q_OBJECT

public:
	AddFundsDialog(QWidget *parent = Q_NULLPTR);
	~AddFundsDialog();
};
