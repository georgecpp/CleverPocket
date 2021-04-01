#pragma once
//#include "client_backend.h"
#include <QtWidgets/QMainWindow>
#include "ui_startup.h"
#include <qstackedwidget.h>
#include <QtGui>


class Startup : public QWidget
{
	Q_OBJECT

public:
	Startup(QWidget *parent = Q_NULLPTR);
	~Startup();
private:
	Ui::Startup ui;
	void resizeToLoginPage();
	void resizeToRegisterPage();
	void resizeToForgotPasswordPage();
	//Client c;
private slots:
	void on_registerLinkButton_clicked();
	void on_alreadyRegisteredLinkButton_clicked();
	void on_registerPushButton_clicked();
};
