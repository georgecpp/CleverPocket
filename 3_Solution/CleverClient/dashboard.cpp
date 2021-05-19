#include "Client.h"
#include "dashboard.h"
#include <qmessagebox.h>
#include <qtimer.h>
#include <qfiledialog.h>
#include <iomanip>
#include <qbuffer.h>

Dashboard::Dashboard(QStackedWidget* parentStackedWidget, QWidget* parent)
	: QWidget(parent)
{
	this->userCashCurrencyISO = "";
	this->currUserCash = "";
	this->fromStackedWidget = parentStackedWidget;
	ui.setupUi(this);
	ui.stackedWidget->setCurrentWidget(ui.dashboardPage);
	connect(ui.cardPicker, SIGNAL(activated(int)), this, SLOT(on_cardSelected()));
	connect(ui.tranzactionTypePicker, SIGNAL(activated(int)), this, SLOT(on_TranzactionTypeSelected()));
	connect(ui.incomePicker, SIGNAL(activated(int)), this, SLOT(on_recurenciesSelected()));
	connect(ui.outcomePicker, SIGNAL(activated(int)), this, SLOT(on_recurenciesSelected()));
	toggleTranzactionsButtons(0);
	ui.statisticsTabWidget->setTabPosition(QTabWidget::TabPosition::South);
	initMonthCodeMap();
}

Dashboard::Dashboard(const QString& PAT, QStackedWidget* parentStackedWidget, QWidget* parent)
	: Dashboard(parentStackedWidget, parent)
{
	this->PATLoggedIn = PAT;
	this->usernameLoggedIn = "";
	loadCash();
	prepareAllOptionsComboBox();
	loadCards();
	loadRecurencies();
	loadCurrencyISOS();
	ui.preferenceCurrencyComboBox->setCurrentText(QString(userCashCurrencyISO.c_str()));
	loadTranzactionsHistory();
	loadSavings();
	loadBudgets();
	init_statistics();
}

Dashboard::Dashboard(const std::string& username, QStackedWidget* parentStackedWidget, QWidget* parent)
	: Dashboard(parentStackedWidget, parent)
{
	this->usernameLoggedIn = username;
	this->PATLoggedIn = "";
	loadCards();
	loadCash();
	loadRecurencies();
	prepareAllOptionsComboBox();
	loadCurrencyISOS();
	ui.preferenceCurrencyComboBox->setCurrentText(QString(userCashCurrencyISO.c_str()));
	loadTranzactionsHistory();
	loadSavings();
	loadBudgets();
	init_statistics();
}

Dashboard::~Dashboard()
{

}

void Dashboard::logout()
{
	// check if PAT file exists.
	FILE* fin = fopen("PAT.txt", "r");
	if (fin)
	{
		// if so, request from server to logout with remembered PAT -- delete record in sessions for that PAT.
		char buffer[21];
		fgets(buffer, sizeof(buffer), fin);
		std::string PAT = buffer;
		bool stillConnectedWaitingForAnswer = true;
		QMessageBox* msgBox = new QMessageBox;
		Client& c = Client::getInstance();
		c.Incoming().clear();
		c.LogoutWithRememberMe(PAT);
		while (c.Incoming().empty())
		{
			if (!c.IsConnected())
			{
				msgBox->setText("Server down! Client disconnected!");
				msgBox->show();
				stillConnectedWaitingForAnswer = false;
				break;
			}
		}

		if (stillConnectedWaitingForAnswer)
		{
			if (!c.Incoming().empty())
			{
				auto msg = c.Incoming().pop_front().msg;
				if (msg.header.id == clever::MessageType::LogoutRememberedRequest)
				{
					char responseBack[1024];
					msg >> responseBack;
					if (strcmp(responseBack, "LogoutRememberedSuccess") == 0)
					{
						// all good, log out successfully, with PAT deleted from sessions in DB.
					}
					else
					{
						msgBox->setText("An error occured while logging out...");
						msgBox->show();
						QTimer::singleShot(2500, msgBox, SLOT(close()));
					}
					stillConnectedWaitingForAnswer = false;
				}
			}
		}
		if (msgBox)
		{
			delete msgBox;
		}

		// close file pointer.
		fclose(fin);

		// delete PAT file.
		std::remove("PAT.txt");
	}
	// finally switch back to startup.
	if (this->fromStackedWidget!=Q_NULLPTR)
	{
		this->fromStackedWidget->removeWidget(this);
		this->fromStackedWidget->setCurrentIndex(0);
	}

}

void Dashboard::loadCash()
{
	// TO DO.
		// loads all cards for this USER LOGGED IN into cardPicker.
	bool stillConnectedWaitingForAnswer = true;
	QMessageBox* msgBox = new QMessageBox;
	Client& c = Client::getInstance();
	c.Incoming().clear();
	if (this->PATLoggedIn == "") // it means user-login
	{
		c.UsernameGetCashDetails(usernameLoggedIn);
	}
	else // else PAT-login
	{
		c.PATGetCashDetails(PATLoggedIn.toStdString());
	}
	while (c.Incoming().empty())
	{
		if (!c.IsConnected())
		{
			msgBox->setText("Server down! Client disconnected!");
			msgBox->show();
			stillConnectedWaitingForAnswer = false;
			break;
		}
	}
	if (!c.Incoming().empty() && stillConnectedWaitingForAnswer)
	{
		auto msg = c.Incoming().pop_front().msg;
		if (msg.header.id == clever::MessageType::ServerGetCashResponse)
		{
			char responseBack[1024];
			char userCash[1024];
			char userCurrencyISO[1024];
			char usernameLogged[1024];

			char firstName[1024];
			char lastName[1024];
			char email[1024];
			char country[1024];
			char phoneNumber[1024];
			char picture[1024];
			char dailyMailState[1024];
			msg >> responseBack;
			if (strcmp(responseBack, "SuccesGetCashDetails") == 0)
			{
				msg >> userCurrencyISO >> userCash >> usernameLogged >> firstName >> lastName >> email >> country >> phoneNumber >> picture >> dailyMailState;
				this->currUserCash = userCash;
				this->userCashCurrencyISO = userCurrencyISO;
				this->usernameLoggedIn = usernameLogged;

				this->user_information.setUsername(usernameLogged);
				this->user_information.setFirstName(firstName);
				this->user_information.setLastName(lastName);
				this->user_information.setEmail(email);
				this->user_information.setCountryID(country);
				this->user_information.setPhoneNumber(phoneNumber);
				this->profilePicture = picture;


				// loading profile picture
				QByteArray ss(this->profilePicture.c_str(), this->profilePicture.length());
				QByteArray by = QByteArray::fromBase64(ss);
				QImage img = QImage::fromData(by, "png");
				ui.profilePictureLabel->setPixmap(QPixmap::fromImage(img));

				//loading profile info
				strcat(firstName, " ");
				strcat(firstName, lastName);
				ui.firstLastNameLabel->setText(firstName);
				ui.emailLabel->setText(email);
				ui.phoneLabel->setText(phoneNumber);
				ui.countryLabel->setText(country);

				//setting preference option
				if (strcmp(dailyMailState,"False")==0)
				{
					ui.checkBox->setChecked(false); //rename checkbox qt
					this->dailyMailState = "False";
				}
				else
				{
					ui.checkBox->setChecked(true); // rename checkbox qt
					this->dailyMailState = "True";
				}
			}
			if (strcmp(responseBack, "FailGetCashDetails") == 0)
			{
				msgBox->setText("An error occured while getting cash details out...");
				msgBox->show();
				QTimer::singleShot(2500, msgBox, SLOT(close()));
				return;
			}
		}
	}
}

void Dashboard::loadCards()
{
	// TO DO.
	// loads all cards for this USER LOGGED IN into cardPicker.
	ui.cardPicker->clear();
	bool stillConnectedWaitingForAnswer = true;
	QMessageBox* msgBox = new QMessageBox;
	Client& c = Client::getInstance();
	c.Incoming().clear();
	if (this->PATLoggedIn == "") // it means user-login
	{
		c.UsernameGetCardsDetails(usernameLoggedIn);
	}
	else // else PAT-login
	{
		c.PATGetCardsDetails(PATLoggedIn.toStdString());
	}
	while (c.Incoming().empty())
	{
		if (!c.IsConnected())
		{
			msgBox->setText("Server down! Client disconnected!");
			msgBox->show();
			stillConnectedWaitingForAnswer = false;
			break;
		}
	}
	int cardsToCome=0;
	int cardsIndex = 0;
	if (!c.Incoming().empty())
	{
		auto msg = c.Incoming().pop_front().msg;
		if (msg.header.id == clever::MessageType::ServerGetCardsResponse)
		{
			msg >> cardsToCome;
		}
	}
	if (cardsToCome > 0)
	{
		if (ui.cardPicker->itemText(0) == "Nu exista carduri adaugate la acest cont")
		{
			ui.cardPicker->removeItem(0);
		}
		map_cards.clear();
	}
	while (stillConnectedWaitingForAnswer)
	{
		if (!c.IsConnected())
		{
			msgBox->setText("Server down! Client disconnected!");
			msgBox->show();
			stillConnectedWaitingForAnswer = false;
			break;
		}
		if (cardsIndex == cardsToCome)
		{
			stillConnectedWaitingForAnswer = false;
			break;
		}
		if (!c.Incoming().empty())
		{
			auto msg = c.Incoming().pop_front().msg;
			if (msg.header.id == clever::MessageType::ServerGetCardsResponse)
			{
				char cardName[1024]; msg >> cardName;
				char cardHolder[1024]; msg >> cardHolder;
				char cardNumber[1024]; msg >> cardNumber;
				char cardValidUntil[1024]; msg >> cardValidUntil;
				char cardCurrencyISO[1024]; msg >> cardCurrencyISO;
				float cardSold; msg >> cardSold;
				
				clever::CardCredentialHandler newCard(cardName, cardHolder, cardNumber, cardCurrencyISO, cardValidUntil,cardSold);
				map_cards.insert(std::pair<std::string, clever::CardCredentialHandler>(cardName, newCard));
				ui.cardPicker->addItem(newCard.getCardName());
				cardsIndex++;
			}
		}
	}
}

void Dashboard::addCardExec(AddCardDialog& adc)
{
	if (adc.exec())
	{
		// request to db in dialog add button.
		if (adc.result() == QDialog::Accepted)
		{
			if (ui.cardPicker->itemText(0) == "Nu exista carduri adaugate la acest cont")
			{
				ui.cardPicker->removeItem(0);
			}
			loadCards();
		}
	}
}

