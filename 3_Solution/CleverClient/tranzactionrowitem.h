#pragma once

#include <QWidget>
#include "ui_tranzactionrowitem.h"
#include <qlabel.h>
#include <qlayout.h>

class TranzactionRowItem : public QWidget, public Ui::TranzactionRowItem
{
	Q_OBJECT

public:
	TranzactionRowItem();
	TranzactionRowItem(int rowSize, const char* title, const char* date, const char* value, const char* currencyISO);
	~TranzactionRowItem();
	Ui::TranzactionRowItem ui;
private:
	QVBoxLayout* layout;
	QHBoxLayout* othersLayout;
	QHBoxLayout* dateLayout;
	QHBoxLayout* titleLayout;
	QLabel* tranzactionTitle;
	QLabel* tranzactionDate;
	QLabel* tranzactionValue;
	QLabel* tranzactionCurrencyISO;
};
