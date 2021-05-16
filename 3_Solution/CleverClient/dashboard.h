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
#include <addincomedialog.h>
#include <addoutcomedialog.h>
<<<<<<< Updated upstream
=======
#include<addspendingsdialog.h>
#include <addfundstosavingdialog.h>
#include <addsavingdialog.h>
#include <QtCharts/qpieseries.h>
#include <QtCharts/qchart.h>
#include <QtCharts/qchartview.h>
#include <QtCharts/qlineseries.h>



using namespace QtCharts;


enum class TranzactionFilters
{
	TranzactionFinanceName,
	TranzactionType,
	TranzactionDate,
};
>>>>>>> Stashed changes

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
	std::map<std::string, clever::FinanceTypeCredentialHandler> map_recurencies;
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
	void loadRecurencies();
	void loadCurrencyISOS();
	void loadRecurenciesComboBoxes();
	void loadTranzactionsHistory();
	void addCardExec(AddCardDialog& adc);
	void addIncomeExec(AddIncomeDialog& adi);
	void addOutcomeExec(AddOutComeDialog& ado);
	void addFundsExec(AddFundsDialog& adf);
	void editCardExec(EditCardDialog& edc);
	void rechargeCashExec(RechargeCashDialog& rcd);
	void toggleTranzactionsButtons(int state);
	void populateTranzactionsFinanceType();
	void insertBDProfiePicture(std::string &filename);
	void load_spendingsTotals();
private slots:
	void on_financesCommandLinkButton_clicked();
	void on_menuItemSelected(int index);
	void on_financesTabWidget_currentChanged(int index);
	void on_addCardPushButton_clicked();
	void on_addFundsPushButton_clicked();
	void on_editCardPushButton_clicked();
	void on_cardSelected();
	void on_recurenciesSelected();
	void on_choseImagePushButton_clicked();
	void on_addCashPushButton_clicked();
	void on_transactionsCommandLinkButton_clicked();
	void on_pickDatePushButton_clicked();
	void on_TranzactionTypeSelected();
	void on_spendingsCategoriesPushButton_clicked();
	void on_backToTranzactionsPushButtton_clicked();
	void on_showFinanceHistory_clicked();
	void on_savePreferencesButton_clicked();
	void on_addIncomePushButton_clicked();
	void on_periodicallyIncomePushButton();
	void on_incomeBackToTranzactionsPushButton_clicked();
	void on_recurrentSpendingsCommandLInkButton_clicked();
	void on_outcomeBackToCategoriesPushButton_clicked();
	void on_addoutcomePushButton_clicked();
<<<<<<< Updated upstream
=======
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
	void on_calculatePushButton_1_clicked();
	void on_calculatePushButton_2_clicked();

>>>>>>> Stashed changes
private:
	void prepareOptionsComboBox(QComboBox* comboBoxToPrepare);
};