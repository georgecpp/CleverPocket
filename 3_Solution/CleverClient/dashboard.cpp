#include "dashboard.h"
<<<<<<< Updated upstream
=======
#include "Client.h"
#include <qmessagebox.h>
#include <qtimer.h>
#include <addcarddialog.h>
#include <addfundsdialog.h>
>>>>>>> Stashed changes

Dashboard::Dashboard(QWidget* parent)
	: QWidget(parent)
{
	ui.setupUi(this);
	ui.stackedWidget->setCurrentWidget(ui.dashboardPage);
<<<<<<< Updated upstream
=======
	this->prepareOptionsComboBox(ui.dashboardOptions);
	this->prepareOptionsComboBox(ui.financesOptions);
	this->prepareOptionsComboBox(ui.profileOptions);
	//this->prepareOptionsComboBox(ui.preferencesOptions);
	
>>>>>>> Stashed changes
}

Dashboard::~Dashboard()
{

<<<<<<< Updated upstream
}
=======
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

void Dashboard::addFundsExec(AddFundsDialog& adc)
{
		
			//if (ui.cardPicker->itemText(0) == "Nu exista carduri adaugate la acest cont") //right one
			if((ui.cardPicker->currentText() == QString("Nu exista carduri adaugate la acest cont"))) //testing one
			{
				QMessageBox* msgBox = new QMessageBox;
				msgBox->setText("Please add a card first!");
				msgBox->show();
			}
	
}

void Dashboard::loadCardCurrencyISO(const std::string& cardName)
{
	bool stillConnectedWaitingForAnswer = true;
	QMessageBox* msgBox = new QMessageBox;
	Client& c = Client::getInstance();
	c.Incoming().clear();
	if (this->PATLoggedIn == "") // it means user-login
	{
		c.UsernameGetSelectedCardCurrency(usernameLoggedIn, cardName);
	}
	else // else PAT-login
	{
		c.PATGetSelectedCardCurrency(PATLoggedIn.toStdString(), cardName);
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
			auto msg = c.Incoming().pop_front().msg;
			if (msg.header.id == clever::MessageType::ServerGetCurrencyResponse)
			{
				char cardCurrencyISO[1024]; msg >> cardCurrencyISO;
				
			}
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
>>>>>>> Stashed changes
