#pragma once
#include <iostream>
#include <string>

namespace clever
{
	class CredentialHandler
	{
	public:
		CredentialHandler() {}
		void setFirstName(const std::string& FirstName)
		{
			this->m_FirstName = FirstName;
		}
		void setLastName(const std::string& LastName)
		{
			this->m_LastName = LastName;
		}
		void setUsername(const std::string& Username)
		{
			this->m_Username = Username;
		}
		void setPassword(const std::string& Password)
		{
			this->m_Password = Password;
		}
		void setEmail(const std::string& Email)
		{
			this->m_Email = Email;
		}
		void setCountryID(const std::string& CountryID)
		{
			this->m_CountryID = CountryID;
		}
		void setPhoneNumber(const std::string& PhoneNumber)
		{
			this->m_PhoneNumber = PhoneNumber;
		}

		const char* getFirstName() const
		{
			if (m_FirstName != "")
			{
				return m_FirstName.c_str();
			}
			return NULL;
		}
		const char* getLastName() const
		{
			if (m_LastName != "")
			{
				return m_LastName.c_str();
			}
			return NULL;
		}
		const char* getUsername() const
		{
			if (m_Username != "")
			{
				return m_Username.c_str();
			}
			return NULL;
		}
		const char* getPassword() const
		{
			if (m_Password != "")
			{
				return m_Password.c_str();
			}
			return NULL;
		}
		const char* getEmail() const
		{
			if (m_Email != "")
			{
				return m_Email.c_str();
			}
			return NULL;
		}
		const char* getCountryID() const
		{
			if (m_CountryID != "")
			{
				return m_CountryID.c_str();
			}
			return NULL;
		}
		const char* getPhoneNumber() const
		{
			if (m_PhoneNumber != "")
			{
				return m_PhoneNumber.c_str();
			}
			return NULL;
		}
	private:
		std::string m_FirstName;
		std::string m_LastName;
		std::string m_Username;
		std::string m_Password;
		std::string m_Email;
		std::string m_CountryID;
		std::string m_PhoneNumber;
	};

	class CardCredentialHandler
	{
	public:
		CardCredentialHandler() {}
		CardCredentialHandler(std::string CardName, std::string CardHolder, std::string CardNumber, std::string CurrencyISO, std::string ValidUntil, float Sold=0.0f)
			: m_CardName(CardName), m_CardHolder(CardHolder), m_CardNumber(CardNumber), m_CurrencyISO(CurrencyISO), m_ValidUntil(ValidUntil), m_Sold(Sold)
		{

		}

		const char* getCardName() const
		{
			if (m_CardName != "")
			{
				return m_CardName.c_str();
			}
			return NULL;
		}
		const char* getCardHolder() const
		{
			if (m_CardHolder != "")
			{
				return m_CardHolder.c_str();
			}
			return NULL;
		}
		const char* getCardNumber() const
		{
			if (m_CardNumber != "")
			{
				return m_CardNumber.c_str();
			}
			return NULL;
		}
		const char* getCardCurrencyISO() const
		{
			if (m_CurrencyISO != "")
			{
				return m_CurrencyISO.c_str();
			}
			return NULL;
		}
		const char* getCardValidUntil() const
		{
			if (m_ValidUntil!="")
			{
				return m_ValidUntil.c_str();
			}
			return NULL;
		}
		float getCardSold() const {
			return m_Sold;
		}
		void setCardSold(float newCardSold)
		{
			this->m_Sold = newCardSold;
		}
		void setCardName(std::string cardName)
		{
			this->m_CardName = cardName;
		}
		void setCardHolder(std::string cardHolder)
		{
			this->m_CardHolder = cardHolder;
		}
		void setCardNumber(std::string cardNumber)
		{
			this->m_CardNumber = cardNumber;
		}
		void setCardCurrencyISO(std::string cardCurrencyISO)
		{
			this->m_CurrencyISO = cardCurrencyISO;
		}
		void setCardValidUntil(std::string validUntil)
		{
			this->m_ValidUntil = validUntil;
		}
	private:
		std::string m_CardName;
		std::string m_CardHolder;
		std::string m_CardNumber;
		std::string m_CurrencyISO;
		std::string m_ValidUntil;
		float m_Sold;
	};

	enum class TranzactionType
	{
		Income=1,
		Spending,
	};

	class TranzactionHandler
	{
	public:
		TranzactionHandler() {}
		TranzactionHandler(std::string title, std::string source, std::string destination, std::string timestamp, std::string financename,
			TranzactionType tranzactionType, float value, std::string currencyISO, std::string description, std::string categoryName)
			: m_TranzactionTitle(title), m_Source(source), m_Destination(destination), m_Timestamp(timestamp),
			m_FinanceName(financename), m_TranzactionType(tranzactionType), m_Value(value), m_CurrencyISO(currencyISO),
			m_Description(description),m_CategoryName(categoryName)
		{

		}
		const char* getTranzactionTitle() const
		{
			if (this->m_TranzactionTitle != "")
			{
				return this->m_TranzactionTitle.c_str();
			}
			return NULL;
		}
		void setTranzactionTitle(std::string newTitle)
		{
			this->m_TranzactionTitle = newTitle;
		}
		const char* getTranzactionSource() const
		{
			if (this->m_Source != "")
			{
				return this->m_Source.c_str();
			}
			return NULL;
		}
		void setTranzactionSource(std::string newSource)
		{
			this->m_Source = newSource;
		}
		const char* getTranzactionDestination() const
		{
			if (this->m_Destination != "")
			{
				return m_Destination.c_str();
			}
			return NULL;
		}
		void setTranzactionDestination(std::string newDestination)
		{
			this->m_Destination = newDestination;
		}
		const char* getTranzactionTimestamp() const
		{
			if (this->m_Timestamp != "")
			{
				return this->m_Timestamp.c_str();
			}
			return NULL;
		}
		void setTranzactionTimestamp(std::string newTimestamp)
		{
			this->m_Timestamp = newTimestamp;
		}
		const char* getTranzactionFinanceName() const
		{
			if (this->m_FinanceName != "")
			{
				return this->m_FinanceName.c_str();
			}
			return NULL;
		}
		void setTranzactionFinanceName(std::string newFinanceName)
		{
			this->m_FinanceName = newFinanceName;
		}

