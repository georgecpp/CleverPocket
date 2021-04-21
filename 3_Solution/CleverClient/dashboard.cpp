#include "dashboard.h"

Dashboard::Dashboard(QWidget* parent)
	: QWidget(parent)
{
	ui.setupUi(this);
	ui.stackedWidget->setCurrentWidget(ui.dashboardPage);
}

Dashboard::~Dashboard()
{

}