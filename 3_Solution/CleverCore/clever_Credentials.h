#pragma once
#include "clever_inclusions.h"

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
}