void Dashboard::addFundsExec(AddFundsDialog& adf)
{
	if (adf.exec())
	{
		if (adf.result() == QDialog::Accepted)
		{
			float soldFromWall = std::stof(ui.soldWallLabel->text().toStdString());
			soldFromWall += std::stof(adf.fundValueLineEdit->text().toStdString());
			ui.soldWallLabel->setText(std::to_string(soldFromWall).c_str());
			this->map_cards[adf.cardNameLineEdit->text().toStdString()].setCardSold(soldFromWall);
		}
	}
}

void Dashboard::editCardExec(EditCardDialog& edc)
{
	edc.cardNameLineEdit->setText(ui.cardPicker->currentText());
	edc.cardHolderLineEdit->setText(this->map_cards[ui.cardPicker->currentText().toStdString()].getCardHolder());
	edc.cardNumberLineEdit->setText(this->map_cards[ui.cardPicker->currentText().toStdString()].getCardNumber());
	edc.validUntilLineEdit->setText(this->map_cards[ui.cardPicker->currentText().toStdString()].getCardValidUntil());
	edc.isoCurrencyComboBox->setCurrentText(this->map_cards[ui.cardPicker->currentText().toStdString()].getCardCurrencyISO());
	edc.setOldCardName(ui.cardPicker->currentText().toStdString());
	std::string oldcardname = ui.cardPicker->currentText().toStdString();
	float oldSold = this->map_cards[oldcardname].getCardSold();
	if (edc.exec())
	{
		clever::CardCredentialHandler newCard(edc.cardNameLineEdit->text().toStdString(), edc.cardHolderLineEdit->text().toStdString(), edc.cardNumberLineEdit->text().toStdString(), edc.isoCurrencyComboBox->currentText().toStdString(), edc.validUntilLineEdit->text().toStdString(), oldSold);
		if (edc.result() == QDialog::Accepted)
		{
			// handle map cards.
			this->map_cards.erase(oldcardname);
			ui.cardPicker->removeItem(ui.cardPicker->currentIndex());
			this->map_cards.insert(std::pair<std::string, clever::CardCredentialHandler>(edc.cardNameLineEdit->text().toStdString(), newCard));
			ui.cardPicker->addItem(newCard.getCardName());
			//ui.cardPicker->setCurrentIndex(ui.cardPicker->count() - 1);
			ui.cardPicker->setCurrentText(newCard.getCardName());
			on_cardSelected();
		}
	}
}

void Dashboard::rechargeCashExec(RechargeCashDialog& rcd)
{
	rcd.cashCurrencyISOLabel->setText(QString(userCashCurrencyISO.c_str()));
	if (rcd.exec())
	{
		if (rcd.result() == QDialog::Accepted)
		{
			this->map_cards = rcd.getCardMap();
			float currUserCashFloat = std::stof(currUserCash);
			currUserCashFloat += rcd.cashFundsLIneEdit->text().toFloat();
			this->currUserCash = std::to_string(currUserCashFloat);


			ui.financeNameWallLabel->setText(QString("Cash"));
			ui.currencyWallLabel->setText(QString(userCashCurrencyISO.c_str()));
			ui.soldWallLabel->setText(QString(currUserCash.c_str()));
			ui.currentCashSoldLabel->setText(QString(currUserCash.c_str()));
		}
	}
}

void Dashboard::addSpendingExec(AddSpendingsDialog& ads)
{
	if (ads.exec())
	{
		if (ads.result() == QDialog::Accepted)
		{
			if (ads.categoryNameLineEdit->text() == "Cash")
			{
				float currCashSold = std::stoi(this->currUserCash) - ads.categoryTranzactionValueLineEdit->text().toFloat();
				this->currUserCash = std::to_string(currCashSold);
			}
			else
			{
				this->map_cards[ads.categoryFinancePicker->currentText().toStdString()].setCardSold(this->map_cards[ads.categoryFinancePicker->currentText().toStdString()].getCardSold() - ads.categoryTranzactionValueLineEdit->text().toFloat());
			}

			std::string tranzTitle = ads.categoryTranzactionTitleLineEdit->text().toStdString();
			std::string tranzSource = this->usernameLoggedIn;
			std::string tranzDestination = ads.categoryTranzactionDestinationLineEdit->text().toStdString();
			std::string timeStamp = this->getCurrentDateTime() + ".000";
			std::string tranzFinanceName = ads.categoryFinancePicker->currentText().toStdString();
			clever::TranzactionType trType = clever::TranzactionType::Spending;
			float tranzVal = ads.categoryTranzactionValueLineEdit->text().toFloat();
			std::string tranzISO = ads.financeISOLineEdit->text().toStdString();
			std::string tranzDescription = ads.categoryTranzactionDetailsLineEdit->text().toStdString();
			std::string tranzCategoryName = ads.categoryNameLineEdit->text().toStdString();

			clever::TranzactionHandler newTranzaction(tranzTitle,
				tranzSource,
				tranzDestination,
				timeStamp,
				tranzFinanceName,
				trType,
				tranzVal,
				tranzISO,
				tranzDescription,
				tranzCategoryName);

			this->all_user_tranzactions.insert(this->all_user_tranzactions.begin(), 1, newTranzaction);
			load_spendingsTotals();
		}
	}
}

void Dashboard::toggleTranzactionsButtons(int state)
{
	switch (state)
	{
	case 0:
		// means we're not in this page OR select'All Tranzactions', so both buttons invisible.
		ui.periodicallyIncomePushButton->setVisible(false);
		ui.periodicallyIncomePushButton->setEnabled(false);
		ui.spendingsCategoriesPushButton->setVisible(false);
		ui.spendingsCategoriesPushButton->setEnabled(false);
		break;
	case 1: 
		// means we selected Income
		ui.periodicallyIncomePushButton->setEnabled(true);
		ui.periodicallyIncomePushButton->setVisible(true);
		ui.spendingsCategoriesPushButton->setVisible(false);
		ui.spendingsCategoriesPushButton->setEnabled(false);
		break;

	case 2:
		// means Spendings selected.
		ui.periodicallyIncomePushButton->setEnabled(false);
		ui.periodicallyIncomePushButton->setVisible(false);
		ui.spendingsCategoriesPushButton->setVisible(true);
		ui.spendingsCategoriesPushButton->setEnabled(true);
		break;
	}
}

void Dashboard::populateTranzactionsFinanceType()
{
	ui.financeTypePicker->clear();
	ui.financeTypePicker->addItem("All Finances");
	
	// get all cards from map and push them.
	for (auto it : this->map_cards)
	{
		ui.financeTypePicker->addItem(it.second.getCardName());
	}

	ui.financeTypePicker->addItem("Cash");
}

void Dashboard::insertBDProfiePicture(std::string& hexImg)
{
	QMessageBox* msgBox = new QMessageBox;
	if (hexImg.empty())
	{
		return;
	}
	bool stillConnectedWaitingAnswer = true;
	Client& c = Client::getInstance(); // handles connection too, if it is not connected.!!
	c.Incoming().clear();
	if (this->usernameLoggedIn == "")
	{
		c.AddPATPicture(this->PATLoggedIn.toStdString(), hexImg);
	}
	else
	{
		c.AddUsernamePicture(this->usernameLoggedIn, hexImg);
	}
	while (c.Incoming().empty())
	{
		if (!c.IsConnected())
		{
			msgBox->setText("Server down! Client disconnected!");
			msgBox->show();
			QTimer::singleShot(2500, msgBox, SLOT(close()));
			stillConnectedWaitingAnswer = false;
			break;
		}
	}
	if (!c.Incoming().empty() && stillConnectedWaitingAnswer)
	{
		auto msg = c.Incoming().pop_front().msg;
		if (msg.header.id == clever::MessageType::ServerAddPictureResponse)
		{
			// server has responded back to add picture request.
			char responseBack[1024];
			msg >> responseBack;
			if (strcmp(responseBack, "SuccessAddPicture") == 0)
			{
				// ALL GOOD.
				msgBox->setText("Picture added with success!");
				msgBox->show();
				QTimer::singleShot(2500, msgBox, SLOT(close()));
			}
			else
			{
				msgBox->setText("Couldn't add profile picture! Server problem. Try again.");
				msgBox->show();
				QTimer::singleShot(2500, msgBox, SLOT(close()));
				return;
			}
			stillConnectedWaitingAnswer = false;
		}
	}
}

void Dashboard::load_spendingsTotals()
{
	float transportTotal = 0;
	float educationTotal = 0;
	float  shoppingTotal = 0;
	float freeTimeTotal = 0;
	float healthTotal = 0;
	float holidayTotal = 0;

	std::time_t t = std::time(0); // get time now.
	std::tm* now = std::localtime(&t);
	int month = 1 + (now->tm_mon);
	std::string tranzMonth;
	std::string currMonth = "";
	if (month < 10)
	{
		currMonth += "0";
	}
	currMonth += std::to_string(month);
	for (auto& it : this->all_user_tranzactions)
	{
		tranzMonth = it.getTranzactionTimestamp();
		tranzMonth = tranzMonth.substr(5, 2); // yyyy-mm-dd hh:mm:ss
		if (currMonth == tranzMonth)
		{
			if (strcmp(it.getTranzactionCategoryName(),"Education")==0)
			{
				educationTotal += it.getTranzactionValue();
			}
			if (strcmp(it.getTranzactionCategoryName(), "Free Time")==0)
			{
				freeTimeTotal += it.getTranzactionValue();
			}
			if (strcmp(it.getTranzactionCategoryName(), "Health & Self-Care")==0)
			{
				healthTotal += it.getTranzactionValue();
			}
			if (strcmp(it.getTranzactionCategoryName(), "Holiday & Travel")==0)
			{
				holidayTotal += it.getTranzactionValue();
			}
			if (strcmp(it.getTranzactionCategoryName(), "Public Transport & Taxi")==0)
			{
				transportTotal += it.getTranzactionValue();
			}
			if (strcmp(it.getTranzactionCategoryName(),"Shopping")==0)
			{
				shoppingTotal += it.getTranzactionValue();
			}
		}
	}

	ui.transportMonthSpentLabel->setText(getFloatText2Decimal(transportTotal).c_str());
	ui.freeTimeMonthSpentLabel->setText(getFloatText2Decimal(freeTimeTotal).c_str());
	ui.holidayMonthSpentLabel->setText(getFloatText2Decimal(holidayTotal).c_str());
	ui.healthMonthSpentLabel->setText(getFloatText2Decimal(healthTotal).c_str());
	ui.shoppingMonthSpentLabel->setText(getFloatText2Decimal(shoppingTotal).c_str());
	ui.educationMonthSpentLabel->setText(getFloatText2Decimal(educationTotal).c_str());

	QPieSeries* series = new QPieSeries;

	series->append("Education", educationTotal);
	series->append("Free Time", freeTimeTotal);
	series->append("Health & Self-Care", healthTotal);
	series->append("Holiday & Travel", holidayTotal);
	series->append("Public Transport & Taxi", transportTotal);
	series->append("Shopping", shoppingTotal);



	QChart* totalSpendingsChart = new QChart;
	totalSpendingsChart->addSeries(series);
	totalSpendingsChart->setTitle("Total Spendings");
	totalSpendingsChart->legend()->hide();
	totalSpendingsChart->setAnimationOptions(QChart::AllAnimations);
	totalSpendingsChart->setBackgroundVisible(false);

	series->setLabelsVisible();

	ui.graphicsView->setChart(totalSpendingsChart);
	ui.graphicsView->setRenderHint(QPainter::Antialiasing);

}

