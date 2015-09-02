#pragma once
#ifndef __EXECUTESQLITE_H__ 
#define __EXECUTESQLITE_H__

#include <vector>

struct sqlite3;
class wxString;

class cExecuteSqlite
{
public:
	static void ExecuteSqliteStatements(CString &FirstRelationTable, CString &SecondRelationTable, CString &FirstRelationField, CString &SecondRelationField, const char *dPath, 
		                                std::vector<std::pair<CString,CString>> &deletefirstrelation, std::vector<std::pair<CString,CString>> &deletesecondrelation, std::vector<std::pair<CString,CString>> &updatefirstrelation,
										std::vector<std::pair<CString,CString>> &updatesecondrelation, std::vector<CString> &deletetriggernames, std::vector<CString> &updatetriggernames, 
										std::vector<CString> &foreignkeyfirsttable, std::vector<CString> &foreignkeysecondtable, std::vector<CString> &foreignkeyfirstfield, 
										std::vector<CString> &foreignkeysecondfield, std::vector<CString> &tablenamesSQL, std::vector<CString> &tablenames, int RelationFlag);
	static void SaveCurrentDrawing(std::vector<wxString> &createdtables, std::vector<int> &height, std::vector<int> &width, std::vector<int> x_coordinate, std::vector<int> y_coordinate,
								   const char *dPath);
private:
	static void RecreateDeleteTriggers(CString &FirstRelationTable, CString &SecondRelationTable, CString &FirstRelationField, CString &SecondRelationField, std::vector<std::pair<CString,CString>> &deletefirstrelation, 
		                               std::vector<std::pair<CString,CString>> &deletesecondrelation, sqlite3 *&sqlitedatabase);
	static void RecreateUpdateTriggers(CString &RelationFirstTable, CString &RelationSecondTable, CString &RelationFirstField, CString &RelationSecondField, std::vector<std::pair<CString,CString>> &updatefirstrelation, 
		                        std::vector<std::pair<CString,CString>> &updatesecondrelation ,sqlite3 *&sqlitedatabase);
};
#endif