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

TranzactionRowItem::TranzactionRowItem(int rowSize, const char* title, const char* date, const char* value, const char* currencyISO)
{
	this->layout = new QVBoxLayout();


	tranzactionTitle = new QLabel(title);
	tranzactionDate = new QLabel(date);
	tranzactionValue = new QLabel(value);
	tranzactionCurrencyISO = new QLabel(currencyISO);

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