void Dashboard::loadCurrencyISOS()
{
	// load CurrencyISO from resource text file into combobox.
	FILE* fin = fopen("currencyISO.txt", "r");
	if (fin)
	{
		char buffer[256];
		while (fgets(buffer, sizeof(buffer), fin))
		{
			std::string line;
			if (buffer[strlen(buffer) - 1] != '\n')
			{
				line = buffer;
			}
			else
			{
				line.assign(buffer, strlen(buffer) - 1);
			}
			this->currencyISOS.push_back(line);
		}
		fclose(fin);
		ui.preferenceCurrencyComboBox->clear();
		for (std::vector<std::string>::iterator it = currencyISOS.begin(); it != currencyISOS.end(); it++)
		{
			ui.preferenceCurrencyComboBox->addItem(it->c_str());
		}
	}
}

void Dashboard::on_menuItemSelected(int index)
{
	// index 0 is reserved for header : Avatar + "Andronache George" -- current user logged in.
	switch (index)
	{
	case 1:
		ui.stackedWidget->setCurrentWidget(ui.yourProfilePage);
		break;
	case 2:
		ui.stackedWidget->setCurrentWidget(ui.dashboardPage);
		break;
	case 3:
		ui.stackedWidget->setCurrentWidget(ui.preferencesPage);
		break;
	case 4:
		logout();
		break;
	default:
		break;
	}


}

void Dashboard::on_financesTabWidget_currentChanged(int index)
{
	if (ui.financesTabWidget->currentWidget() == ui.cashTab)
	{
		ui.financeNameWallLabel->setText(QString("Cash"));
		ui.currencyWallLabel->setText(QString(userCashCurrencyISO.c_str()));
		ui.soldWallLabel->setText(QString(currUserCash.c_str()));
		ui.currentCashSoldLabel->setText(QString(currUserCash.c_str()));
	}
	if (ui.financesTabWidget->currentWidget() == ui.cardsTab)
	{
		on_cardSelected();
	}
}

void Dashboard::on_tranzactionTypePicker_currentTextChanged(const QString& newText)
{
	updateTranzactionsByFilters(currFinanceName, newText.toStdString(), currTranzDate);
	this->currTranzType = newText.toStdString();
}

void Dashboard::on_financeTypePicker_currentTextChanged(const QString& newText)
{
	updateTranzactionsByFilters(newText.toStdString(), currTranzType, currTranzDate);
	this->currFinanceName = newText.toStdString();
}
void Dashboard::prepareOptionsComboBox(QComboBox* comboBoxToPrepare)
{
	if (this->usernameLoggedIn != "")
	{
		comboBoxToPrepare->addItem(this->usernameLoggedIn.c_str());
	}
	comboBoxToPrepare->addItem("Your profile");
	comboBoxToPrepare->addItem("Dashboard");
	comboBoxToPrepare->addItem("Preferences");
	comboBoxToPrepare->addItem("Log out");
	connect(comboBoxToPrepare, SIGNAL(activated(int)), this, SLOT(on_menuItemSelected(int)));
}

void Dashboard::prepareAllOptionsComboBox()
{
	this->prepareOptionsComboBox(ui.incomeOptions);
	this->prepareOptionsComboBox(ui.dashboardOptions);
	this->prepareOptionsComboBox(ui.financesOptions);
	this->prepareOptionsComboBox(ui.preferencesOptions);
	this->prepareOptionsComboBox(ui.profileOptions);
	this->prepareOptionsComboBox(ui.tranzactionsOptions);
	this->prepareOptionsComboBox(ui.categoriesOptions);
	this->prepareOptionsComboBox(ui.outcomeOptions);
	this->prepareOptionsComboBox(ui.goalsOptions);
	this->prepareOptionsComboBox(ui.savingsOptions);
	this->prepareOptionsComboBox(ui.budgetOptions);
	this->prepareOptionsComboBox(ui.statisticsOptions);
}

void Dashboard::refreshCash()
{
	this->cash_details.first = this->currUserCash;
	this->cash_details.second = this->userCashCurrencyISO;
}
void Dashboard::updateTranzactionsDefault()
{
	ui.listWidget->clear();
	for (std::vector<clever::TranzactionHandler>::iterator iter = all_user_tranzactions.begin(); iter != all_user_tranzactions.end(); iter++)
	{
		QListWidgetItem* item = new QListWidgetItem(ui.listWidget);
		ui.listWidget->addItem(item);
		std::string valToShow = std::to_string(iter->getTranzactionValue());
		if (valToShow.find('.'))
		{
			valToShow.erase(valToShow.find('.') + 3);
		}
		TranzactionRowItem* tranzRowItem = new TranzactionRowItem(ui.listWidget->width(),
		iter->getTranzactionTitle(),iter->getTranzactionTimestamp(),valToShow.c_str(),iter->getTranzactionCurrencyISO(), iter->getTranzactionType());
		item->setSizeHint(tranzRowItem->minimumSizeHint());
		ui.listWidget->setItemWidget(item, tranzRowItem);
	}
}

void Dashboard::updateTranzactionsByFilters(std::string FinanceName, std::string TranzactionType, std::string TranzactionDate)
{
	clever::TranzactionType trType = (TranzactionType == "Income") ? clever::TranzactionType::Income : clever::TranzactionType::Spending;
	ui.listWidget->clear();
	for (std::vector<clever::TranzactionHandler>::iterator iter = all_user_tranzactions.begin(); iter != all_user_tranzactions.end(); iter++)
	{
		if ((strcmp(iter->getTranzactionFinanceName(),FinanceName.c_str())==0 || FinanceName == "All Finances") && (iter->getTranzactionType() == trType || TranzactionType == "All Tranzactions") && (strncmp(iter->getTranzactionTimestamp(),TranzactionDate.c_str(),10)<=0))
		{
			QListWidgetItem* item = new QListWidgetItem(ui.listWidget);
			ui.listWidget->addItem(item);
			std::string valToShow = std::to_string(iter->getTranzactionValue());
			valToShow.erase(valToShow.find('.') + 3);

			// HANDLES DATE FORMAT TRIM IN CONSTRUCTOR OF ROWITEM
			/*std::string dateTimestamp = iter->getTranzactionTimestamp();
		dateTimestamp.erase(dateTimestamp.find('.'));*/ 
			TranzactionRowItem* tranzRowItem = new TranzactionRowItem(ui.listWidget->width(),
				iter->getTranzactionTitle(), iter->getTranzactionTimestamp(), valToShow.c_str(), iter->getTranzactionCurrencyISO(), iter->getTranzactionType());
			item->setSizeHint(tranzRowItem->minimumSizeHint());
			ui.listWidget->setItemWidget(item, tranzRowItem);
		}
	}
}
clever::TranzactionHandler Dashboard::getTranzactionByCurrentItem(const char* trTitle, const char* trTimestamp)
{
	// TODO: insert return statement here
	for (std::vector<clever::TranzactionHandler>::iterator iter = all_user_tranzactions.begin(); iter != all_user_tranzactions.end(); iter++)
	{
		if (strcmp(iter->getTranzactionTitle(), trTitle) == 0 && strcmp(iter->getTranzactionTimestamp(), trTimestamp) == 0)
		{
			return *iter;
		}
	}
	return clever::TranzactionHandler();
}
void Dashboard::on_financesCommandLinkButton_clicked()
{
	ui.stackedWidget->setCurrentWidget(ui.financesPage);
	on_cardSelected();
}

void Dashboard::on_addCardPushButton_clicked()
{
	if (this->PATLoggedIn == "")
	{
		AddCardDialog addcarddialog(usernameLoggedIn,this);
		addCardExec(addcarddialog);
		
	}
	else
	{
		AddCardDialog addcarddialog(PATLoggedIn, this);
		addCardExec(addcarddialog);
	}
}

void Dashboard::on_addFundsPushButton_clicked()
{
	QMessageBox* msgBox = new QMessageBox;
	if (ui.cardPicker->itemText(0) == "Nu exista carduri adaugate la acest cont")
	{
		msgBox->setText("First add cards!");
		msgBox->show();
		QTimer::singleShot(2000, msgBox, SLOT(close()));
		return;
	}
	if (this->PATLoggedIn == "")
	{
		AddFundsDialog addfundsdialog(ui.cardPicker->currentText(), ui.currencyWallLabel->text(), this->usernameLoggedIn);
		addFundsExec(addfundsdialog);
	}
	else
	{
		AddFundsDialog addfundsdialog(ui.cardPicker->currentText(), ui.currencyWallLabel->text(), this->PATLoggedIn);
		addFundsExec(addfundsdialog);
	}
}

void Dashboard::on_editCardPushButton_clicked()
{
	QMessageBox* msgBox = new QMessageBox;
	if (ui.cardPicker->itemText(0) == "Nu exista carduri adaugate la acest cont")
	{
		msgBox->setText("First add cards!");
		msgBox->show();
		QTimer::singleShot(2000, msgBox, SLOT(close()));
		return;
	}
	if (this->PATLoggedIn == "")
	{
		EditCardDialog editcarddialog(usernameLoggedIn, this);
		editCardExec(editcarddialog);
	}
	else
	{
		EditCardDialog editcarddialog(PATLoggedIn, this);
		editCardExec(editcarddialog);
	}

}

