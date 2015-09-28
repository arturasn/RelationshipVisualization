#pragma once
#ifndef __READFROMSQLITE_H__ 
#define __READFROMSQLITE_H__

#include <vector>

class wxString;

class ReadSqlite
{
public:
	static void ReadSqliteStatetements(std::vector<std::pair<CString, CString>> &DeleteFirstRelation, std::vector<std::pair<CString, CString>> &DeleteSecondRelation, 
		                               std::vector<std::pair<CString, CString>> &UpdateFirstRelation, std::vector<std::pair<CString, CString>> &UpdateSecondRelation, 
									   std::vector<CString> &ForeignKeyFirstTable, std::vector<CString> &ForeignKeySecondTable,
									   std::vector<CString> &ForeignKeyFirstField, std::vector<CString> &ForeignKeySecondField, std::vector< std::vector <CString> > &FieldNames,
									   std::vector<CString> &TableNames, std::vector<CString> &PrimaryKeyFields, std::vector<CString> &deletetriggernames, std::vector<CString> &updatetriggernames, 
									   std::vector<CString> &TableNamesSQL, std::vector<CString> &uniquefields, std::vector<wxString> &createdtable, std::vector<int> &x, 
									   std::vector<int> &y, std::vector< std::vector<wxString> > &createdfields, const char *Path);
private:
	static void GetRepaintValues(std::vector<wxString> &createdtable, std::vector<int> &x, std::vector<int> &y, std::vector< std::vector<wxString> > &createdfields, 
								 std::vector<CString> &TableNames, std::vector< std::vector <CString> > &FieldNames,
								 const char *Path);
};

#endif