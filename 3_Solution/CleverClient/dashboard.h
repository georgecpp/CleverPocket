#pragma once
#include <QWidget>
#include "ui_dashboard.h"
#include <qstackedwidget.h>
#include <addcarddialog.h>
#include <addfundsdialog.h>
#include <editcarddialog.h>
#include <rechargecashdialog.h>
#include <clever_Credentials.h>

class Dashboard : public QWidget, public Ui::Dashboard
{
	Q_OBJECT

public:
	Dashboard(QStackedWidget* parentStackedWidget = Q_NULLPTR, QWidget* parent = Q_NULLPTR);
	Dashboard(const QString& PAT, QStackedWidget* parentStackedWidget = Q_NULLPTR, QWidget* parent = Q_NULLPTR);
	Dashboard(const std::string& username, QStackedWidget* parentStackedWidget = Q_NULLPTR, QWidget* parent = Q_NULLPTR);
	~Dashboard();
private:
	Ui::Dashboard ui;
	QStackedWidget* fromStackedWidget;
	QString PATLoggedIn;
	std::string usernameLoggedIn;
	std::string userCashCurrencyISO;
	std::string currUserCash;
	void logout();
	void loadCards();
	void loadCash();
	void addCardExec(AddCardDialog& adc);
	void addFundsExec(AddFundsDialog& adf);
	void editCardExec(EditCardDialog& edc);
	std::map<std::string, clever::CardCredentialHandler> map_cards;
private slots:
	void on_financesCommandLinkButton_clicked();
	void on_menuItemSelected(int index);
	void on_addCardPushButton_clicked();
	void on_addFundsPushButton_clicked();
	void on_editCardPushButton_clicked();
	void on_cardSelected();
	void on_choseImagePushButton_clicked();
	void on_financesTabWidget_currentChanged(int index);
	void on_addCashPushButton_clicked();
private:
	void prepareOptionsComboBox(QComboBox* comboBoxToPrepare);
	void rechargeCashExec(RechargeCashDialog& rcd);
};