void Dashboard::on_cardSelected()
{
	QString cardName = ui.cardPicker->currentText();
	ui.cardNameDetailsLabel->setText(this->map_cards[cardName.toStdString()].getCardName());
	ui.holderDetailsLabel->setText(this->map_cards[cardName.toStdString()].getCardHolder());
	ui.cardNumberDetailsLabel->setText(this->map_cards[cardName.toStdString()].getCardNumber());
	ui.validUntilDetailsLabel->setText(this->map_cards[cardName.toStdString()].getCardValidUntil());

	// update wall as well.
	ui.financeNameWallLabel->setText(this->map_cards[cardName.toStdString()].getCardName());
	ui.currencyWallLabel->setText(this->map_cards[cardName.toStdString()].getCardCurrencyISO());
	ui.soldWallLabel->setText(std::to_string(this->map_cards[cardName.toStdString()].getCardSold()).c_str());
}

void Dashboard::on_choseImagePushButton_clicked()
{
	QString filename = QFileDialog::getOpenFileName(this, tr("Chose"), "", tr("Images(*.png *.jpg *.jpeg *.bmp *.gif)")); // title of file dialog-default folder-file format

	if (QString::compare(filename, QString()) != 0)
	{
		QImage image;
		bool valid = image.load(filename);

		if (valid)
		{
			image = image.scaledToWidth(ui.profilePictureLabel->width(), Qt::SmoothTransformation);
			ui.profilePictureLabel->setPixmap(QPixmap::fromImage(image));
			QByteArray imgByteArray;
			QBuffer inByteArray(&imgByteArray);
			image.save(&inByteArray, "png");
			QByteArray hexImg = imgByteArray.toBase64();
			insertBDProfiePicture(hexImg.toStdString());
			this->profilePicture = hexImg.toStdString();
		}
		else
		{
			//Error handling
		}

	}

}

void Dashboard::on_addCashPushButton_clicked()
{
	QMessageBox* msgBox = new QMessageBox;
	if (ui.cardPicker->itemText(0) == "Nu exista carduri adaugate la acest cont!")
	{
		msgBox->setText("First add cards!");
		msgBox->show();
		QTimer::singleShot(2000, msgBox, SLOT(close()));
		return;
	}
	if (this->userCashCurrencyISO=="0") //nu este selectata moneda implicita de la preferences //TESTAT EMPTY
	{
		msgBox->setText("Please complete the preferences options first!");
		msgBox->show();
		QTimer::singleShot(2000, msgBox, SLOT(close()));
		return;
	}
	if (this->PATLoggedIn == "")
	{
		RechargeCashDialog rechargeCashDialog(userCashCurrencyISO, map_cards, usernameLoggedIn, this);
		rechargeCashExec(rechargeCashDialog);
	}
	else
	{
		RechargeCashDialog rechargeCashDialog(userCashCurrencyISO, map_cards, this->PATLoggedIn, this);
		rechargeCashExec(rechargeCashDialog);
	}
}
void Dashboard::on_transactionsCommandLinkButton_clicked()
{
	ui.stackedWidget->setCurrentWidget(ui.tranzactionsHistoryPage);
	ui.tranzactionTypePicker->setCurrentIndex(0); // All tranzactions.
	populateTranzactionsFinanceType();
	ui.dateLineEdit->setText(QDate::currentDate().toString());
	toggleTranzactionsButtons(0);
	this->currFinanceName = ui.financeTypePicker->currentText().toStdString();
	this->currTranzType = ui.tranzactionTypePicker->currentText().toStdString();
	this->currTranzDate = QDate::currentDate().toString("yyyy-MM-dd").toStdString();
	updateTranzactionsByFilters(currFinanceName, currTranzType, currTranzDate);
}

void Dashboard::on_pickDatePushButton_clicked()
{
	DatePickerDialog datePicker(this);
	if (datePicker.exec())
	{
		if (datePicker.result() == QDialog::Accepted)
		{
			// update DATE line edit. YYYY-MM-DD.
			ui.dateLineEdit->setText(datePicker.calendarWidget->selectedDate().toString());
			this->currTranzDate = datePicker.calendarWidget->selectedDate().toString("yyyy-MM-dd").toStdString();
			updateTranzactionsByFilters(currFinanceName, currTranzType, currTranzDate);
		}
	}
}

void Dashboard::on_TranzactionTypeSelected()
{
	//
	if (ui.tranzactionTypePicker->currentText() == "All Tranzactions")
	{
		toggleTranzactionsButtons(0);
	}
	if (ui.tranzactionTypePicker->currentText() == "Income")
	{
		toggleTranzactionsButtons(1);
	}
	if (ui.tranzactionTypePicker->currentText() == "Spendings")
	{
		toggleTranzactionsButtons(2);
	}
}

void Dashboard::on_spendingsCategoriesPushButton_clicked()
{
	ui.stackedWidget->setCurrentWidget(ui.categoriesPage);
	load_spendingsTotals();
}

void Dashboard::on_backToTranzactionsPushButtton_clicked()
{
	on_transactionsCommandLinkButton_clicked();
}

void Dashboard::on_showFinanceHistory_clicked()
{
	ui.stackedWidget->setCurrentWidget(ui.tranzactionsHistoryPage);
	ui.tranzactionTypePicker->setCurrentIndex(0); // All tranzactions.
	toggleTranzactionsButtons(0);
	ui.dateLineEdit->setText(QDate::currentDate().toString());
	populateTranzactionsFinanceType();
	ui.financeTypePicker->setCurrentText(ui.financeNameWallLabel->text());
	this->currFinanceName = ui.financeTypePicker->currentText().toStdString();
	this->currTranzType = ui.tranzactionTypePicker->currentText().toStdString();
	this->currTranzDate = QDate::currentDate().toString("yyyy-MM-dd").toStdString();
	updateTranzactionsByFilters(currFinanceName, currTranzType, currTranzDate);
}

void Dashboard::on_savePreferencesButton_clicked()
{
	this->userCashCurrencyISO = ui.preferenceCurrencyComboBox->currentText().toStdString();
	if (ui.checkBox->isChecked())  // rename this object in qt designer
	{
		this->dailyMailState = "True";
	}
	else
	{
		this->dailyMailState = "False";
	}
	bool stillConnectedWaitingAnswer = true;
	QMessageBox* msgBox = new QMessageBox();
	Client& c = Client::getInstance(); // handles connection too, if it is not connected.!!
	c.Incoming().clear();
	c.AddPreferences(usernameLoggedIn, dailyMailState, userCashCurrencyISO);
	while (c.Incoming().empty())
	{
		if (!c.IsConnected())
		{
			msgBox->setText("Server down! Client disconnected!");
			msgBox->show();
			QTimer::singleShot(2500, msgBox, SLOT(close()));
			stillConnectedWaitingAnswer = false;
			break;
		}
	}
	if (!c.Incoming().empty() && stillConnectedWaitingAnswer)
	{
		auto msg = c.Incoming().pop_front().msg;
		if (msg.header.id == clever::MessageType::ServerAddPreferencesOptionResponse)
		{
			// server has responded back to add preferences options request.
			char responseBack[1024];
			msg >> responseBack;
			if (strcmp(responseBack, "SuccessAddPreferencesOption") == 0)
			{
				// ALL GOOD.
				msgBox->setText("Preferences added with success!");
				msgBox->show();
				QTimer::singleShot(2500, msgBox, SLOT(close()));
			}
			else if (strcmp(responseBack, "AlreadyCheckedDailyNotification") == 0)
			{
				msgBox->setText("Already checked for daily notification!");
				msgBox->show();
				QTimer::singleShot(2500, msgBox, SLOT(close()));
			}
			else
			{
				msgBox->setText("Couldn't add preferences ! Server problem. Try again.");
				msgBox->show();
				QTimer::singleShot(2500, msgBox, SLOT(close()));
				return;
			}
			stillConnectedWaitingAnswer = false;
		}
	}
}

void Dashboard::on_listWidget_itemClicked(QListWidgetItem* item)
{
	TranzactionRowItem* tr = dynamic_cast<TranzactionRowItem*>(ui.listWidget->itemWidget(item));
	clever::TranzactionHandler tranzInfo;
	if (tr != nullptr)
	{
		tranzInfo = getTranzactionByCurrentItem(tr->getTranzactionTitle(), tr->getTranzactionTimestamp());
	}
	InfoTranzactionDialog infotranzdialog(this);
	infotranzdialog.titleInfoLineEdit->setText(tranzInfo.getTranzactionTitle());
	infotranzdialog.sourceInfoLineEdit->setText(tranzInfo.getTranzactionSource());
	infotranzdialog.destinationInfoLineEdit->setText(tranzInfo.getTranzactionDestination());
	infotranzdialog.financeInfoLineEdit->setText(tranzInfo.getTranzactionFinanceName());

	std::string dateTimestamp = tranzInfo.getTranzactionTimestamp();
	dateTimestamp.erase(dateTimestamp.find('.'));

	infotranzdialog.timestampInfoLineEdit->setText(dateTimestamp.c_str());

	std::string valToShow = std::to_string(tranzInfo.getTranzactionValue());
	valToShow.erase(valToShow.find('.') + 3);
	clever::TranzactionType trType = tranzInfo.getTranzactionType();
	std::string finalVal = (trType == clever::TranzactionType::Income)? "+" : "-";
	finalVal += valToShow;
	finalVal += " ";
	finalVal += (tranzInfo.getTranzactionCurrencyISO());
	if (trType == clever::TranzactionType::Income)
	{
		infotranzdialog.valueInfoLineEdit->setStyleSheet("QLineEdit {color : green; }");
	}
	else
	{
		infotranzdialog.valueInfoLineEdit->setStyleSheet("QLineEdit {color : red; }");
	}
	infotranzdialog.valueInfoLineEdit->setText(finalVal.c_str());
	infotranzdialog.descriptionInfoLineEdit->setText(tranzInfo.getTranzactionDescription());
	infotranzdialog.categoryInfoLabel->setText(tranzInfo.getTranzactionCategoryName());
	infotranzdialog.exec();
	QListWidgetItem* currItem = ui.listWidget->currentItem();
	ui.listWidget->setItemSelected(currItem, false);
}

