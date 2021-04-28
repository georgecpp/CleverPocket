#include "datepickerdialog.h"

DatePickerDialog::DatePickerDialog(QWidget *parent)
	: QDialog(parent)
{
	setupUi(this);
}

DatePickerDialog::~DatePickerDialog()
{

}

void DatePickerDialog::on_okPushButton_clicked()
{
	// check if calendar is picked.
	this->done(QDialog::Accepted);
}
