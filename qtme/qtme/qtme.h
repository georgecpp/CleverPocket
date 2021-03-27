#pragma once

#include <QtWidgets/QMainWindow>
#include "ui_qtme.h"

class qtme : public QMainWindow
{
    Q_OBJECT

public:
    qtme(QWidget *parent = Q_NULLPTR);

private slots:
    void on_pushButton_register_clicked();

private:
    Ui::qtmeClass ui;
};