void Dashboard::on_recurenciesSelected()
{
	QString recurenciesName;
	if (ui.stackedWidget->currentWidget() == ui.periodicallyIncomePage)
	{
		recurenciesName = ui.incomePicker->currentText();
		ui.incomeNameLineEdit->setText(this->map_recurencies[recurenciesName.toStdString()].getFinanceTypeName());
		ui.incomeValueLineEdit->setText(std::to_string(this->map_recurencies[recurenciesName.toStdString()].getFinanceTypeValue()).c_str());
		ui.dayOfMonthLineEdit->setText(this->map_recurencies[recurenciesName.toStdString()].getDayOfFinanceType());
	}
	if (ui.stackedWidget->currentWidget() == ui.periodicallyOutcomePage)
	{
		recurenciesName = ui.outcomePicker->currentText();
		ui.outcomeNameLineEdit->setText(this->map_recurencies[recurenciesName.toStdString()].getFinanceTypeName());
		ui.outcomeValueLineEdit->setText(std::to_string(this->map_recurencies[recurenciesName.toStdString()].getFinanceTypeValue()).c_str());
		ui.outcomeDayOfMonthLineEdit->setText(this->map_recurencies[recurenciesName.toStdString()].getDayOfFinanceType());
	}
}

void Dashboard::on_addIncomePushButton_clicked()
{
	AddIncomeDialog addincomedialog(map_cards, usernameLoggedIn, this);
	addIncomeExec(addincomedialog);
}

void Dashboard::on_periodicallyIncomePushButton_clicked()
{
	ui.stackedWidget->setCurrentWidget(ui.periodicallyIncomePage);
	on_recurenciesSelected();
}

void Dashboard::on_incomeBackToTranzactionsPushButton_clicked()
{
	ui.stackedWidget->setCurrentWidget(ui.tranzactionsHistoryPage);
	ui.tranzactionTypePicker->setCurrentIndex(0); // All tranzactions.
	populateTranzactionsFinanceType();
	ui.dateLineEdit->setText(QDate::currentDate().toString());
	toggleTranzactionsButtons(0);
}

void Dashboard::on_recurrentSpendingsCommandLInkButton_clicked()
{
	ui.stackedWidget->setCurrentWidget(ui.periodicallyOutcomePage);
	on_recurenciesSelected();
}

void Dashboard::on_outcomeBackToCategoriesPushButton_clicked()
{
	ui.stackedWidget->setCurrentWidget(ui.categoriesPage);
}

void Dashboard::on_addoutcomePushButton_clicked()
{
	AddOutComeDialog addoutcomedialog(map_cards, usernameLoggedIn, this);
	addOutcomeExec(addoutcomedialog);
}

void Dashboard::on_transportCommandLinkButton_clicked()
{
	refreshCash();
	std::string categoryName = "Public Transport & Taxi";
	AddSpendingsDialog addspendingsdialog(categoryName, this->cash_details, this->map_cards, this->usernameLoggedIn);
	addSpendingExec(addspendingsdialog);
}

void Dashboard::on_educationCommandLinkButton_clicked()
{
	refreshCash();
	std::string categoryName = "Education";
	AddSpendingsDialog addspendingsdialog(categoryName, this->cash_details, this->map_cards, this->usernameLoggedIn);
	addSpendingExec(addspendingsdialog);
}

void Dashboard::on_shoppingCommandLinkButton_clicked()
{
	refreshCash();
	std::string categoryName = "Shopping";
	AddSpendingsDialog addspendingsdialog(categoryName, this->cash_details, this->map_cards, this->usernameLoggedIn);
	addSpendingExec(addspendingsdialog);
}

void Dashboard::on_freeTimeCommandLinkButton_clicked()
{
	refreshCash();
	std::string categoryName = "Free Time";
	AddSpendingsDialog addspendingsdialog(categoryName, this->cash_details, this->map_cards, this->usernameLoggedIn);
	addSpendingExec(addspendingsdialog);
}

void Dashboard::on_holidayCommandLinkButton_clicked()
{
	refreshCash();
	std::string categoryName = "Holiday & Travel";
	AddSpendingsDialog addspendingsdialog(categoryName, this->cash_details, this->map_cards, this->usernameLoggedIn);
	addSpendingExec(addspendingsdialog);
}

void Dashboard::on_goalsCommandLinkButton_clicked()
{
	ui.stackedWidget->setCurrentWidget(ui.goalsPage);
}

void Dashboard::on_savingsCommandLinkButton_clicked()
{
	ui.stackedWidget->setCurrentWidget(ui.savingsPage);
}

void Dashboard::on_budgetCommandLinkButton_clicked()
{
	turnIntoBudgetSet(this->currentlyOnBudget);
	ui.stackedWidget->setCurrentWidget(ui.budgetPage);
	ui.budgetISOLabel->setText(this->userCashCurrencyISO.c_str());
}

void Dashboard::on_savingsbackToGoalsPushButton_clicked()
{
	ui.stackedWidget->setCurrentWidget(ui.goalsPage);
}

void Dashboard::on_budgetbackToGoalsPushButton_clicked()
{
	ui.stackedWidget->setCurrentWidget(ui.goalsPage);
}

void Dashboard::on_backToDashboardGoalsLinkButton_clicked()
{
	ui.stackedWidget->setCurrentWidget(ui.dashboardPage);
}

void Dashboard::on_budgetpickStartDatePushButton_clicked()
{
	DatePickerDialog datePicker(this);
	datePicker.calendarWidget->setMinimumDate(QDate::currentDate());
	if (datePicker.exec())
	{
		if (datePicker.result() == QDialog::Accepted)
		{
			ui.budgetStartDateLineEdit->setText(datePicker.calendarWidget->selectedDate().toString());
		}
	}
}

void Dashboard::on_budgetpickEndDatePushButton_clicked()
{
	DatePickerDialog datePicker(this);
	datePicker.calendarWidget->setMinimumDate(QDate::currentDate().addDays(1));
	if (datePicker.exec())
	{
		if (datePicker.result() == QDialog::Accepted)
		{
			ui.budgetEndDateLineEdit->setText(datePicker.calendarWidget->selectedDate().toString());
		}
	}
}

void Dashboard::on_budgetSetPushButton_clicked()
{
	bool stillConnectedWaitingForAnswer = true;
	QMessageBox* msgBox = new QMessageBox;
	if (ui.budgetStartDateLineEdit->text() == "" || ui.budgetEndDateLineEdit->text() == "" || ui.valueNotToExceedLineEdit->text() == "")
	{
		msgBox->setText("Please fill all the boxes!");
		msgBox->show();
		QTimer::singleShot(2250, msgBox, SLOT(close()));
		return;
	}
	float valBudget = ui.valueNotToExceedLineEdit->text().toFloat();
	if (valBudget < 0)
	{
		msgBox->setText("Only positive numbers allowed!");
		msgBox->show();
		QTimer::singleShot(2250, msgBox, SLOT(close()));
		return;
	}
	std::string startDate = QDate::fromString(ui.budgetStartDateLineEdit->text()).toString("yyyy-MM-dd").toStdString();
	std::string endDate = QDate::fromString(ui.budgetEndDateLineEdit->text()).toString("yyyy-MM-dd").toStdString();
	
	Client& c = Client::getInstance();
	c.Incoming().clear();
	clever::BudgetHandler newBudget(startDate, endDate, valBudget);
	c.UserAddBudget(this->usernameLoggedIn, newBudget);
	while (c.Incoming().empty())
	{
		if (!c.IsConnected())
		{
			msgBox->setText("Server down! Client disconnected!");
			msgBox->show();
			stillConnectedWaitingForAnswer = false;
			break;
		}
	}
	if (!c.Incoming().empty() && stillConnectedWaitingForAnswer)
	{
		auto msg = c.Incoming().pop_front().msg;
		if (msg.header.id == clever::MessageType::ServerAddBudgetResponse)
		{
			char responseBack[1024];
			msg >> responseBack;
			if (strcmp(responseBack, "SuccessAddBudget") == 0)
			{
				msgBox->setText("Budget added with succes!");
				msgBox->show();
				QTimer::singleShot(2250, msgBox, SLOT(close()));
				this->user_budget = newBudget;
				this->currentlyOnBudget = true;
				turnIntoBudgetSet(currentlyOnBudget);
			}
			else
			{
				msgBox->setText("Failed to add budget. Server problem. Try again.");
				msgBox->show();
				QTimer::singleShot(2250, msgBox, SLOT(close()));
			}
		}
	}
}

void Dashboard::on_deleteBudgetPushButton_clicked()
{
	// delete current budget and restore context on budgetPage.
	bool stillConnectedWaitingForAnswer = true;
	QMessageBox* msgBox = new QMessageBox;
	Client& c = Client::getInstance();
	c.Incoming().clear();
	c.UserDeleteBudget (this->usernameLoggedIn);
	while (c.Incoming().empty())
	{
		if (!c.IsConnected())
		{
			msgBox->setText("Server down! Client disconnected!");
			msgBox->show();
			stillConnectedWaitingForAnswer = false;
			break;
		}
	}
	if (!c.Incoming().empty() && stillConnectedWaitingForAnswer)
	{
		auto msg = c.Incoming().pop_front().msg;
		if (msg.header.id == clever::MessageType::ServerDeleteBudgetResponse)
		{
			char responseBack[1024];
			msg >> responseBack;
			if (strcmp(responseBack, "SuccessDeleteBudget") == 0)
			{
				msgBox->setText("Budget deleted with succes!");
				msgBox->show();
				QTimer::singleShot(2250, msgBox, SLOT(close()));
				ui.budgetEndDateLineEdit->clear();
				ui.budgetStartDateLineEdit->clear();
				ui.valueNotToExceedLineEdit->clear();
				this->currentlyOnBudget = false;
				this->turnIntoBudgetSet(currentlyOnBudget);
			}
			else
			{
				msgBox->setText("Failed to delet budget. Server problem. Try again.");
				msgBox->show();
				QTimer::singleShot(2250, msgBox, SLOT(close()));
			}
		}
	}
}

void Dashboard::on_statisticsCommandLinkButton_clicked()
{
	ui.stackedWidget->setCurrentWidget(ui.statisticsPage);
	this->init_statistics();
}

void Dashboard::on_healthCommandLinkButton_clicked()
{
	refreshCash();
	std::string categoryName = "Health & Self-Care";
	AddSpendingsDialog addspendingsdialog(categoryName, this->cash_details, this->map_cards, this->usernameLoggedIn);
	addSpendingExec(addspendingsdialog);
}

