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
#include<addspendingsdialog.h>
#include <addfundstosavingdialog.h>
#include <addsavingdialog.h>
#include <QtCharts/qpieseries.h>
#include <QtCharts/qchart.h>
#include <QtCharts/qchartview.h>


using namespace QtCharts;


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
	std::pair<std::string, std::string> cash_details;
	std::vector<std::string> currencyISOS;
	std::vector<clever::TranzactionHandler> all_user_tranzactions;
	std::map<std::string, clever::FinanceTypeCredentialHandler> map_recurencies;
	std::map<std::string, clever::SavingHandler> map_savings;
	clever::CredentialHandler user_information;
	clever::BudgetHandler user_budget;
	std::string dailyMailState;
	std::string profilePicture = "Images/img3.jpg";
	bool currentlyOnBudget;
	std::map<int, std::string> month_code_map;
	std::map<std::string, std::string> endMonthsContextMap;
	std::vector<std::string> currLastSixMonths;
	QBarSet* setIncome;
	QBarSet* setOutcome;
	QLineSeries* lineSeries;

private:
	void logout();
	void loadCash();
	void loadCards();
	void loadCurrencyISOS();
	void loadTranzactionsHistory();
	void loadRecurencies();
	void loadRecurenciesComboBoxes();
	void loadSavings();
	void loadBudgets();
	void addIncomeExec(AddIncomeDialog& adi);
	void addOutcomeExec(AddOutComeDialog& ado);
	void addSpendingExec(AddSpendingsDialog& ads);
	void filterTranzactionsBy(TranzactionFilters filterApplied);
	void addCardExec(AddCardDialog& adc);
	void addFundsExec(AddFundsDialog& adf);
	void editCardExec(EditCardDialog& edc);
	void rechargeCashExec(RechargeCashDialog& rcd);
	void toggleTranzactionsButtons(int state);
	void populateTranzactionsFinanceType();
	void insertBDProfiePicture(std::string& filename);
	void load_spendingsTotals();
	void init_statistics();
private slots:
	void on_financesCommandLinkButton_clicked();
	void on_menuItemSelected(int index);
	void on_financesTabWidget_currentChanged(int index);
	void on_tranzactionTypePicker_currentTextChanged(const QString& newText);
	void on_financeTypePicker_currentTextChanged(const QString& newText);
	void on_savingPicker_currentTextChanged(const QString& SavingChosen);
	void on_addSavingPushButton_clicked();
	void on_addFundsSavingPushButton_clicked();
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
	void on_transportCommandLinkButton_clicked();
	void on_educationCommandLinkButton_clicked();
	void on_shoppingCommandLinkButton_clicked();
	void on_freeTimeCommandLinkButton_clicked();
	void on_healthCommandLinkButton_clicked();
	void on_holidayCommandLinkButton_clicked();
	void on_goalsCommandLinkButton_clicked();
	void on_savingsCommandLinkButton_clicked();
	void on_budgetCommandLinkButton_clicked();
	void on_savingsbackToGoalsPushButton_clicked();
	void on_budgetbackToGoalsPushButton_clicked();
	void on_backToDashboardGoalsLinkButton_clicked();
	void on_budgetpickStartDatePushButton_clicked();
	void on_budgetpickEndDatePushButton_clicked();
	void on_budgetSetPushButton_clicked();
	void on_deleteBudgetPushButton_clicked();
	void on_statisticsCommandLinkButton_clicked();
	void on_setIncome_hovered(bool status, int index);
	void on_setOutcome_hovered(bool status, int index);
	void on_lineSeries_hovered(const QPointF& point, bool state);
	void on_investmentsCommandLinkButton_clicked();
	void on_calculatePushButton_1_clicked();
	void on_calculatePushButton_2_clicked();

private:
	void prepareOptionsComboBox(QComboBox* comboBoxToPrepare);
	void prepareAllOptionsComboBox();
	void prepareCapitalLabelWithValue(QLabel* capitalLabelToPrepare, std::string& stringWith);
	void refreshCash();
	void turnIntoBudgetSet(bool isUserOnBudget);
	std::string getCurrentDateTime();
	std::string getFloatText2Decimal(float value);
	float generateIncomeForMonth(std::string monthCode);
	float generateOutcomeForMonth(std::string monthCode);
	void initMonthCodeMap();
	std::map<int, std::string>& getMonthCodeMap();
	std::map<std::string, std::string>& getEndMonthCodeMap();

	float getTransactionsIncomeForMonthCode(int monthCodeInt);
	float getTransactionsOutcomeForMonthCode(int monthCodeInt);
	float generateCapitalForMonth(std::string monthCode);

private:
	std::string currFinanceName;
	std::string currTranzType;
	std::string currTranzDate;
private:
	void updateTranzactionsDefault();
	void updateTranzactionsByFilters(std::string FinanceName, std::string TranzactionType, std::string TranzactionDate);	
	clever::TranzactionHandler getTranzactionByCurrentItem(const char* trTitle, const char* trTimestamp);
};