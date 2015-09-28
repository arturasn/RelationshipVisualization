#define _AFXDLL
#include "stdafx.h"
#include "sqlite3.h"
#include <string>
#include <wx/string.h>
#include "ExecuteSqlite.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

#pragma comment (lib, "sqlite3.lib")

void cExecuteSqlite::ExecuteSqliteStatements(CString &RelationFirstTable, CString &RelationSecondTable, CString &RelationFirstField, CString &RelationSecondField, const char *dPath, 
		                                std::vector<std::pair<CString,CString>> &deletefirstrelation, std::vector<std::pair<CString,CString>> &deletesecondrelation, std::vector<std::pair<CString,CString>> &updatefirstrelation,
										std::vector<std::pair<CString,CString>> &updatesecondrelation, std::vector<CString> &deletetriggernames, std::vector<CString> &updatetriggernames, 
										std::vector<CString> &foreignkeyfirsttable, std::vector<CString> &foreignkeysecondtable, std::vector<CString> &foreignkeyfirstfield, 
										std::vector<CString> &foreignkeysecondfield, std::vector<CString> &tablenamesSQL, std::vector<CString> &tablenames, int RelationFlag)
{
	char *zErrMsg;
	sqlite3 *sqlitedatabase;
	int rc;
	rc = sqlite3_open_v2(dPath, &sqlitedatabase, SQLITE_OPEN_READWRITE, NULL);
    if( rc )
	  {
         exit(0);
      }
	sqlite3_exec(sqlitedatabase, "PRAGMA synchronous = OFF", NULL, NULL, &zErrMsg);
	sqlite3_exec(sqlitedatabase, "PRAGMA journal_mode = MEMORY", NULL, NULL, &zErrMsg);
	CString statement(_T("CREATE TRIGGER "));
	if( RelationFlag & 1 )
	{
		statement += RelationFirstTable + RelationSecondTable + _T("Update AFTER UPDATE OF ") + RelationFirstField + _T(" ON ") + RelationFirstTable + _T(" BEGIN UPDATE ")
			      + RelationSecondTable + _T(" SET ") + RelationSecondField + _T(" = new.") + RelationFirstField + _T(" WHERE ") + RelationSecondField + _T(" = OLD.") + RelationFirstField
				  + _T("; END");
		updatefirstrelation.push_back(std::make_pair(RelationFirstTable, RelationFirstField));
		updatesecondrelation.push_back(std::make_pair(RelationSecondTable, RelationSecondField));
		updatetriggernames.push_back(RelationFirstTable + RelationSecondTable + _T("Update"));
		
	}
	else if( RelationFlag & 2 )
	{
		statement += RelationFirstTable + RelationSecondTable + _T("Delete AFTER DELETE ON ") + RelationFirstTable + _T(" FOR EACH ROW BEGIN DELETE FROM ") + RelationSecondTable 
			      + _T( " WHERE ") + RelationSecondField + _T(" = OLD.") + RelationFirstField + _T("; END");
		deletefirstrelation.push_back(std::make_pair(RelationFirstTable, RelationFirstField));
		deletesecondrelation.push_back(std::make_pair(RelationSecondTable, RelationSecondField));
		deletetriggernames.push_back(RelationFirstTable + RelationSecondTable + _T("Delete"));
	}
	else if( RelationFlag & 4 )
	{
		CString Temp;
		unsigned nTableCount = tablenames.size();
		for( unsigned i = 0; i < nTableCount; ++i )
		{
			if( RelationFirstTable == tablenames[i] )
			{
				Temp = tablenamesSQL[i];
				int ind = Temp.Find(_T("("));
				Temp = Temp.Right(Temp.GetLength() - ind);
				CString TemporaryTable = _T("CREATE TABLE `temporarytable` ");
				TemporaryTable += Temp;
				CT2CA pszConvertedAnsiString (TemporaryTable);
				std::string strStd (pszConvertedAnsiString);
				sqlite3_exec(sqlitedatabase, _strdup(strStd.c_str()), NULL, NULL, &zErrMsg);
				CString sql = _T("INSERT INTO temporarytable SELECT * FROM ") + RelationFirstTable + _T(";");
				CT2CA pszConvertedAnsiString2 (sql);
				std::string strStd2 (pszConvertedAnsiString2);
				sqlite3_exec(sqlitedatabase, _strdup(strStd2.c_str()), NULL, NULL, &zErrMsg);
				CString Temp3 = _T("DROP TABLE ") + RelationFirstTable;
				CT2CA pszConvertedAnsiString3 (Temp3);
				std::string strStd3 (pszConvertedAnsiString3);
				sqlite3_exec(sqlitedatabase, _strdup(strStd3.c_str()), NULL, NULL, &zErrMsg);
				statement = _T(", FOREIGN KEY(") + RelationFirstField + _T(") REFERENCES ") + RelationSecondTable + _T("(") + RelationSecondField + _T("))");
				Temp = Temp.Left(Temp.GetLength() - 1);
				Temp += statement;
				CString Temp4 = _T("CREATE TABLE `") + RelationFirstTable + _T("` ") + Temp;
				tablenamesSQL[i] = Temp4;
				CT2CA pszConvertedAnsiString4 (Temp4);
				std::string strStd4 (pszConvertedAnsiString4);
				sqlite3_exec(sqlitedatabase, _strdup(strStd4.c_str()), NULL, NULL, &zErrMsg);
				CString Temp5 = _T("INSERT INTO ") + RelationFirstTable + _T(" SELECT * FROM temporarytable;");
				CT2CA pszConvertedAnsiString5 (Temp5);
				std::string strStd5 (pszConvertedAnsiString5);
				sqlite3_exec(sqlitedatabase, _strdup(strStd5.c_str()), NULL, NULL, &zErrMsg);
				sqlite3_exec(sqlitedatabase, "DROP TABLE temporarytable", NULL, NULL, &zErrMsg);
				foreignkeyfirsttable.push_back(RelationFirstTable);
				foreignkeysecondtable.push_back(RelationSecondTable);
				foreignkeyfirstfield.push_back(RelationFirstField);
				foreignkeysecondfield.push_back(RelationSecondField);
				RecreateDeleteTriggers(RelationFirstTable, RelationSecondTable, RelationFirstField, RelationSecondField, deletefirstrelation, deletesecondrelation, sqlitedatabase);
				RecreateUpdateTriggers(RelationFirstTable, RelationSecondTable, RelationFirstField, RelationSecondField, updatefirstrelation, updatesecondrelation, sqlitedatabase);
				sqlite3_close(sqlitedatabase);
				return;
			}

		}
		
	}
	else if( RelationFlag & 8 )
	{
		unsigned nUpdateCount = updatefirstrelation.size();
		
		for( unsigned i = 0; i < nUpdateCount; ++i )
		{
			if( updatefirstrelation[i].first == RelationFirstTable && updatefirstrelation[i].second == RelationFirstField && updatesecondrelation[i].first == 
				RelationSecondTable && updatesecondrelation[i].second == RelationSecondField)
			{
				updatefirstrelation.erase(updatefirstrelation.begin() + i);
				updatesecondrelation.erase(updatesecondrelation.begin() + i);
				statement = "DROP TRIGGER " + updatetriggernames[i];
				updatetriggernames.erase(updatetriggernames.begin() + i);
				break;
			}
		}
		nUpdateCount = updatefirstrelation.size();
		for( unsigned i = 0; i < nUpdateCount; ++i )
		{
			if( updatefirstrelation[i].first == RelationSecondTable && updatefirstrelation[i].second == RelationSecondField && updatesecondrelation[i].first == 
				RelationFirstTable && updatesecondrelation[i].second == RelationFirstField)
			{
				updatefirstrelation.erase(updatefirstrelation.begin() + i);
				updatesecondrelation.erase(updatesecondrelation.begin() + i);
				statement = "DROP TRIGGER " + updatetriggernames[i];
				updatetriggernames.erase(updatetriggernames.begin() + i);
				break;
			}
		}
	}
	else if( RelationFlag & 16 )
	{
		unsigned nDeleteCount = deletefirstrelation.size();
		for( unsigned i = 0; i < nDeleteCount; ++i )
		{
			if( deletefirstrelation[i].first == RelationFirstTable && deletefirstrelation[i].second == RelationFirstField && deletesecondrelation[i].first == 
					RelationSecondTable && deletesecondrelation[i].second == RelationSecondField)
				{
					deletefirstrelation.erase(deletefirstrelation.begin() + i);
					deletesecondrelation.erase(deletesecondrelation.begin() + i);
					statement = "DROP TRIGGER " + deletetriggernames[i];
					deletetriggernames.erase(deletetriggernames.begin() + i);
					break;
				}
		}
		nDeleteCount = deletefirstrelation.size();
		for( unsigned i = 0; i < nDeleteCount; ++i )
		{
			if( deletefirstrelation[i].first == RelationSecondTable && deletefirstrelation[i].second == RelationSecondField && deletesecondrelation[i].first == 
					RelationFirstTable && deletesecondrelation[i].second == RelationFirstField)
				{
					deletefirstrelation.erase(deletefirstrelation.begin() + i);
					deletesecondrelation.erase(deletesecondrelation.begin() + i);
					statement = "DROP TRIGGER " + deletetriggernames[i];
					deletetriggernames.erase(deletetriggernames.begin() + i);
					break;
				}
		}
	}
	else if(RelationFlag & 32)
	{
		CString Temp;
		unsigned nTableCount = tablenames.size();
		for( unsigned i = 0; i < nTableCount; ++i )
		{
			if( RelationFirstTable == tablenames[i] )
			{
				Temp = tablenamesSQL[i];
				int ind = Temp.Find(_T("("));
				Temp = Temp.Right(Temp.GetLength() - ind);
				CString TemporaryTable = _T("CREATE TABLE `temporarytable` ");
				CString SearchingFK = _T(", FOREIGN KEY(") + RelationFirstField + _T(") REFERENCES ") + RelationSecondTable + _T("(") + RelationSecondField + _T("))");
				Temp.Replace(SearchingFK, _T(")"));
				SearchingFK = SearchingFK.Left(SearchingFK.GetLength() - 1)  + _T(",");
				Temp.Replace(SearchingFK, _T(","));
				CString tablenameSQLreplacement = _T("CREATE TABLE `") + RelationFirstTable + _T("` ");
				tablenamesSQL[i] = tablenameSQLreplacement + Temp;
				TemporaryTable += Temp;
				CT2CA pszConvertedAnsiString (TemporaryTable);
				std::string strStd (pszConvertedAnsiString);
				sqlite3_exec(sqlitedatabase, _strdup(strStd.c_str()), NULL, NULL, &zErrMsg);
				CString Temp2 = _T("INSERT INTO temporarytable SELECT * FROM ") + RelationFirstTable + _T(";");
				CT2CA pszConvertedAnsiString2 (Temp2);
				std::string strStd2 (pszConvertedAnsiString2);
				sqlite3_exec(sqlitedatabase, _strdup(strStd2.c_str()), NULL, NULL, &zErrMsg);
				CString Temp3 = _T("DROP TABLE ") + RelationFirstTable;
				CT2CA pszConvertedAnsiString3 (Temp3);
				std::string strStd3 (pszConvertedAnsiString3);
				sqlite3_exec(sqlitedatabase, _strdup(strStd3.c_str()), NULL, NULL, &zErrMsg);
				CString Temp4 = _T("ALTER TABLE temporarytable RENAME TO ") + RelationFirstTable + _T(";");
				CT2CA pszConvertedAnsiString4 (Temp4);
				std::string strStd4 (pszConvertedAnsiString4);
				sqlite3_exec(sqlitedatabase, _strdup(strStd4.c_str()), NULL, NULL, &zErrMsg);
				unsigned nForeignKeyCount = foreignkeyfirsttable.size();
				for(unsigned i1 = 0; i1 < nForeignKeyCount; ++i1)
				{
					if( foreignkeyfirsttable[i1] == RelationFirstTable && foreignkeysecondtable[i1] == RelationSecondTable && foreignkeyfirstfield[i1] == RelationFirstField 
						&& RelationSecondField == foreignkeysecondfield[i1] )
					{
						foreignkeyfirsttable.erase(foreignkeyfirsttable.begin() + i1);
						foreignkeysecondtable.erase(foreignkeysecondtable.begin() + i1);
						foreignkeyfirstfield.erase(foreignkeyfirstfield.begin() + i1);
						foreignkeysecondfield.erase(foreignkeysecondfield.begin() + i1);
						break;
					}
				}
				RecreateDeleteTriggers(RelationFirstTable, RelationSecondTable, RelationFirstField, RelationSecondField, deletefirstrelation, deletesecondrelation, sqlitedatabase);
				RecreateUpdateTriggers(RelationFirstTable, RelationSecondTable, RelationFirstField, RelationSecondField, updatefirstrelation, updatesecondrelation, sqlitedatabase);
				sqlite3_close(sqlitedatabase);
				return;
			}

		}
		
	}
	CT2CA pszConvertedAnsiString (statement);
	std::string strStd (pszConvertedAnsiString);
	const char *statementforexecution = _strdup(strStd.c_str());
	sqlite3_exec(sqlitedatabase, statementforexecution, NULL, NULL, &zErrMsg);
	sqlite3_close(sqlitedatabase);
}
void cExecuteSqlite::RecreateDeleteTriggers(CString &RelationFirstTable, CString &RelationSecondTable, CString &RelationFirstField, CString &RelationSecondField, std::vector<std::pair<CString,CString>> &deletefirstrelation, 
		                        std::vector<std::pair<CString,CString>> &deletesecondrelation ,sqlite3 *&sqlitedatabase)
{	
	char *zErrMsg;
	unsigned nDeleteCount = deletefirstrelation.size();
	for( unsigned i1 = 0; i1 < nDeleteCount; ++i1 )
	{
		if( RelationFirstTable == deletefirstrelation[i1].first )
		{
			CString RecreateTrigger = _T("CREATE TRIGGER ") +  deletefirstrelation[i1].first + deletefirstrelation[i1].second + 
							            deletesecondrelation[i1].first + deletesecondrelation[i1].second + _T("Delete AFTER DELETE ON ") + RelationFirstTable + 
										_T(" FOR EACH ROW BEGIN DELETE FROM ") + RelationSecondTable + _T( " WHERE ") + RelationSecondField + _T(" = OLD.") + RelationFirstField + _T("; END");
			CT2CA pszConvertedAnsiString (RecreateTrigger);
			std::string strStd (pszConvertedAnsiString);
			sqlite3_exec(sqlitedatabase, _strdup(strStd.c_str()), NULL, NULL, &zErrMsg);
		}
		if( RelationFirstTable == deletesecondrelation[i1].first )
		{
			CString RecreateTrigger = _T("CREATE TRIGGER ") + deletesecondrelation[i1].first + deletesecondrelation[i1].second + deletefirstrelation[i1].first + 
							            deletefirstrelation[i1].second + _T("Delete AFTER DELETE ON ") + RelationSecondTable + 
										_T(" FOR EACH ROW BEGIN DELETE FROM ") + RelationFirstTable + _T( " WHERE ") + RelationFirstField + _T(" = OLD.") + RelationSecondField + _T("; END");
			CT2CA pszConvertedAnsiString (RecreateTrigger);
			std::string strStd (pszConvertedAnsiString);
			sqlite3_exec(sqlitedatabase, _strdup(strStd.c_str()), NULL, NULL, &zErrMsg);
		}
	}
}
void cExecuteSqlite::RecreateUpdateTriggers(CString &RelationFirstTable, CString &RelationSecondTable, CString &RelationFirstField, CString &RelationSecondField, std::vector<std::pair<CString,CString>> &updatefirstrelation, 
		                        std::vector<std::pair<CString,CString>> &updatesecondrelation ,sqlite3 *&sqlitedatabase)
{
	char *zErrMsg;
	unsigned nUpdateCount = updatefirstrelation.size();
	for( unsigned i1 = 0; i1< nUpdateCount; ++i1 )
	{
		if( RelationFirstTable == updatefirstrelation[i1].first )
		{
			CString RecreateTrigger = _T("CREATE TRIGGER ") +  updatefirstrelation[i1].first + updatefirstrelation[i1].second + 
							            updatesecondrelation[i1].first + updatesecondrelation[i1].second + _T("Update AFTER UPDATE OF ") + RelationFirstField + _T(" ON ") + RelationFirstTable + _T(" BEGIN UPDATE ")
										+ RelationSecondTable + _T(" SET ") + RelationSecondField + _T(" = new.") + RelationFirstField + _T(" WHERE ") + RelationSecondField + _T(" = OLD.") + RelationFirstField
										 + _T("; END");
			CT2CA pszConvertedAnsiString (RecreateTrigger);
			std::string strStd (pszConvertedAnsiString);
			sqlite3_exec(sqlitedatabase, _strdup(strStd.c_str()), NULL, NULL, &zErrMsg);
		}
		if( RelationFirstTable == updatesecondrelation[i1].first )
		{
			CString RecreateTrigger = _T("CREATE TRIGGER ") +  updatesecondrelation[i1].first + updatesecondrelation[i1].second + 
							            updatefirstrelation[i1].first + updatefirstrelation[i1].second + _T("Update AFTER UPDATE OF ") + RelationSecondField + _T(" ON ") + RelationSecondTable + _T(" BEGIN UPDATE ")
										+ RelationFirstTable + _T(" SET ") + RelationFirstField + _T(" = new.") + RelationSecondField + _T(" WHERE ") + RelationFirstField + _T(" = OLD.") + RelationSecondField
										 + _T("; END");
			CT2CA pszConvertedAnsiString (RecreateTrigger);
			std::string strStd (pszConvertedAnsiString);
			sqlite3_exec(sqlitedatabase, _strdup(strStd.c_str()), NULL, NULL, &zErrMsg);
		}
	}
}
void cExecuteSqlite::SaveCurrentDrawing(std::vector<wxString> &createdtables, std::vector<int> x_coordinate, std::vector<int> y_coordinate,
	                                    const char *dPath)
{
	char *zErrMsg;
	sqlite3 *sqlitedatabase;
	int rc;
	rc = sqlite3_open(dPath, &sqlitedatabase);
    if( rc )
	  {
         exit(0);
      }
	wxString Statement;
	unsigned nTableCount = createdtables.size();
	sqlite3_exec(sqlitedatabase, "DROP TABLE SavedRelationships", NULL, NULL, &zErrMsg);
	Statement = wxT("CREATE TABLE `SavedRelationships` ( `Table` TEXT, `x` INTEGER, `y` INTEGER)");
	const char *statementforexecution = Statement.mb_str();
	sqlite3_exec(sqlitedatabase, "PRAGMA synchronous = OFF", NULL, NULL, &zErrMsg);
	sqlite3_exec(sqlitedatabase, "PRAGMA journal_mode = MEMORY", NULL, NULL, &zErrMsg);
	sqlite3_exec(sqlitedatabase, statementforexecution, NULL, NULL, &zErrMsg);
	for( unsigned i = 0; i < nTableCount; ++i )
	{
		Statement = wxT("INSERT INTO SavedRelationships VALUES ('") + createdtables[i] + wxT("' ,") + wxString::Format(wxT("%i"), x_coordinate[i]) + wxT(",");
		Statement << y_coordinate[i]; 
		Statement += wxT(")");
		statementforexecution = Statement.mb_str();
		sqlite3_exec(sqlitedatabase, statementforexecution, NULL, NULL, &zErrMsg);
	}
	sqlite3_close(sqlitedatabase);
}