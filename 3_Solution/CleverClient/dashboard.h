#pragma once

#include <QWidget>
#include "ui_dashboard.h"

class Dashboard : public QWidget, public Ui::Dashboard
{
	Q_OBJECT

public:
	Dashboard(QWidget *parent = Q_NULLPTR);
	~Dashboard();
};