		// 1 - income, 2 - spending
		TranzactionType getTranzactionType() const
		{
			return this->m_TranzactionType;
		}
		void setTranzactionTitle(TranzactionType trType)
		{
			this->m_TranzactionType = trType;
		}
		float getTranzactionValue() const
		{
			return this->m_Value;
		}
		void setTranzactionValue(float newValue)
		{
			this->m_Value = newValue;
		}
		const char* getTranzactionCurrencyISO() const
		{
			if (m_CurrencyISO != "")
			{
				return this->m_CurrencyISO.c_str();
			}
			return NULL;
		}
		void setTranzactionCurrencyISO(std::string newCurrencyISO)
		{
			this->m_CurrencyISO = newCurrencyISO;
		}
		const char* getTranzactionDescription() const
		{
			if (m_Description != "")
			{
				return m_Description.c_str();
			}
			return NULL;
		}
		void setTranzactionDescription(std::string newDescription)
		{
			this->m_Description = newDescription;
		}
		const char* getTranzactionCategoryName() const
		{
			if (this->m_CategoryName != "")
			{
				return this->m_CategoryName.c_str();
			}
			return NULL;
		}
		void setTranzactionCategoryName(std::string newCategoryName)
		{
			this->m_CategoryName = newCategoryName;
		}
	private:
		std::string m_TranzactionTitle;
		std::string m_Source;
		std::string m_Destination;
		std::string m_Timestamp;
		std::string m_FinanceName;
		TranzactionType m_TranzactionType;
		float m_Value;
		std::string m_CurrencyISO;
		std::string m_Description;
		std::string m_CategoryName;
	};

	class FinanceTypeCredentialHandler
	{
	public:
		FinanceTypeCredentialHandler() {}
		FinanceTypeCredentialHandler(std::string FinanceTypeName, std::string FinanceTypeSource,
			std::string FinanceTypeCurrencyISO, std::string DayOfFinanceType, std::string FinanceTypeToCard, float FinanceTypeValue = 0.0f, std::string FinanceTypeRecurencies="1")
			: m_FinanceTypeName(FinanceTypeName), m_FinanceTypeSource(FinanceTypeSource), m_FinanceTypeValue(FinanceTypeValue),
			m_DayOfFinanceType(DayOfFinanceType), m_FinanceTypeToCard(FinanceTypeToCard), m_FinanceTypeCurrencyISO(FinanceTypeCurrencyISO), m_FinanceTypeRecurencies(FinanceTypeRecurencies)
		{

		}
		const char* getFinanceTypeName() const
		{
			if (m_FinanceTypeName != "")
			{
				return m_FinanceTypeName.c_str();
			}
			return NULL;
		}
		const char* getFinanceTypeSource() const
		{
			if (m_FinanceTypeSource != "")
			{
				return m_FinanceTypeSource.c_str();
			}
			return NULL;
		}
		const char* getFinanceTypeCurrencyISO() const
		{
			if (m_FinanceTypeCurrencyISO != "")
			{
				return m_FinanceTypeCurrencyISO.c_str();
			}
			return NULL;
		}
		const char* getDayOfFinanceType() const
		{
			if (m_DayOfFinanceType != "")
			{
				return m_DayOfFinanceType.c_str();
			}
			return NULL;
		}
		const char* getFinanceTypeToCard() const
		{
			if (m_FinanceTypeToCard != "")
			{
				return m_FinanceTypeToCard.c_str();
			}
			return NULL;
		}
		const char* getFinanceTypeRecurencies() const
		{
			if (m_FinanceTypeRecurencies != "")
			{
				return m_FinanceTypeRecurencies.c_str();
			}
			return NULL;
		}
		float getFinanceTypeValue() const {
			return m_FinanceTypeValue;
		}
		void setFinanceTypeValue(float newFinanceTypeValue)
		{
			this->m_FinanceTypeValue = newFinanceTypeValue;
		}
		void setFinanceTypeSource(std::string newFinanceTypeSource)
		{
			this->m_FinanceTypeSource = newFinanceTypeSource;
		}
		void setFinanceTypeCurrencyISO(std::string newFinanceTypeCurrencyISO)
		{
			this->m_FinanceTypeCurrencyISO = newFinanceTypeCurrencyISO;
		}
		void setDayOfFinanceType(std::string newDayOfFinanceType)
		{
			this->m_DayOfFinanceType = newDayOfFinanceType;
		}
		void setFinanceTypeToCard(std::string newFinanceTypeToCard)
		{
			this->m_FinanceTypeToCard = newFinanceTypeToCard;
		}
		void setFinanceTypeName(std::string newFinanceTypeName)
		{
			this->m_FinanceTypeName = newFinanceTypeName;
		}
		void setFinanceTypeRecurencies(std::string newFinanceTypeRecurencies)
		{
			this->m_FinanceTypeRecurencies = newFinanceTypeRecurencies;
		}
	private:
		std::string m_FinanceTypeName;
		std::string m_FinanceTypeCurrencyISO;
		std::string m_FinanceTypeToCard;
		std::string	m_DayOfFinanceType;
		float m_FinanceTypeValue;
		std::string m_FinanceTypeSource;
		std::string m_FinanceTypeRecurencies;
	};

}