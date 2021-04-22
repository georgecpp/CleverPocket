#pragma once
#include <QWidget>
#include "ui_dashboard.h"
#include <qstackedwidget.h>

class Dashboard : public QWidget, public Ui::Dashboard
{
	Q_OBJECT

public:
	Dashboard(QStackedWidget* parentStackedWidget = Q_NULLPTR, QWidget* parent = Q_NULLPTR);
	~Dashboard();
private:
	Ui::Dashboard ui;
	QStackedWidget* fromStackedWidget;
	void logout();
private slots:
	void on_financesCommandLinkButton_clicked();
	void on_menuItemSelected(int index);
private:
	void prepareOptionsComboBox(QComboBox* comboBoxToPrepare);
};