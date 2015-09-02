#pragma once
#ifndef __RELATIONSHIPSINFO_H__ 
#define __RELATIONSHIPSINFO_H__ 

class wxString;

#include <vector>
#include <afx.h>

class RelationshipsInfo 
{
public:
	friend class CustomDialog;
	bool m_dragging;
	bool m_field_dragging;
	bool m_SomethingSelected;
	wxString m_SelectedFirstTable;
	wxString m_SelectedFirstField;
	int m_rect_index;
	std::vector<std::pair<CString, CString>> m_deletefirstrelation;
	std::vector<std::pair<CString, CString>> m_deletesecondrelation;
	std::vector<std::pair<CString, CString>> m_updatefirstrelation;
	std::vector<std::pair<CString, CString>> m_updatesecondrelation;
	std::vector<CString> m_foreignkeyfirsttable;
	std::vector<CString> m_tablenamesSQL;
	std::vector<CString> m_deletetriggernames;
	std::vector<CString> m_updatetriggernames;
	std::vector<CString> m_foreignkeysecondtable;
	std::vector<CString> m_foreignkeyfirstfield;
	std::vector<CString> m_foreignkeysecondfield;
	std::vector<CString> m_tablenames;
	std::vector<CString> m_primarykeyfields;
	std::vector<CString> m_uniquefields;
	std::vector< std::vector <CString> > m_fieldnames;
	std::vector<wxString> m_createdtable;
	std::vector< std::vector<wxString> > m_createdfields;
	std::vector<std::vector<bool>> m_oneaddition;
	std::vector< std::vector<int> > m_createdfieldcoordy;
	std::vector<int> m_x;
	std::vector<int> m_y;
	std::vector<int> m_previous_mouse_x;
	std::vector<int> m_previous_mouse_y;
	std::vector<int> m_width;
	std::vector<int> m_height;
	std::vector<std::pair <int, int> > m_linetopleft;
	std::vector<std::pair <int, int> > m_linetopright;
	std::vector<std::pair <int, int> > m_linebottomleft;
	std::vector<std::pair <int, int> > m_linebottomright;
	std::vector<std::pair <int, int> > m_boundaringlineindexes;
	std::vector<std::pair<int,int> > m_RelationIndex;	
};
#endif