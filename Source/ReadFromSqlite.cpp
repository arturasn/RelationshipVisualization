#define _AFXDLL
#include "stdafx.h"
#include "sqlite3.h"
#include <wx/string.h>
#include "ReadFromSqlite.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

#pragma comment (lib, "sqlite3.lib")

void ReadSqlite::ReadSqliteStatetements(std::vector<std::pair<CString, CString>> &DeleteFirstRelation, std::vector<std::pair<CString, CString>> &DeleteSecondRelation, 
		                               std::vector<std::pair<CString, CString>> &UpdateFirstRelation, std::vector<std::pair<CString, CString>> &UpdateSecondRelation, 
									   std::vector<CString> &ForeignKeyFirstTable, std::vector<CString> &ForeignKeySecondTable,
									   std::vector<CString> &ForeignKeyFirstField, std::vector<CString> &ForeignKeySecondField, std::vector< std::vector <CString> > &FieldNames,
									   std::vector<CString> &TableNames, std::vector<CString> &PrimaryKeyFields, std::vector<CString> &deletetriggernames, std::vector<CString> &updatetriggernames,
									   std::vector<CString> &TableNamesSQL, std::vector<CString> &uniquefields, std::vector<wxString> &createdtable, std::vector<int> &x, 
									   std::vector<int> &y, std::vector< std::vector<wxString> > &createdfields, const char *Path)
{
	sqlite3 *sqlitedatabase;
	int rc = sqlite3_open(Path, &sqlitedatabase);
	sqlite3_stmt * stmt;
	sqlite3_stmt * stmt2;
	int s, s2;
	int index;
	bool DoRecreatePainting = false;
	CString sDeleteSecondRelation;
	unsigned nLength;
	char *sql2;
	char *sql = "select * from sqlite_master where type = 'trigger';";
	rc = sqlite3_prepare_v2(sqlitedatabase, sql, -1, &stmt, 0);
	while (1) 
	{
        s = sqlite3_step (stmt);
        if (s == SQLITE_ROW) 
		{
            const unsigned char * text;
            text  = sqlite3_column_text (stmt, 1);
			CString Temp(text);
			Temp = Temp.Right(6);
			if( !(Temp.CompareNoCase(_T("Delete"))) )
			{
				text = sqlite3_column_text(stmt, 2);
				CString Temp2(text);
				text = sqlite3_column_text(stmt, 1);
				CString Temp5(text);
				deletetriggernames.push_back(Temp5);
				text = sqlite3_column_text(stmt, 4);
				CString Temp3(text);
				index = Temp3.Find(_T("FROM"));
				Temp3 = Temp3.Right(Temp3.GetLength() - index - 5);
				index = Temp3.Find(_T(" WHERE "));
				Temp3 = Temp3.Left(index);
				CString Temp4(text);
				index = Temp4.Find(_T("WHERE "));
				Temp4 = Temp4.Right(Temp4.GetLength() - index - 6);
				index = Temp4.Find(_T(" "));
				sDeleteSecondRelation = Temp4.Left(index);
				nLength = Temp2.GetLength();
				Temp4 = Temp4.Right(Temp4.GetLength() - index - 7);
				index = Temp4.Find(_T(";"));
				DeleteSecondRelation.push_back(std::make_pair(Temp3, sDeleteSecondRelation));
				DeleteFirstRelation.push_back(std::make_pair(Temp2, Temp4.Left(index)));
			}
			else
			{
				text = sqlite3_column_text(stmt, 2);
				CString Temp2(text);
				text = sqlite3_column_text(stmt, 1);
				CString Temp6(text);
				updatetriggernames.push_back(Temp6);
				text = sqlite3_column_text(stmt, 4);
				CString Temp4(text);
				CString Temp3(text);
				index = Temp4.Find(_T("OF "));
				Temp4 = Temp4.Right(Temp4.GetLength() - index);
				index = Temp4.Find(_T(" ON"));
				Temp4 = Temp4.Left(index);
				UpdateFirstRelation.push_back(std::make_pair(Temp2,Temp4.Right(Temp4.GetLength() - 3)));
				CString Temp5(text);
				index = Temp5.Find(_T("SET "));
				Temp5 = Temp5.Right(Temp5.GetLength() - index - 4);
				index = Temp5.Find(_T(" ="));
				text = sqlite3_column_text(stmt, 1);
				nLength = Temp2.GetLength();
				Temp5 = Temp5.Left(index);
				index = Temp3.Find(_T("BEGIN UPDATE "));
				Temp3 = Temp3.Right(Temp3.GetLength() - index - 13);
				index = Temp3.Find(_T(" SET"));
				Temp3 = Temp3.Left(index);
				UpdateSecondRelation.push_back(std::make_pair(Temp3, Temp5));
			}
        }
        else if (s == SQLITE_DONE) 
				 break;
        else exit(1);
  
    }
	sqlite3_finalize(stmt);
	sql = "SELECT * FROM sqlite_master WHERE type = 'table' ORDER BY name ASC;";
	rc = sqlite3_prepare_v2(sqlitedatabase, sql, -1, &stmt, 0);
	while(1)
	{
	   s = sqlite3_step (stmt);
	   if (s == SQLITE_ROW) 
		{
			const unsigned char * text;
			text  = sqlite3_column_text (stmt, 1);
			CString Temp(text);
			if( Temp.Compare(_T("sqlite_sequence") ) && Temp.Compare(_T("SavedRelationships") ) )
			{
			  TableNames.push_back(Temp);
			  text = sqlite3_column_text (stmt, 4);
			  CString Temp2(text);
			  TableNamesSQL.push_back(Temp2);
			}
			if( !(Temp.Compare(_T("SavedRelationships"))) )
			  {
				  DoRecreatePainting = true;
			  }

	    }
	   else if(s == SQLITE_DONE)
	   {
		   break;
	   }
	   else exit(1);

	}
	sqlite3_finalize(stmt);
	auto end_it = TableNames.end();
	for(auto it = TableNames.begin(); it != end_it; ++it)
	{

		CString Temp = _T("PRAGMA foreign_key_list(");
		Temp += *it;
		Temp += _T(")");
		CT2CA pszConvertedAnsiString (Temp);
	    std::string strStd (pszConvertedAnsiString);
		sql = _strdup(strStd.c_str() );
		sqlite3_prepare_v2(sqlitedatabase, sql, -1, &stmt, 0);
		while(1)
	      {
	        s = sqlite3_step(stmt);
			if (s == SQLITE_ROW) 
				{
					const unsigned char *text;
					text  = sqlite3_column_text (stmt, 2);
					CString Temp(text);
					ForeignKeyFirstTable.push_back(*it);
					ForeignKeySecondTable.push_back(Temp);
					text = sqlite3_column_text (stmt, 3);
					CString Temp2(text);
					ForeignKeyFirstField.push_back(Temp2);
					text = sqlite3_column_text(stmt, 4);
					CString Temp3(text);
					ForeignKeySecondField.push_back(Temp3);
			    }
			else if(s == SQLITE_DONE)
				{
				   break;
				}
			else exit(1);

		  }
	  sqlite3_finalize(stmt);
	}
	std::vector<CString> Tables;
	index = 0;
	for(auto it = TableNames.begin(); it != end_it; ++it)
	{
		CString Temp = _T("PRAGMA table_info(");
		Temp += *it;
		Temp += _T(")");
		CT2CA pszConvertedAnsiString (Temp);
	    std::string strStd (pszConvertedAnsiString);
		sql = _strdup(strStd.c_str() );
		FieldNames.push_back(Tables);
		sqlite3_prepare_v2(sqlitedatabase, sql, -1, &stmt, 0);
		while(1)
	      {
	        s = sqlite3_step(stmt);
			if (s == SQLITE_ROW) 
				{
					const unsigned char *text;
					text  = sqlite3_column_text (stmt, 1);
					CString Temp(text);
					FieldNames[index].push_back(Temp);
					text = sqlite3_column_text (stmt, 5);
					CString Temp2(text);
					if(Temp2 == _T("1"))
					{
						Temp2 = *it + FieldNames[index].back();
						PrimaryKeyFields.push_back(Temp2);
					}
					
			    }
			else if(s == SQLITE_DONE)
				{
				   break;
				}
			else exit(1);
			
		  }
	   ++index;
	   sqlite3_finalize(stmt);
	}

	for(auto it = TableNames.begin(); it != end_it; ++it)
	{
		CString Temp = _T("PRAGMA INDEX_LIST(");
		Temp += *it;
		Temp += _T(")");
		CT2CA pszConvertedAnsiString (Temp);
	    std::string strStd (pszConvertedAnsiString);
		sql = _strdup(strStd.c_str() );
		sqlite3_prepare_v2(sqlitedatabase, sql, -1, &stmt, 0);
		while(1)
	    {
			s = sqlite3_step(stmt);
			if (s == SQLITE_ROW) 
			{
				const unsigned char *text;
				text  = sqlite3_column_text (stmt, 2);
				CString Temp2(text);
				if( Temp2 == _T("1"))
				{
					text = sqlite3_column_text(stmt, 1);
					CString Temp3(text);
					CString Temp4 =  _T("PRAGMA INDEX_INFO(");
					Temp4 += Temp3;
					Temp4 += _T(")");
					CT2CA pszConvertedAnsiString2 (Temp4);
					std::string strStd2 (pszConvertedAnsiString2);
					sql2 = _strdup(strStd2.c_str() );
					sqlite3_prepare_v2(sqlitedatabase, sql2, -1, &stmt2, 0);
					while(1)
					{
						s2 = sqlite3_step(stmt2);
						if( s2 == SQLITE_ROW )
						{
							const unsigned char *text2;
							text2 = sqlite3_column_text(stmt2, 2);
							CString Temp4(text2);
							uniquefields.push_back(*it + Temp4);
						}
						else if( s2 == SQLITE_DONE )
						{
							break;
						}
						else exit(1);
					}
					sqlite3_finalize(stmt2);
				}
			}
			else if(s == SQLITE_DONE)
			{
				 break;
			}
			else exit(1);
		}
		sqlite3_finalize(stmt);
	}
	if( DoRecreatePainting )
		GetRepaintValues(createdtable, x, y, createdfields, TableNames, FieldNames, Path);
	sqlite3_close(sqlitedatabase);
}
void ReadSqlite::GetRepaintValues(std::vector<wxString> &createdtable, std::vector<int> &x, std::vector<int> &y, std::vector< std::vector<wxString> > &createdfields, 
								  std::vector<CString> &TableNames, std::vector< std::vector <CString> > &FieldNames,
								 const char *Path)
{
	sqlite3 *sqlitedatabase;
	int s;
	int rc = sqlite3_open(Path, &sqlitedatabase);
	sqlite3_stmt * stmt;
	char *sql = "SELECT * FROM SavedRelationships";
	sqlite3_prepare_v2(sqlitedatabase, sql, -1, &stmt, 0);
	while(1)
	{
		s = sqlite3_step(stmt);
		if (s == SQLITE_ROW) 
		{
			const unsigned char *text;
			text = sqlite3_column_text(stmt, 0);
			wxString Temp(text);
			createdtable.push_back(Temp);
			text = sqlite3_column_text(stmt, 1);
			wxString Temp2(text);
			x.push_back(wxAtoi(Temp2));
			text = sqlite3_column_text(stmt, 2);
			wxString Temp3(text);
			y.push_back(wxAtoi(Temp3));
		}
		else if(s == SQLITE_DONE)
		{
			break;
		}
		else exit(1);
	}
	sqlite3_finalize(stmt);
	unsigned nCreatedTableCount = createdtable.size();
	unsigned nTableCount = TableNames.size();
	unsigned nFieldCount;
	std::vector<wxString> Temporary;
	wxString Temp;
	for( unsigned  i = 0; i < nCreatedTableCount; ++i )
	{
		for( unsigned i1 = 0; i1 < nTableCount; ++i1 )
		{
			CT2CA pszConvertedAnsiString (TableNames[i1]);
			std::string strStd (pszConvertedAnsiString);
			Temp = wxString::FromUTF8(_strdup(strStd.c_str() ) );
			if( createdtable[i] == Temp )
			{
				createdfields.push_back(Temporary);
				nFieldCount = FieldNames[i1].size();
				for( unsigned i2 = 0; i2 < nFieldCount; ++i2 )
				{
					CT2CA pszConvertedAnsiString (FieldNames[i1][i2]);
					std::string strStd (pszConvertedAnsiString);
					Temp = wxString::FromUTF8(_strdup(strStd.c_str() ) );
					createdfields[i].push_back(Temp);
				}
			}
		}
	}
	sqlite3_close(sqlitedatabase);
}