void Dashboard::loadTranzactionsHistory()
{
	// loads all transactions
	bool allGood = false;
	bool stillConnectedWaitingForAnswer = true;
	QMessageBox* msgBox = new QMessageBox;
	Client& c = Client::getInstance();
	c.Incoming().clear();
	c.UserGetTranzactions(this->usernameLoggedIn);
	while (c.Incoming().empty())
	{
		if (!c.IsConnected())
		{
			msgBox->setText("Server down! Client disconnected!");
			msgBox->show();
			stillConnectedWaitingForAnswer = false;
			break;
		}
	}
	int tranzactionsToCome = 0;
	int tranzactionIndex = 0;
	if (!c.Incoming().empty())
	{
		auto msg = c.Incoming().pop_front().msg;
		if (msg.header.id == clever::MessageType::ServerGetTranzactionsResponse)
		{
			msg >> tranzactionsToCome;
		}
	}
	if (tranzactionsToCome > 0)
	{
		ui.listWidget->clear();
		all_user_tranzactions.clear();
	}
	else
	{
		return;
	}
	while (stillConnectedWaitingForAnswer)
	{
		if (!c.IsConnected())
		{
			msgBox->setText("Server down! Client disconnected!");
			msgBox->show();
			stillConnectedWaitingForAnswer = false;
			break;
		}
		if (tranzactionIndex == tranzactionsToCome)
		{
			stillConnectedWaitingForAnswer = false;
			allGood = true;
			break;
		}
		if (!c.Incoming().empty())
		{
			auto msg = c.Incoming().pop_front().msg;
			if (msg.header.id == clever::MessageType::ServerGetTranzactionsResponse)
			{
				char tranzactionTitle[1024]; msg >> tranzactionTitle;
				char tranzactionSource[1024]; msg >> tranzactionSource;
				char tranzactionDestination[1024]; msg >> tranzactionDestination;
				char tranzactionTimestamp[1024]; msg >> tranzactionTimestamp;
				char tranzactionFinanceName[1024]; msg >> tranzactionFinanceName;
				unsigned int tranzType; msg >> tranzType;
				float tranzVal; msg >> tranzVal;

				char tranzactionCurrencyISO[1024]; msg >> tranzactionCurrencyISO;
				char tranzactionDescription[1024]; msg >> tranzactionDescription;
				char tranzactionCategoryName[1024]; msg >> tranzactionCategoryName;

				clever::TranzactionType trType = (tranzType == 1) ? clever::TranzactionType::Income : clever::TranzactionType::Spending;

				clever::TranzactionHandler newTranzaction(tranzactionTitle,
					tranzactionSource,
					tranzactionDestination,
					tranzactionTimestamp,
					tranzactionFinanceName,
					trType,
					tranzVal,
					tranzactionCurrencyISO,
					tranzactionDescription,
					tranzactionCategoryName);

				all_user_tranzactions.push_back(newTranzaction);
				tranzactionIndex++;
			}
		}
	}
	if (allGood)
	{
		// now add to that listWidget custom items.
		// listWidget->addItem...
		updateTranzactionsDefault();
	}
}

void Dashboard::loadRecurencies()
{
	bool stillConnectedWaitingForAnswer = true;
	QMessageBox* msgBox = new QMessageBox;
	Client& c = Client::getInstance();
	c.Incoming().clear();
	c.UsernameGetRecurenciesDetails(this->usernameLoggedIn);
	while (c.Incoming().empty())
	{
		if (!c.IsConnected())
		{
			msgBox->setText("Server down! Client disconnected!");
			msgBox->show();
			stillConnectedWaitingForAnswer = false;
			break;
		}
	}
	int recurenciesToCome = 0;
	int recurenciesIndex = 0;
	if (!c.Incoming().empty())
	{
		auto msg = c.Incoming().pop_front().msg;
		if (msg.header.id == clever::MessageType::ServerGetRecurenciesResponse)
		{
			msg >> recurenciesToCome;
		}
	}
	while (stillConnectedWaitingForAnswer)
	{
		if (!c.IsConnected())
		{
			msgBox->setText("Server down! Client disconnected!");
			msgBox->show();
			stillConnectedWaitingForAnswer = false;
			break;
		}
		if (recurenciesIndex == recurenciesToCome)
		{
			stillConnectedWaitingForAnswer = false;
			break;
		}
		if (!c.Incoming().empty())
		{
			auto msg = c.Incoming().pop_front().msg;
			if (msg.header.id == clever::MessageType::ServerGetRecurenciesResponse)
			{
				char recurenciesName[1024]; msg >> recurenciesName;
				char recurenciesReceiver[1024]; msg >> recurenciesReceiver;
				char recurenciesCard[1024]; msg >> recurenciesCard;
				char recurenciesISO[1024]; msg >> recurenciesISO;
				char dayOfMonth[1024]; msg >> dayOfMonth;
				char recurenciesTypeID[1024]; msg >> recurenciesTypeID;
				float recurenciesValue; msg >> recurenciesValue;

				clever::FinanceTypeCredentialHandler newIncome(recurenciesName, recurenciesReceiver, recurenciesISO, dayOfMonth, recurenciesCard, recurenciesValue, recurenciesTypeID);
				map_recurencies.insert(std::pair<std::string, clever::FinanceTypeCredentialHandler>(recurenciesName, newIncome));
				recurenciesIndex++;
			}
		}
	}
	loadRecurenciesComboBoxes();
}

void Dashboard::loadRecurenciesComboBoxes()
{
	ui.incomePicker->clear();
	ui.outcomePicker->clear();

	for (auto& it : map_recurencies)
	{
		if (strcmp(it.second.getFinanceTypeRecurencies(), "1") == 0)
		{
			ui.incomePicker->addItem(it.second.getFinanceTypeName());
		}
		else
		{
			ui.outcomePicker->addItem(it.second.getFinanceTypeName());
		}
	}
}
void Dashboard::addIncomeExec(AddIncomeDialog& adi)
{
	if (adi.exec())
	{
		if (adi.result() == QDialog::Accepted)
		{
			ui.incomePicker->addItem(adi.incomeNameLineEdit->text());
			ui.incomePicker->setCurrentText(adi.incomeNameLineEdit->text());
			ui.incomeNameLineEdit->setText(adi.incomeNameLineEdit->text());
			ui.incomeValueLineEdit->setText(adi.incomeValue->text());
			ui.dayOfMonthLineEdit->setText(adi.dayOfMonthComboBox->currentText());
			clever::FinanceTypeCredentialHandler newRecurencies(adi.incomeNameLineEdit->text().toStdString(), adi.incomeSourceLineEdit->text().toStdString(), adi.cardCurrencyISO->text().toStdString(), adi.dayOfMonthComboBox->currentText().toStdString(), adi.cardsIncomeComboBox->currentText().toStdString(), adi.incomeValue->text().toFloat(), "1");
			this->map_recurencies.insert(std::pair<std::string, clever::FinanceTypeCredentialHandler>(adi.incomeNameLineEdit->text().toStdString(), newRecurencies));

		}
	}
}

void Dashboard::addOutcomeExec(AddOutComeDialog& ado)
{
	if (ado.exec())
	{
		if (ado.result() == QDialog::Accepted)
		{
			ui.outcomePicker->addItem(ado.OutcomeNameLineEdit->text());
			ui.outcomePicker->setCurrentText(ado.OutcomeNameLineEdit->text());
			ui.outcomeNameLineEdit->setText(ado.OutcomeNameLineEdit->text());
			ui.outcomeValueLineEdit->setText(ado.outcomeValue->text());
			ui.outcomeDayOfMonthLineEdit->setText(ado.dayOfMonthComboBox->currentText());
			clever::FinanceTypeCredentialHandler newRecurencies(ado.OutcomeNameLineEdit->text().toStdString(), ado.OutcomeDestinationLineEdit->text().toStdString(), ado.cardCurrencyISO->text().toStdString(), ado.dayOfMonthComboBox->currentText().toStdString(), ado.cardsOutcomeComboBox->currentText().toStdString(), ado.outcomeValue->text().toFloat(), "2");
			this->map_recurencies.insert(std::pair<std::string, clever::FinanceTypeCredentialHandler>(ado.OutcomeNameLineEdit->text().toStdString(), newRecurencies));
		}
	}
}

void Dashboard::filterTranzactionsBy(TranzactionFilters filterApplied)
{
	switch (filterApplied)
	{

	case TranzactionFilters::TranzactionFinanceName:
		// do stuff.
		break;

	case TranzactionFilters::TranzactionDate:
		// do stuff;
		break;
	case TranzactionFilters::TranzactionType:
		// do stuff;
		break;
	default:
		break;
	}
}

std::string Dashboard::getCurrentDateTime()
{
	std::string dateTimeX = "";
	std::time_t t = std::time(0); // get time now.
	std::tm* now = std::localtime(&t);
	dateTimeX += std::to_string(1900 + (now->tm_year));
	dateTimeX += "-";
	int currMonth = 1 + (now->tm_mon);
	if (currMonth < 10)
	{
		dateTimeX += "0";
	}
	dateTimeX += std::to_string(currMonth);
	dateTimeX += "-";
	int currDay = now->tm_mday;
	if (currDay < 10)
	{
		dateTimeX += "0";
	}
	dateTimeX += std::to_string(currDay);
	dateTimeX += " ";
	int currHour = now->tm_hour;
	if (currHour < 10)
	{
		dateTimeX += "0";
	}
	dateTimeX += std::to_string(currHour);
	dateTimeX += ":";
	int currMin = now->tm_min;
	if (currMin < 10)
	{
		dateTimeX += "0";

	}
	dateTimeX += std::to_string(currMin);
	dateTimeX += ":";
	int currSec = now->tm_sec;
	if (currSec < 10)
	{
		dateTimeX += "0";
	}
	dateTimeX += std::to_string(currSec);

	return dateTimeX;
}

std::string Dashboard::getFloatText2Decimal(float value)
{
	std::string valToShow = std::to_string(value);
	if (valToShow.find('.'))
	{
		valToShow.erase(valToShow.find('.') + 3);
	}
	return valToShow;
}



