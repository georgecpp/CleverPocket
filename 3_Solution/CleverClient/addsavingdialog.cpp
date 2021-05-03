#include "addsavingdialog.h"
#include <qmessagebox.h>
#include <qtimer.h>
#include <Client.h>

AddSavingDialog::AddSavingDialog(const std::string& dateToday, const std::string& usernameLogged, QWidget *parent)
	: QDialog(parent)
{
	setupUi(this);
	this->username = usernameLogged;
	this->date = dateToday;
}

AddSavingDialog::~AddSavingDialog()
{

}

void AddSavingDialog::on_addSavingPushButton_clicked()
{
	bool stillConnectedWaitingAnswer = true;
	QMessageBox* msgBox = new QMessageBox();
	QString savingTitle = this->savingTitleLineEdit->text();
	QString savingGoal = this->savingGoalLineEdit->text();
	if (savingTitle == "" || savingGoal == "")
	{
		msgBox->setText("Please fill all the boxes!");
		msgBox->show();
		QTimer::singleShot(2250, msgBox, SLOT(close()));
		return;
	}
	Client& c = Client::getInstance();
	c.Incoming().clear();
	clever::SavingHandler saving(savingTitle.toStdString(), savingGoal.toFloat(), this->isoLabel->text().toStdString(), this->date, 0.0f);
	c.UsernameAddSaving(this->username, saving);
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
		if (msg.header.id == clever::MessageType::ServerAddSavingResponse)
		{
			// server has responded back to add saving request.
			char responseBack[1024];
			msg >> responseBack;
			if (strcmp(responseBack, "SuccessAddSaving") == 0)
			{
				// ALL GOOD.
				msgBox->setText("Saving added with success!");
				msgBox->show();
				QTimer::singleShot(2500, msgBox, SLOT(close()));
			}
			else
			{
				msgBox->setText("Couldn't add the new saving! Server problem. Try again.");
				msgBox->show();
				QTimer::singleShot(2500, msgBox, SLOT(close()));
				return;
			}
			stillConnectedWaitingAnswer = false;
		}
	}
	this->done(QDialog::Accepted);
}
