#pragma once

#include <QDialog>
#include "ui_addcarddialog.h"

class AddCardDialog : public QDialog, public Ui::AddCardDialog
{
	Q_OBJECT

public:
	AddCardDialog(QWidget *parent = Q_NULLPTR);
	~AddCardDialog();
};
