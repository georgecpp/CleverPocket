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
#include <infotranzactiondialog.h>
#include <addincomedialog.h>
#include <addoutcomedialog.h>


enum class TranzactionFilters
{
	TranzactionFinanceName,
	TranzactionType,
	TranzactionDate,
};

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
	std::string userCashCurrencyISO;
	std::string currUserCash;
	std::vector<std::string> currencyISOS;
	std::vector<clever::TranzactionHandler> all_user_tranzactions;
	std::map<std::string, clever::FinanceTypeCredentialHandler> map_recurencies;
	clever::CredentialHandler user_information;
	std::string dailyMailState;
	std::string profilePicture;
private:
	void logout();
	void loadCash();
	void loadCards();
	void loadCurrencyISOS();
	void loadTranzactionsHistory();
	void loadRecurencies();
	void loadRecurenciesComboBoxes();
	void addIncomeExec(AddIncomeDialog& adi);
	void addOutcomeExec(AddOutComeDialog& ado);
	void filterTranzactionsBy(TranzactionFilters filterApplied);
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
	void on_tranzactionTypePicker_currentTextChanged(const QString& newText);
	void on_financeTypePicker_currentTextChanged(const QString& newText);
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
	void on_listWidget_itemClicked(QListWidgetItem* item);
	void on_recurenciesSelected();
	void on_addIncomePushButton_clicked();
	void on_periodicallyIncomePushButton_clicked();
	void on_incomeBackToTranzactionsPushButton_clicked();
	void on_recurrentSpendingsCommandLInkButton_clicked();
	void on_outcomeBackToCategoriesPushButton_clicked();
	void on_addoutcomePushButton_clicked();
private:
	void prepareOptionsComboBox(QComboBox* comboBoxToPrepare);
private:
	std::string currFinanceName;
	std::string currTranzType;
	std::string currTranzDate;
private:
	void updateTranzactionsDefault();
	void updateTranzactionsByFilters(std::string FinanceName, std::string TranzactionType, std::string TranzactionDate);	
	clever::TranzactionHandler getTranzactionByCurrentItem(const char* trTitle, const char* trTimestamp);
};