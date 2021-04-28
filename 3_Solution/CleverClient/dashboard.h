#pragma once
#include <QWidget>
#include "ui_dashboard.h"
#include <qstackedwidget.h>
#include <addcarddialog.h>
#include <addfundsdialog.h>
#include <editcarddialog.h>
#include <rechargecashdialog.h>
#include <datepickerdialog.h>
#include <clever_Credentials.h>
#include <tranzactionrowitem.h>

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
	std::map<std::string, clever::CardCredentialHandler> map_cards;
	clever::CredentialHandler user_information;
	std::string userCashCurrencyISO;
	std::string dailyMailState;
	std::string currUserCash;
	std::string profilePicture;
	std::vector<std::string> currencyISOS;
	std::vector<clever::TranzactionHandler> all_user_tranzactions;
private:
	void logout();
	void loadCash();
	void loadCards();
	void loadCurrencyISOS();
	void loadTranzactionsHistory();
	void addCardExec(AddCardDialog& adc);
	void addFundsExec(AddFundsDialog& adf);
	void editCardExec(EditCardDialog& edc);
	void rechargeCashExec(RechargeCashDialog& rcd);
	void toggleTranzactionsButtons(int state);
	void populateTranzactionsFinanceType();
	void insertBDProfiePicture(std::string& hexImg);
private slots:
	void on_financesCommandLinkButton_clicked();
	void on_menuItemSelected(int index);
	void on_financesTabWidget_currentChanged(int index);
	void on_addCardPushButton_clicked();
	void on_addFundsPushButton_clicked();
	void on_editCardPushButton_clicked();
	void on_cardSelected();
	void on_choseImagePushButton_clicked();
	void on_addCashPushButton_clicked();
	void on_transactionsCommandLinkButton_clicked();
	void on_pickDatePushButton_clicked();
	void on_TranzactionTypeSelected();
	void on_spendingsCategoriesPushButton_clicked();
	void on_backToTranzactionsPushButtton_clicked();
	void on_showFinanceHistory_clicked();
	void on_savePreferencesButton_clicked();
private:
	void prepareOptionsComboBox(QComboBox* comboBoxToPrepare);
};