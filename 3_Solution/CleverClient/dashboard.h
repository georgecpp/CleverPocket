#pragma once
#include <QWidget>
#include "ui_dashboard.h"
#include <qstackedwidget.h>
<<<<<<< Updated upstream
#include <qfiledialog.h>
#include<addfundsdialog.h>
=======
#include <addcarddialog.h>
#include <addfundsdialog.h>
#include <editcarddialog.h>
#include <rechargecashdialog.h>
#include <clever_Credentials.h>
>>>>>>> Stashed changes

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
<<<<<<< Updated upstream
	std::string userCurrencyISO;
=======
	std::string userCashCurrencyISO = "---";
	std::string currUserCash = "0";
>>>>>>> Stashed changes
	//TO DO - preferences
	void logout();
<<<<<<< Updated upstream
	void addFundsExec(AddFundsDialog& adc);
=======
	void loadCards();
	//void loadCash();
	void addCardExec(AddCardDialog& adc);
	void addFundsExec(AddFundsDialog& adf);
	void editCardExec(EditCardDialog& edc);
	void rechargeCashExec(RechargeCashDialog& rcd);
	std::map<std::string, clever::CardCredentialHandler> map_cards;
>>>>>>> Stashed changes
private slots:
	void on_financesCommandLinkButton_clicked();
	void on_menuItemSelected(int index);
	void on_addCardPushButton_clicked();
	void on_choseImagePushButton_clicked();
<<<<<<< Updated upstream
<<<<<<< Updated upstream
	void on_addFundsPushButton_clicked();
	
=======
	void on_financesTabWidget_currentChanged(int index);
	void on_addCashPushButton_clicked();
>>>>>>> Stashed changes
=======
	void on_financesTabWidget_currentChanged(int index);
	void on_addCashPushButton_clicked();
>>>>>>> Stashed changes
private:
	void prepareOptionsComboBox(QComboBox* comboBoxToPrepare);
};