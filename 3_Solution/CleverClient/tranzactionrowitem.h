#pragma once

#include <QWidget>
#include "ui_tranzactionrowitem.h"
#include <qlabel.h>
#include <qlayout.h>
#include <clever_Credentials.h>


class TranzactionRowItem : public QWidget, public Ui::TranzactionRowItem
{
	Q_OBJECT

public:
	TranzactionRowItem();
	TranzactionRowItem(int rowSize, const char* title, const char* date, const char* value, const char* currencyISO, clever::TranzactionType trType);
	~TranzactionRowItem();
	Ui::TranzactionRowItem ui;
	const char* getTranzactionTitle();
	const char* getTranzactionTimestamp();
private:
	QVBoxLayout* layout;
	QHBoxLayout* othersLayout;
	QHBoxLayout* dateLayout;
	QHBoxLayout* titleLayout;
	QLabel* tranzactionTitle;
	QLabel* tranzactionDate;
	QLabel* tranzactionValue;
	QLabel* tranzactionCurrencyISO;
private:
	std::string trStoredTitle;
	std::string trStoredDate;
};
