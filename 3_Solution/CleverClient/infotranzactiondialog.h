#pragma once
#include <clever_Credentials.h>
#include <QDialog>
#include "ui_infotranzactiondialog.h"

class InfoTranzactionDialog : public QDialog, public Ui::InfoTranzactionDialog
{
	Q_OBJECT

public:
	InfoTranzactionDialog(QWidget *parent = Q_NULLPTR);
	~InfoTranzactionDialog();
};
