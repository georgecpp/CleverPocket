#pragma once
#include "net_common.h"
#include <windows.h>
#include <sqlext.h>
#include <sqltypes.h>
#include <sql.h>

namespace olc
{
	namespace net
	{
		#define SQL_RESULT_LEN 240
		#define SQL_RETURN_CODE_LEN 1000

		template<typename T>
		class dbconnection
		{
		public:
			dbconnection()
			{
				sqlEnvHandle =  NULL;
				sqlConnHandle = NULL;
				sqlStmtHandle = NULL;
			}
			virtual ~dbconnection()
			{
				//close connection and free resources
				SQLFreeHandle(SQL_HANDLE_STMT, sqlStmtHandle);
				SQLDisconnect(sqlConnHandle);
				SQLFreeHandle(SQL_HANDLE_DBC, sqlConnHandle);
				SQLFreeHandle(SQL_HANDLE_ENV, sqlEnvHandle);
			}
			bool OnAttemptToConnect()
			{
				if (SQLAllocHandle(SQL_HANDLE_ENV, SQL_NULL_HANDLE, &sqlEnvHandle) != SQL_SUCCESS)
				{
					return false;
				}
				if (SQLSetEnvAttr(sqlEnvHandle, SQL_ATTR_ODBC_VERSION, (SQLPOINTER)SQL_OV_ODBC3, 0) != SQL_SUCCESS)
				{
					return false;
				}
				if (SQLAllocHandle(SQL_HANDLE_DBC, sqlEnvHandle, &sqlConnHandle)!=SQL_SUCCESS)
				{
					return false;
				}

				std::cout << "[DBCONNECTION] Attempting connection to SQL Server...\n";

				//connect to SQL Server  
				switch (SQLDriverConnect(sqlConnHandle,
					NULL,
					//(SQLWCHAR*)L"DRIVER={SQL Server};SERVER=localhost, 1433;DATABASE=master;UID=username;PWD=password;",
					(SQLWCHAR*)L"DRIVER={SQL Server};SERVER=localhost, 1434;DATABASE=CleverPocket;Trusted=true;",
					SQL_NTS,
					retconstring,
					1024,
					NULL,
					SQL_DRIVER_NOPROMPT)) {
				case SQL_SUCCESS:
					std::cout << "[DBCONNECTION] Successfully connected to SQL Server";
					std::cout << "\n";
					break;
				case SQL_SUCCESS_WITH_INFO:
					std::cout << "[DBCONNECTION] Successfully connected to SQL Server";
					std::cout << "\n";
					break;
				case SQL_INVALID_HANDLE:
					std::cout << "[DBCONNECTION] Could not connect to SQL Server";
					std::cout << "\n";
					return false;
				case SQL_ERROR:
					std::cout << "[DBCONNECTION] Could not connect to SQL Server";
					std::cout << "\n";
					return false;
				default:
					break;
				}

				// if there is a problem connecting then exit
				if (SQLAllocHandle(SQL_HANDLE_STMT, sqlConnHandle, &sqlStmtHandle) != SQL_SUCCESS)
				{
					return false;
				}

				return true;
			}

			void ExecuteQuery(const std::string& query)
			{
				//SQLCHAR queryResult[SQL_RESULT_LEN];

				std::cout << "\n";
				std::cout << "[DBCONNECTION] Executing SQL query...";
				std::cout << "\n";

				if (SQL_SUCCESS != SQLExecDirect(sqlStmtHandle, (SQLWCHAR*)L"SELECT TOP(1000) * FROM[CleverPocket].[dbo].[Users]", SQL_NTS))
				{
					std::cout << "[DBCONNECTION] Error querying SQL Server";
					std::cout << "\n";
					throw - 1;
				}
				else
				{
					//declare output variable and pointer
					SQLCHAR queryResult[SQL_RESULT_LEN];
					SQLINTEGER ptrQueryResult;
					while (SQLFetch(sqlStmtHandle) == SQL_SUCCESS) {
						SQLGetData(sqlStmtHandle, 2, SQL_CHAR, queryResult, SQL_RESULT_LEN, &ptrQueryResult);
						std::cout << "\nQuery Result:\n\n";
						std::cout << queryResult << " ";
					}

					//return queryResult;
				}
			}


		private:
			SQLHANDLE sqlConnHandle;
			SQLHANDLE sqlStmtHandle;
			SQLHANDLE sqlEnvHandle;
			SQLWCHAR retconstring[SQL_RETURN_CODE_LEN];
		};
	}
}