#ifndef __RELATIONSHIPVISUALIZATION_H__ 
#define __RELATIONSHIPVISUALIZATION_H__ 
#pragma once

class CustomDialog : public wxFrame
{
public:
	 std::string m_dPath;
	 RelationshipsInfo Get;
	 CustomDialog(const wxString& title);
	 wxListBox *m_pTables;
	 wxListBox *m_pCreatedtableslist;
	 wxString *m_pChoices;
	 void SaveTables(wxCommandEvent &WXUNUSED(event) );
	 void OnEraseBackGround(wxEraseEvent &WXUNUSED(event));
	 void PaintBackground(wxDC &dc);
	 int GetRectangleWidth(unsigned &nSize, const int &CurMax, const int &selection);
private:
	 void OnShowTables( wxCommandEvent &WXUNUSED(event) );
	 void OnOpenFile( wxCommandEvent &WXUNUSED(event) );
	 void OnAddTable( wxCommandEvent &WXUNUSED(event) );
	 void OnRemoveTable( wxCommandEvent &WXUNUSED(event) );
	 void OnDeleteTable( wxCommandEvent &WXUNUSED(event) );
	 void OnClear (wxCommandEvent &WXUNUSED(event) );
	 void OnLeftMouseDown( wxMouseEvent &event );
	 void OnLeftMouseReleased(wxMouseEvent &event);
	 void OnMouseMoved(wxMouseEvent& event);
	 void OnMouseLeftWindow(wxMouseEvent &WXUNUSED( event));
	 void OnPaint(wxPaintEvent &event);
	 void OnShowAllRelations(wxCommandEvent &WXUNUSED(event) );
	 void OnShowRelations(wxCommandEvent &WXUNUSED (event) ); 
	 void OnLeftDoubleClick(wxMouseEvent & event);
	 void GetRelationLines(std::vector<std::pair<int, int>> &Drawnline, unsigned &nSize);
	 
	 void MakeTableSelected();
	 void EditRelationShips(wxString &FirstRelationTable, wxString &FirstRelationField, wxString &SecondRelationTable, wxString &SecondRelationField, const bool &bUpdateCascade,
	                        const bool &bDeleteCascade, const bool &bReferentialIntegrity);
	 std::pair<int,int> GetFieldIndex(int &FirstTableIndex, int &SecondTableIndex, int &RelationshipIndex, int &RelationFlag);
	 wxString CstringToWxString(CString &String4Conversion);
	 void GetOneAdditionFields();
	 int GetIndex(CString sSearch);
	 bool isCreated(CString &sSearch);
	 wxString RelationshipView(int &FirstTableIndex, int &SecondTableIndex, int &RelationshipIndex, int &RelationFlag);
	 double Triangle(double a, double b, double c);
	 wxDECLARE_EVENT_TABLE();
};


class MyApp : public wxApp
{
 public:
    virtual bool OnInit();
};
enum
{
	Show_Table = wxID_HIGHEST + 1,
	Clear_Layout,
	Show_Relationships,
	Show_All_Relationships,
	open_file,
	add_table,
	save_tables,
	delete_table,
	remove_table
};

DECLARE_APP(MyApp)
 
#endif