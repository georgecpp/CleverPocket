#pragma once

#include <QDialog>
#include "ui_editcarddialog.h"

class EditCardDialog : public QDialog, public Ui::EditCardDialog
{
	Q_OBJECT

public:
	EditCardDialog(QWidget *parent = Q_NULLPTR);
	~EditCardDialog();
};