void Dashboard::loadSavings()
{
	bool allGood = false;
	bool stillConnectedWaitingForAnswer = true;
	QMessageBox* msgBox = new QMessageBox;
	Client& c = Client::getInstance();
	c.Incoming().clear();
	c.UsernameGetSavings(this->usernameLoggedIn);
	while (c.Incoming().empty())
	{
		if (!c.IsConnected())
		{
			msgBox->setText("Server down! Client disconnected!");
			msgBox->show();
			stillConnectedWaitingForAnswer = false;
			break;
		}
	}
	int savingsToCome = 0;
	int savingsIndex = 0;
	if (!c.Incoming().empty())
	{
		auto msg = c.Incoming().pop_front().msg;
		if (msg.header.id == clever::MessageType::ServerGetSavingsResponse)
		{
			msg >> savingsToCome;
		}
	}
	if (savingsToCome > 0)
	{
		if (ui.savingPicker->itemText(0) == "No Savings added to this account.")
		{
			ui.savingPicker->removeItem(0);
		}
		map_savings.clear();
	}
	else
	{
		return;
	}
	while (stillConnectedWaitingForAnswer)
	{
		if (!c.IsConnected())
		{
			msgBox->setText("Server down! Client disconnected!");
			msgBox->show();
			QTimer::singleShot(2250, msgBox, SLOT(close()));
			stillConnectedWaitingForAnswer = false;
			break;
		}
		if (savingsIndex == savingsToCome)
		{
			allGood = true;
			stillConnectedWaitingForAnswer = false;
			break;
		}
		if (!c.Incoming().empty())
		{
			auto msg = c.Incoming().pop_front().msg;
			if (msg.header.id == clever::MessageType::ServerGetSavingsResponse)
			{
				char savingTitle[1024]; msg >> savingTitle;
				float savingGoal; msg >> savingGoal;
				float savingCurrentMoney; msg >> savingCurrentMoney;
				char savingCurrencyISO[1024]; msg >> savingCurrencyISO;
				char savingInitialDate[1024]; msg >> savingInitialDate;

				clever::SavingHandler newSaving(savingTitle, savingGoal, savingCurrencyISO, savingInitialDate, savingCurrentMoney);
				map_savings.insert(std::pair<std::string, clever::SavingHandler>(savingTitle, newSaving));
				ui.savingPicker->addItem(newSaving.getSavingTitle());
				savingsIndex++;
			}
		}
	}
}

void Dashboard::on_savingPicker_currentTextChanged(const QString& SavingChosen)
{
	ui.savingTitleLineEdit->setText(SavingChosen);
	float val = this->map_savings[SavingChosen.toStdString()].getSavingGoal();
	std::string valToShow = std::to_string(val);
	if (valToShow.find('.'))
	{
		valToShow.erase(valToShow.find('.') + 3);
	}
	ui.savingGoalLineEdit->setText(valToShow.c_str());
	ui.savingISOLabel->setText(this->map_savings[SavingChosen.toStdString()].getSavingCurrencyISO());
	float currMoney = this->map_savings[SavingChosen.toStdString()].getSavingCurrMoney();
	int percent = (int)((currMoney / val) * 100.0);
	ui.savingCurrProgressBar->setValue(percent);
	ui.savingDateEstablishmentLineEdit->setText(this->map_savings[SavingChosen.toStdString()].getSavingInitialDate());
}

void Dashboard::on_addSavingPushButton_clicked()
{
	std::string datetime = this->getCurrentDateTime();
	std::stringstream ss(datetime);
	std::string date;
	std::getline(ss, date, ' ');
	AddSavingDialog asd(date,this->usernameLoggedIn, this);
	asd.isoLabel->setText(ui.preferenceCurrencyComboBox->currentText());
	if (asd.exec())
	{
		if (asd.result() == QDialog::Accepted)
		{
			std::string savingTitle = asd.savingTitleLineEdit->text().toStdString();
			float goal = asd.savingGoalLineEdit->text().toFloat();
			std::string savingCurrencyISO = asd.isoLabel->text().toStdString();
			clever::SavingHandler newSaving(savingTitle, goal, savingCurrencyISO, date, 0.0f);
			map_savings.insert(std::pair<std::string, clever::SavingHandler>(savingTitle, newSaving));
			ui.savingPicker->addItem(newSaving.getSavingTitle());
			ui.savingPicker->setCurrentText(newSaving.getSavingTitle());
		}
	}
}

void Dashboard::on_addFundsSavingPushButton_clicked()
{
	QMessageBox* msgBox = new QMessageBox;
	if (ui.savingPicker->itemText(0) == "No Savings added to this account.")
	{
		msgBox->setText("First add savings!");
		msgBox->show();
		QTimer::singleShot(2000, msgBox, SLOT(close()));
		return;
	}
	if (ui.savingCurrProgressBar->value() == 100)
	{
		msgBox->setText("Saving goal is completed! Congrats!");
		msgBox->show();
		QTimer::singleShot(2000, msgBox, SLOT(close()));
		return;
	}
	AddFundsToSavingDialog afts(this->usernameLoggedIn,this->map_cards,this);
	afts.savingTitle->setText(ui.savingPicker->currentText());
	if (afts.exec())
	{
		if (afts.result() == QDialog::Accepted)
		{
			// update current money of saving and update progress bar.
			this->map_cards = afts.getMapCardsUpdated();
			float currMoneyOld = this->map_savings[afts.savingTitle->text().toStdString()].getSavingCurrMoney();
			this->map_savings[afts.savingTitle->text().toStdString()].setSavingCurrMoney(currMoneyOld+afts.valueLineEdit->text().toFloat());
			float currMoney = this->map_savings[afts.savingTitle->text().toStdString()].getSavingCurrMoney();
			float val = this->map_savings[afts.savingTitle->text().toStdString()].getSavingGoal();
			int percent = (int)((currMoney / val) * 100.0);
			ui.savingCurrProgressBar->setValue(percent);
		}
	}
}
void Dashboard::loadBudgets()
{

	bool stillConnectedWaitingForAnswer = true;
	QMessageBox* msgBox = new QMessageBox;
	Client& c = Client::getInstance();
	c.Incoming().clear();
	c.UserGetBudget(this->usernameLoggedIn);
	while (c.Incoming().empty())
	{
		if (!c.IsConnected())
		{
			msgBox->setText("Server down! Client disconnected!");
			msgBox->show();
			stillConnectedWaitingForAnswer = false;
			break;
		}
	}
	if (!c.Incoming().empty() && stillConnectedWaitingForAnswer)
	{
		auto msg = c.Incoming().pop_front().msg;
		if (msg.header.id == clever::MessageType::ServerGetBudgetResponse)
		{
			char responseBack[1024]; msg >> responseBack;
			if (strcmp(responseBack, "SuccessGetBudget") == 0)
			{
				char budgetStartDate[1024]; msg >> budgetStartDate;
				char budgetEndDate[1024]; msg >> budgetEndDate;
				float budgetValue; msg >> budgetValue;
				
				this->user_budget.setBudgetStartDate(budgetStartDate);
				this->user_budget.setBudgetEndDate(budgetEndDate);
				this->user_budget.setBudgetValue(budgetValue);
				this->currentlyOnBudget = true;

				this->turnIntoBudgetSet(currentlyOnBudget);
				if (strcmp(this->user_budget.getBudgetEndDate(), this->getCurrentDateTime().substr(0,10).c_str()) == 0)
				{
					std::string result = "The budget goal is over! We can remove it for you to start another one!";
					ui.currentlyOnBudgetLabel->setText(result.c_str());
				}
			}
			else
			{
				this->currentlyOnBudget = false;
				this->turnIntoBudgetSet(currentlyOnBudget);
			}
		}
	}
}

void Dashboard::turnIntoBudgetSet(bool isUserOnBudget)
{
	ui.deleteBudgetPushButton->setEnabled(isUserOnBudget); ui.deleteBudgetPushButton->setVisible(isUserOnBudget);
	ui.currentlyOnBudgetLabel->setEnabled(isUserOnBudget); ui.currentlyOnBudgetLabel->setVisible(isUserOnBudget);
	ui.budgetStartDateLineEdit->setReadOnly(isUserOnBudget); ui.budgetStartDateLineEdit->setEnabled(!isUserOnBudget);
	ui.budgetEndDateLineEdit->setReadOnly(isUserOnBudget); ui.budgetEndDateLineEdit->setEnabled(!isUserOnBudget);
	ui.budgetSetPushButton->setEnabled(!isUserOnBudget);
	ui.valueNotToExceedLineEdit->setReadOnly(isUserOnBudget); ui.valueNotToExceedLineEdit->setEnabled(!isUserOnBudget);
	ui.budgetpickStartDatePushButton->setEnabled(!isUserOnBudget);
	ui.budgetpickEndDatePushButton->setEnabled(!isUserOnBudget);

	if (isUserOnBudget)
	{
		ui.budgetStartDateLineEdit->setText(user_budget.getBudgetStartDate());
		ui.budgetEndDateLineEdit->setText(user_budget.getBudgetEndDate());
		ui.valueNotToExceedLineEdit->setText(std::to_string(user_budget.getBudgetValue()).c_str());
	}
	else
	{
		ui.budgetStartDateLineEdit->setText(QDate::currentDate().toString());
	}
}

float Dashboard::generateIncomeForMonth(std::string monthCode)
{
	std::map<int, std::string> month_code = getMonthCodeMap();
	int monthCodeIntForTransactions = 0;
	for (auto it = month_code.begin(); it != month_code.end(); ++it)
	{
		if (it->second == monthCode)
		{
			monthCodeIntForTransactions = it->first;
			break;
		}
	}
	float incomeForMonth = getTransactionsIncomeForMonthCode(monthCodeIntForTransactions);
	return incomeForMonth;
}

float Dashboard::generateOutcomeForMonth(std::string monthCode)
{
	std::map<int, std::string> month_code = getMonthCodeMap();
	int monthCodeIntForTransactions = 0;
	for (auto it = month_code.begin(); it != month_code.end(); ++it)
	{
		if (it->second == monthCode)
		{
			monthCodeIntForTransactions = it->first;
			break;
		}
	}
	float outcomeForMonth = getTransactionsOutcomeForMonthCode(monthCodeIntForTransactions);
	return outcomeForMonth;
}

void Dashboard::initMonthCodeMap()
{
	month_code_map.insert(std::pair<int, std::string>(1, "JAN"));
	month_code_map.insert(std::pair<int, std::string>(2, "FEB"));
	month_code_map.insert(std::pair<int, std::string>(3, "MAR"));
	month_code_map.insert(std::pair<int, std::string>(4, "APR"));
	month_code_map.insert(std::pair<int, std::string>(5, "MAY"));
	month_code_map.insert(std::pair<int, std::string>(6, "JUN"));
	month_code_map.insert(std::pair<int, std::string>(7, "JUL"));
	month_code_map.insert(std::pair<int, std::string>(8, "AUG"));
	month_code_map.insert(std::pair<int, std::string>(9, "SEP"));
	month_code_map.insert(std::pair<int, std::string>(10, "OCT"));
	month_code_map.insert(std::pair<int, std::string>(11, "NOV"));
	month_code_map.insert(std::pair<int, std::string>(12, "DEC"));
}

