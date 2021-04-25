#pragma once
#include <QWidget>
#include "ui_dashboard.h"
#include <qstackedwidget.h>
#include <qfiledialog.h>
#include<addfundsdialog.h>

class Dashboard : public QWidget, public Ui::Dashboard
{
	Q_OBJECT

public:
	Dashboard(QStackedWidget* parentStackedWidget = Q_NULLPTR, QWidget* parent = Q_NULLPTR);
	~Dashboard();
private:
	Ui::Dashboard ui;
	QStackedWidget* fromStackedWidget;
	QString PATLoggedIn;
	std::string usernameLoggedIn;
	void logout();
	void addFundsExec(AddFundsDialog& adc);
private slots:
	void on_financesCommandLinkButton_clicked();
	void on_menuItemSelected(int index);
	void on_addCardPushButton_clicked();
	void on_choseImagePushButton_clicked();
	void on_addFundsPushButton_clicked();
	
private:
	void prepareOptionsComboBox(QComboBox* comboBoxToPrepare);
};