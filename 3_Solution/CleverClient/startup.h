#pragma once
//#include "client_backend.h"
#include <QtWidgets/QMainWindow>
#include "ui_startup.h"

class Startup : public QWidget
{
	Q_OBJECT

public:
	Startup(QWidget *parent = Q_NULLPTR);
	~Startup();
private:
	Ui::Startup ui;
	//Client c;
private slots:
	void on_registerButton_clicked();
};
