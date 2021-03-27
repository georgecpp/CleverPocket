#include "qtme.h"
#include "stdafx.h"
#include <qmessagebox.h>

qtme::qtme(QWidget *parent)
    : QMainWindow(parent)
{
    ui.setupUi(this);
}

void qtme::on_pushButton_register_clicked()
{
    QString username = ui.line_username->text();
    QString password = ui.line_password->text();
    QString email = ui.line_email->text();

    if (username == "test" && password == "test" && email == "test")
    {
        QMessageBox::information(this, "Register", "sunt corecte");
    }
    else
    {
        QMessageBox::warning(this, "Register", "Nu sunt corecte");
    }
}