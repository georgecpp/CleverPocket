#include "dashboard.h"
#include "Client.h"
#include <qmessagebox.h>
#include <qtimer.h>
#include <addcarddialog.h>
#include <addfundsdialog.h>

Dashboard::Dashboard(QStackedWidget* parentStackedWidget, QWidget* parent)
	: QWidget(parent)
{

	this->fromStackedWidget = parentStackedWidget;
	ui.setupUi(this);
	ui.stackedWidget->setCurrentWidget(ui.dashboardPage);
	this->prepareOptionsComboBox(ui.dashboardOptions);
	this->prepareOptionsComboBox(ui.financesOptions);
	this->prepareOptionsComboBox(ui.profileOptions);
	//this->prepareOptionsComboBox(ui.preferencesOptions);
	
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

<<<<<<< Updated upstream
void Dashboard::addFundsExec(AddFundsDialog& adc)
=======
void Dashboard()
>>>>>>> Stashed changes
{
		
			//if (ui.cardPicker->itemText(0) == "Nu exista carduri adaugate la acest cont") //right one
			if((ui.cardPicker->currentText() == QString("Nu exista carduri adaugate la acest cont"))) //testing one
			{
				QMessageBox* msgBox = new QMessageBox;
				msgBox->setText("Please add a card first!");
				msgBox->show();
			}
	
}

<<<<<<< Updated upstream
=======
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
>>>>>>> Stashed changes

void Dashboard::rechargeCashExec(RechargeCashDialog& rcd)
{
	if (rcd.exec())
	{
		if (rcd.result() == QDialog::Accepted)
		{

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
		//ui.stackedWidget->setCurrentWidget(ui.preferencesOptions);
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
}

void Dashboard::on_addCardPushButton_clicked()
{
	AddCardDialog addcarddialog(this);
	addcarddialog.exec();
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

<<<<<<< Updated upstream
void Dashboard::on_addFundsPushButton_clicked()
{
	if (this->PATLoggedIn == "")
	{
		AddFundsDialog addfundsdialog(ui.cardPicker->currentText(),usernameLoggedIn, this);
		addFundsExec(addfundsdialog);
		addfundsdialog.exec();
		//load currency

	}
	else
	{
		AddFundsDialog addfundsdialog(ui.cardPicker->currentText(),PATLoggedIn, this);
		addFundsExec(addfundsdialog);
		addfundsdialog.exec();
		//load currency
	}
	
}
=======
void Dashboard::on_financesTabWidget_currentChanged(int index)
{
	//to do wall things
	// se face interogarea bd pt numerar
	// se actualizeaza datele din tab / wallet
}

void Dashboard::on_addCashPushButton_clicked()
{
	QMessageBox* msgBox = new QMessageBox;
	if (ui.cardPicker->itemText(0) == "Nu exista carduri adaugate la acest cont")
	{
		msgBox->setText("First add cards!");
		msgBox->show();
		QTimer::singleShot(2000, msgBox, SLOT(close()));
		return;
	}
	if (this->userCurrencyISO.empty()) //nu este selectata moneda implicita de la preferences 
	{
		msgBox->setText("Please complete the preferences options first!");
		msgBox->show();
		QTimer::singleShot(2000, msgBox, SLOT(close()));
		return;
	}
	if (this->PATLoggedIn == "")
	{
		RechargeCashDialog rechargeCashDialog(map_cards, usernameLoggedIn, this);
		rechargeCashExec(rechargeCashDialog);
	}
	else
	{
		RechargeCashDialog rechargeCashDialog(map_cards, usernameLoggedIn, this);
		rechargeCashExec(rechargeCashDialog);
	}
}


>>>>>>> Stashed changes
