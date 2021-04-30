#include "dashboard.h"
#include "Client.h"
#include <qmessagebox.h>
#include <qtimer.h>
#include <qfiledialog.h>
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
	connect(ui.incomePicker, SIGNAL(activated(int)), this, SLOT(on_recurenciesSelected()));
	connect(ui.outcomePicker, SIGNAL(activated(int)), this, SLOT(on_recurenciesSelected()));
	connect(ui.tranzactionTypePicker, SIGNAL(activated(int)), this, SLOT(on_TranzactionTypeSelected()));
	toggleTranzactionsButtons(0);
}

Dashboard::Dashboard(const QString& PAT, QStackedWidget* parentStackedWidget, QWidget* parent)
	: Dashboard(parentStackedWidget, parent)
{
	this->PATLoggedIn = PAT;
	this->usernameLoggedIn = "";
	loadCash();
	this->prepareOptionsComboBox(ui.dashboardOptions);
	this->prepareOptionsComboBox(ui.financesOptions);
	this->prepareOptionsComboBox(ui.preferencesOptions);
	this->prepareOptionsComboBox(ui.profileOptions);
	this->prepareOptionsComboBox(ui.tranzactionsOptions);
	this->prepareOptionsComboBox(ui.categoriesOptions);
	this->prepareOptionsComboBox(ui.incomeOptions);
	this->prepareOptionsComboBox(ui.outcomeOptions);
	loadCards();
	loadRecurencies();
	loadCurrencyISOS();
	ui.preferenceCurrencyComboBox->setCurrentText(QString(userCashCurrencyISO.c_str()));
	loadTranzactionsHistory();
}

Dashboard::Dashboard(const std::string& username, QStackedWidget* parentStackedWidget, QWidget* parent)
	: Dashboard(parentStackedWidget, parent)
{
	this->usernameLoggedIn = username;
	this->PATLoggedIn = "";
	loadCards();
	loadCash();
	loadRecurencies();
	this->prepareOptionsComboBox(ui.dashboardOptions);
	this->prepareOptionsComboBox(ui.financesOptions);
	this->prepareOptionsComboBox(ui.preferencesOptions);
	this->prepareOptionsComboBox(ui.profileOptions);
	this->prepareOptionsComboBox(ui.tranzactionsOptions);
	this->prepareOptionsComboBox(ui.categoriesOptions);
	this->prepareOptionsComboBox(ui.incomeOptions);
	this->prepareOptionsComboBox(ui.outcomeOptions);
	loadCurrencyISOS();
	ui.preferenceCurrencyComboBox->setCurrentText(QString(userCashCurrencyISO.c_str()));
	loadTranzactionsHistory();
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
				msg >> userCurrencyISO >> userCash >> usernameLogged >> firstName >> lastName >> email >> country >> phoneNumber >> picture >>dailyMailState;
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
				if (dailyMailState == "False")
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

void Dashboard::loadRecurencies()
{
	bool stillConnectedWaitingForAnswer = true;
	QMessageBox* msgBox = new QMessageBox;
	Client& c = Client::getInstance();
	c.Incoming().clear();
	//c.UsernameGetCardsDetails(usernameLoggedIn);
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
		if (msg.header.id == clever::MessageType::ServerGetCardsResponse)
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



void Dashboard::loadRecurenciesComboBoxes()
{
	ui.incomePicker->clear();
	// outcomes picker

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

void Dashboard::on_addIncomePushButton_clicked()
{
	if (this->PATLoggedIn == "")
	{
		AddIncomeDialog addincomedialog(map_cards,usernameLoggedIn, this);
		addIncomeExec(addincomedialog);

	}
	else
	{
		AddIncomeDialog addincomedialog(map_cards, PATLoggedIn, this);
		addIncomeExec(addincomedialog);
	}
}

void Dashboard::on_periodicallyIncomePushButton()
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
}

void Dashboard::on_outcomeBackToCategoriesPushButton_clicked()
{
	ui.stackedWidget->setCurrentWidget(ui.categoriesPage);
}

void Dashboard::on_addoutcomePushButton_clicked()
{
	if (this->PATLoggedIn == "")
	{
		AddOutComeDialog addoutcomedialog(map_cards, usernameLoggedIn, this);
		addOutcomeExec(addoutcomedialog);

	}
	else
	{
		AddOutComeDialog addoutcomedialog(map_cards, PATLoggedIn, this);
		addOutcomeExec(addoutcomedialog);
	}
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
}

void Dashboard::on_backToTranzactionsPushButtton_clicked()
{
	ui.stackedWidget->setCurrentWidget(ui.tranzactionsHistoryPage);
}

void Dashboard::on_showFinanceHistory_clicked()
{
	ui.stackedWidget->setCurrentWidget(ui.tranzactionsHistoryPage);
	ui.tranzactionTypePicker->setCurrentIndex(0); // All tranzactions.
	toggleTranzactionsButtons(0);
	ui.dateLineEdit->setText(QDate::currentDate().toString());
	populateTranzactionsFinanceType();
	ui.financeTypePicker->setCurrentText(ui.financeNameWallLabel->text());
}

void Dashboard::loadTranzactionsHistory()
{
	// TO DO.
	// loads all transactions
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

				// now add to that listWidget custom items.
				// listWidget->addItem....
				QListWidgetItem* item = new QListWidgetItem(ui.listWidget);
				ui.listWidget->addItem(item);
				TranzactionRowItem* tranzRowItem = new TranzactionRowItem(ui.listWidget->width(),
					tranzactionTitle,tranzactionTimestamp,std::to_string(tranzVal).c_str(),tranzactionCurrencyISO);
				item->setSizeHint(tranzRowItem->minimumSizeHint());
				ui.listWidget->setItemWidget(item, tranzRowItem);
				tranzactionIndex++;
			}
		}
	}
}
