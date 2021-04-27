#include "dashboard.h"
#include "Client.h"
#include <qmessagebox.h>
#include <qtimer.h>
#include <qfiledialog.h>

Dashboard::Dashboard(QStackedWidget* parentStackedWidget, QWidget* parent)
	: QWidget(parent)
{

	this->fromStackedWidget = parentStackedWidget;
	ui.setupUi(this);
	ui.stackedWidget->setCurrentWidget(ui.dashboardPage);
	this->prepareOptionsComboBox(ui.dashboardOptions);
	this->prepareOptionsComboBox(ui.financesOptions);
	this->prepareOptionsComboBox(ui.preferencesOptions);
	this->prepareOptionsComboBox(ui.profileOptions);
	connect(ui.cardPicker, SIGNAL(activated(int)), this, SLOT(on_cardSelected()));
}

Dashboard::Dashboard(const QString& PAT, QStackedWidget* parentStackedWidget, QWidget* parent)
	: Dashboard(parentStackedWidget, parent)
{
	this->PATLoggedIn = PAT;
	this->usernameLoggedIn = "";
	loadCards();
	loadCash();
	loadCurrencyISOS();
	ui.preferenceCurrencyComboBox->setCurrentText(QString(userCashCurrencyISO.c_str()));
}

Dashboard::Dashboard(const std::string& username, QStackedWidget* parentStackedWidget, QWidget* parent)
	: Dashboard(parentStackedWidget, parent)
{
	this->usernameLoggedIn = username;
	this->PATLoggedIn = "";
	loadCards();
	loadCash();
	loadCurrencyISOS();
	ui.preferenceCurrencyComboBox->setCurrentText(QString(userCashCurrencyISO.c_str()));
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
		this->preferenceCurrencyComboBox->clear();
		for (std::vector<std::string>::iterator it = currencyISOS.begin(); it != currencyISOS.end(); it++)
		{
			this->preferenceCurrencyComboBox->addItem(it->c_str());
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

void Dashboard::loadCash()
{
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
		while (stillConnectedWaitingForAnswer)
		{
			if (!c.IsConnected())
			{
				msgBox->setText("Server down! Client disconnected!");
				msgBox->show();
				stillConnectedWaitingForAnswer = false;
				break;
			}
			if (!c.Incoming().empty())
			{
				char responseBack[1024];
				char userCash[1024];
				char userCurrencyISO[1024];
				auto msg = c.Incoming().pop_front().msg;
				if (msg.header.id == clever::MessageType::ServerGetCashResponse)
				{
					msg >> userCurrencyISO >> userCash >> responseBack;
					currUserCash = userCash;
					userCashCurrencyISO = userCurrencyISO;
					if (strcmp(responseBack,"FailGetCashDetails") == 0)
					{
						msgBox->setText("An error occured while getting cash details out...");
						msgBox->show();
						QTimer::singleShot(2500, msgBox, SLOT(close()));
						return;
					}
				}
			}
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

void Dashboard::prepareOptionsComboBox(QComboBox* comboBoxToPrepare)
{
	comboBoxToPrepare->addItem("Currently logged in:");
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
		}
		else
		{
			//Error handling
		}

	}

}

void Dashboard::on_preferenceCurrencyComboBox_currentTextChanged(const QString& arg1)
{
	//TO DO
	//la schimbarea currency-ului din combobox se introduce in bd
}

void Dashboard::rechargeCashExec(RechargeCashDialog& rcd)
{
	rcd.cashCurrencyISOLabel->setText(QString(userCashCurrencyISO.c_str()));
	if (rcd.exec())
	{
		if (rcd.result() == QDialog::Accepted)
		{

		}

	}
}

void Dashboard::on_financesTabWidget_currentChanged(int index)		//changing wall infos for cash instead of cards
{
	//to do wall things
	// se face interogarea bd pt numerar
	// se actualizeaza datele din tab / wallet
	if (ui.financesTabWidget->currentWidget() == ui.cashTab)
	{
		ui.financeNameWallLabel->setText(QString("Cash"));
		ui.currencyWallLabel->setText(QString(userCashCurrencyISO.c_str()));
		ui.soldWallLabel->setText(QString(currUserCash.c_str()));
		ui.currentCashSoldLabel->setText(QString(currUserCash.c_str()));
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
	if (this->userCashCurrencyISO == "0") 
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
		RechargeCashDialog rechargeCashDialog(userCashCurrencyISO, map_cards, usernameLoggedIn, this);
		rechargeCashExec(rechargeCashDialog);
	}
}