std::map<int, std::string>& Dashboard::getMonthCodeMap()
{
	return this->month_code_map;
}

float Dashboard::getTransactionsIncomeForMonthCode(int monthCodeInt)
{
	float totalIn = 0.0f;
	for (auto it = all_user_tranzactions.begin(); it != all_user_tranzactions.end(); ++it)
	{
		if (it->getTranzactionType() == clever::TranzactionType::Income)
		{
			std::string tranMonth = it->getTranzactionTimestamp();
			tranMonth = tranMonth.substr(5, 2); // yyyy-mm-dd hh:mm:ss
			if (std::stoi(tranMonth) == monthCodeInt)
			{
				totalIn += it->getTranzactionValue();
			}
		}
	}
	return totalIn;
}

float Dashboard::getTransactionsOutcomeForMonthCode(int monthCodeInt)
{
	float totalOut = 0.0f;
	for (auto it = all_user_tranzactions.begin(); it != all_user_tranzactions.end(); ++it)
	{
		if (it->getTranzactionType() == clever::TranzactionType::Spending)
		{
			std::string tranMonth = it->getTranzactionTimestamp();
			tranMonth = tranMonth.substr(5, 2); // yyyy-mm-dd hh:mm:ss
			if (std::stoi(tranMonth) == monthCodeInt)
			{
				totalOut += it->getTranzactionValue();
			}
		}
	}
	return totalOut;
}

void Dashboard::on_setIncome_hovered(bool status, int index)
{
	if (status)
	{
		float currValIncomeMonth=this->setIncome->at(index);
		float currValOutcomeMonth=this->setOutcome->at(index);
		std::string valIncomeToShow = "+";
		valIncomeToShow += std::to_string(currValIncomeMonth);
		if (valIncomeToShow.find('.'))
		{
			valIncomeToShow.erase(valIncomeToShow.find('.') + 3);
		}
		valIncomeToShow += " " + userCashCurrencyISO;
		std::string valOutcomeToShow = "-";
		valOutcomeToShow += std::to_string(currValOutcomeMonth);
		if (valOutcomeToShow.find('.'))
		{
			valOutcomeToShow.erase(valOutcomeToShow.find('.') + 3);
		}
		valOutcomeToShow += " " + userCashCurrencyISO;
		ui.incomingValueLabel->setStyleSheet("QLabel {color : green; }");
		ui.outgoingValueLabel->setStyleSheet("QLabel {color : red; }");
		ui.incomingValueLabel->setText(valIncomeToShow.c_str());
		ui.outgoingValueLabel->setText(valOutcomeToShow.c_str());
		float currValDifference = currValIncomeMonth - currValOutcomeMonth;
		if (currValDifference < 0)
		{
			ui.deltaValueLabel->setStyleSheet("QLabel {color : red; }");
		}
		else
		{
			ui.deltaValueLabel->setStyleSheet("QLabel {color : green; }");
		}
		std::string valDifferenceToShow = std::to_string(currValDifference);
		if (valDifferenceToShow.find('.'))
		{
			valDifferenceToShow.erase(valDifferenceToShow.find('.') + 3);
		}
		valDifferenceToShow += " " + userCashCurrencyISO;
		
		ui.deltaValueLabel->setText(valDifferenceToShow.c_str());
	}
}

void Dashboard::on_setOutcome_hovered(bool status, int index)
{
	if (status)
	{
		float currValIncomeMonth = this->setIncome->at(index);
		float currValOutcomeMonth = this->setOutcome->at(index);
		std::string valIncomeToShow = "+";
		valIncomeToShow += std::to_string(currValIncomeMonth);
		if (valIncomeToShow.find('.'))
		{
			valIncomeToShow.erase(valIncomeToShow.find('.') + 3);
		}
		valIncomeToShow += " " + userCashCurrencyISO;
		std::string valOutcomeToShow = "-";
		valOutcomeToShow += std::to_string(currValOutcomeMonth);
		if (valOutcomeToShow.find('.'))
		{
			valOutcomeToShow.erase(valOutcomeToShow.find('.') + 3);
		}
		valOutcomeToShow += " " + userCashCurrencyISO;
		ui.incomingValueLabel->setStyleSheet("QLabel {color : green; }");
		ui.outgoingValueLabel->setStyleSheet("QLabel {color : red; }");
		ui.incomingValueLabel->setText(valIncomeToShow.c_str());
		ui.outgoingValueLabel->setText(valOutcomeToShow.c_str());
		float currValDifference = currValIncomeMonth - currValOutcomeMonth;
		if (currValDifference < 0)
		{
			ui.deltaValueLabel->setStyleSheet("QLabel {color : red; }");
		}
		else
		{
			ui.deltaValueLabel->setStyleSheet("QLabel {color : green; }");
		}
		std::string valDifferenceToShow = std::to_string(currValDifference);
		if (valDifferenceToShow.find('.'))
		{
			valDifferenceToShow.erase(valDifferenceToShow.find('.') + 3);
		}
		valDifferenceToShow += " " + userCashCurrencyISO;

		ui.deltaValueLabel->setText(valDifferenceToShow.c_str());
	}
}
void Dashboard::init_statistics()
{
	std::map<int, std::string> month_code = getMonthCodeMap();
	int startMonth = QDate::currentDate().month();
	startMonth -= 5;
	if (startMonth <= 0)
	{
		startMonth = 12 - abs(startMonth);
	}
	std::string startMonthString = month_code[startMonth];

	QBarSeries* barSeries = new QBarSeries;
	QBarCategoryAxis* axisX = new QBarCategoryAxis;
	std::string firstMonth = month_code[startMonth].c_str();
	std::string secondMonth = month_code[((startMonth + 1) % 12 == 0) ? 12 : (startMonth + 1) % 12].c_str();
	std::string thirdMonth = month_code[((startMonth + 2) % 12 == 0) ? 12 : (startMonth + 2) % 12].c_str();
	std::string fourthMonth = month_code[((startMonth + 3) % 12 == 0) ? 12 : (startMonth + 3) % 12].c_str();
	std::string fifthMonth = month_code[((startMonth + 4) % 12 == 0) ? 12 : (startMonth + 4) % 12].c_str();
	std::string sixthMonth = month_code[((startMonth + 5) % 12 == 0) ? 12 : (startMonth + 5) % 12].c_str();

	axisX->append(firstMonth.c_str());
	axisX->append(secondMonth.c_str());
	axisX->append(thirdMonth.c_str());
	axisX->append(fourthMonth.c_str());
	axisX->append(fifthMonth.c_str());
	axisX->append(sixthMonth.c_str());

	
	barSeries->attachAxis(axisX);
	setIncome = new QBarSet("Income"); setIncome->setColor("green");
	setOutcome = new QBarSet("Outcome"); setOutcome->setColor("red");

	*setIncome << generateIncomeForMonth(firstMonth) << generateIncomeForMonth(secondMonth) << generateIncomeForMonth(thirdMonth) << generateIncomeForMonth(fourthMonth) << generateIncomeForMonth(fifthMonth) << generateIncomeForMonth(sixthMonth);
	*setOutcome << generateOutcomeForMonth(firstMonth) << generateOutcomeForMonth(secondMonth) << generateOutcomeForMonth(thirdMonth) << generateOutcomeForMonth(fourthMonth) << generateOutcomeForMonth(fifthMonth) << generateOutcomeForMonth(sixthMonth);
	on_setIncome_hovered(true, 5);
	connect(setIncome, SIGNAL(hovered(bool, int)), this, SLOT(on_setIncome_hovered(bool, int)));
	connect(setOutcome, SIGNAL(hovered(bool, int)), this, SLOT(on_setOutcome_hovered(bool, int)));

	barSeries->append(setIncome);
	barSeries->append(setOutcome);

	QChart* chart = new QChart;
	chart->addAxis(axisX, Qt::AlignTop);
	chart->addSeries(barSeries);
	chart->legend()->hide();
	chart->setAnimationOptions(QChart::AllAnimations);
	chart->setBackgroundVisible(false);


	ui.monthsStatsChart->setChart(chart);
	ui.monthsStatsChart->setRenderHint(QPainter::Antialiasing);


	
	QLineSeries* lineSeries = new QLineSeries;

	lineSeries->append(0, 0);
	lineSeries->append(1, 100);
	lineSeries->append(2, 0);
	lineSeries->append(3, 0);
	lineSeries->append(4, 0);
	lineSeries->append(5, 50);
	lineSeries->append(6, 100);



	QBarCategoryAxis* capitalX = new QBarCategoryAxis;
	capitalX->append(firstMonth.c_str());
	capitalX->append(secondMonth.c_str());
	capitalX->append(thirdMonth.c_str());
	capitalX->append(fourthMonth.c_str());
	capitalX->append(fifthMonth.c_str());
	capitalX->append(sixthMonth.c_str());


	QChart* capitalChart = new QChart;
	capitalChart->addAxis(capitalX,Qt::AlignBottom);
	capitalChart->addSeries(lineSeries);
	capitalChart->legend()->hide();
	capitalChart->setAnimationOptions(QChart::AllAnimations);
	capitalChart->setBackgroundVisible(false);

	ui.monthsCapitalChart->setChart(capitalChart);
	ui.monthsCapitalChart->setRenderHint(QPainter::Antialiasing);
	

	ui.statisticsTabWidget->setTabText(0, "IN/OUT");
	ui.statisticsTabWidget->setTabText(1, "Your Capital");
	ui.statisticsUserLabel->setText(ui.firstLastNameLabel->text());
	float allFinancesVal = 0.0f;
	for (auto it : map_cards)
	{
		if (strcmp(it.second.getCardCurrencyISO(),userCashCurrencyISO.c_str())==0)
		{
			allFinancesVal += it.second.getCardSold();
		}
	}
	allFinancesVal += std::stof(currUserCash);
	std::string valToShow = std::to_string(allFinancesVal);
	if (valToShow.find('.'))
	{
		valToShow.erase(valToShow.find('.') + 3);
	}
	valToShow += " ";
	valToShow += userCashCurrencyISO;
	ui.statisticsUserTotalFinance->setText(valToShow.c_str());
}
