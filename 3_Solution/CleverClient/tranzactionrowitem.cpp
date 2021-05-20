#include "tranzactionrowitem.h"

TranzactionRowItem::TranzactionRowItem()
{

	this->layout = new QVBoxLayout();


	tranzactionTitle = new QLabel("Title");
	tranzactionDate = new QLabel("Date");
	tranzactionValue = new QLabel("+-Value");
	tranzactionCurrencyISO = new QLabel("CurrencyISO");

	this->titleLayout = new QHBoxLayout();
	//this->titleLayout->addWidget(new QLabel(""));
	this->titleLayout->addSpacing(100);
	this->titleLayout->addWidget(this->tranzactionTitle);

	this->othersLayout = new QHBoxLayout();
	this->othersLayout->addWidget(tranzactionValue);
	this->othersLayout->addWidget(tranzactionCurrencyISO);

	this->dateLayout = new QHBoxLayout();
	this->dateLayout->addWidget(tranzactionDate);


	this->layout->addLayout(this->titleLayout);
	this->layout->addLayout(this->othersLayout);
	this->layout->addLayout(this->dateLayout);


	this->setLayout(layout);
}

TranzactionRowItem::TranzactionRowItem(int rowSize, const char* title, const char* date, const char* value, const char* currencyISO, clever::TranzactionType trType)
{
	this->trStoredTitle = title;
	this->trStoredDate = date;

	
	this->layout = new QVBoxLayout();

	std::string val = (trType == clever::TranzactionType::Income) ? "+" : "-";
	val += value;
	tranzactionTitle = new QLabel(title);
	tranzactionTitle->setStyleSheet(" QLabel{background:transparent; color:white; font-family: Whitney;font-size: 12px;}");
	std::string dateTimestamp = date;
	if (dateTimestamp.find('.'))
	{
		dateTimestamp.erase(dateTimestamp.find('.'));
	}
	tranzactionDate = new QLabel(dateTimestamp.c_str());
	tranzactionDate->setStyleSheet(" QLabel{background:transparent; color:white; font-family: Whitney;font-size: 10px;}");
	tranzactionValue = new QLabel(val.c_str());
	if (trType == clever::TranzactionType::Income)
	{
		tranzactionValue->setStyleSheet(" QLabel{background:transparent; color:green; font-family: Whitney;font-size: 10px;}");
	}
	else
	{
		tranzactionValue->setStyleSheet(" QLabel{background:transparent; color:red; font-family: Whitney;font-size: 10px;}");
	}
	tranzactionCurrencyISO = new QLabel(currencyISO);
	tranzactionCurrencyISO->setStyleSheet(" QLabel{background:transparent; color:white; font-family: Whitney;font-size: 10px;}");
	this->titleLayout = new QHBoxLayout();
	//this->titleLayout->addWidget(new QLabel(""));
	this->titleLayout->addSpacing(rowSize/4);
	this->titleLayout->addWidget(this->tranzactionTitle);

	this->othersLayout = new QHBoxLayout();
	this->othersLayout->addWidget(tranzactionValue);
	this->othersLayout->addSpacing(rowSize / 2);
	this->othersLayout->addWidget(tranzactionCurrencyISO);

	this->dateLayout = new QHBoxLayout();
	this->dateLayout->addWidget(tranzactionDate);


	this->layout->addLayout(this->titleLayout);
	this->layout->addLayout(this->othersLayout);
	this->layout->addLayout(this->dateLayout);


	this->setLayout(layout);
}

TranzactionRowItem::~TranzactionRowItem()
{

}

const char* TranzactionRowItem::getTranzactionTitle()
{
	if (trStoredTitle != "")
	{
		return this->trStoredTitle.c_str();
	}
	return nullptr;
}

const char* TranzactionRowItem::getTranzactionTimestamp()
{
	if (trStoredDate != "")
	{
		return this->trStoredDate.c_str();
	}
	return nullptr;
}
