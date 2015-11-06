#define _AFXDLL
#include "stdafx.h"
#include <wx/wx.h>
#include <wx/dcbuffer.h>
#include "RelationshipInfo.h"
#include "ExecuteSqlite.h"
#include <fstream>
#include "SimpleIni.h"
#include "RelationShipVisualization.h"
#include "ReadFromSqlite.h"

IMPLEMENT_APP(MyApp)
	wxBEGIN_EVENT_TABLE ( CustomDialog, wxFrame )
		EVT_TOOL(Show_Table, CustomDialog::OnShowTables ) 
		EVT_TOOL(open_file, CustomDialog::OnOpenFile )
		EVT_TOOL(Clear_Layout, CustomDialog::OnClear )
		EVT_LEFT_DOWN(CustomDialog::OnLeftMouseDown)
		EVT_LEFT_UP(CustomDialog::OnLeftMouseReleased)
		EVT_MOTION(CustomDialog::OnMouseMoved)
		EVT_LEAVE_WINDOW(CustomDialog::OnMouseLeftWindow)
		//EVT_TOOL(Show_Relationships, CustomDialog::OnShowRelations)
		//EVT_TOOL(Show_All_Relationships, CustomDialog::OnShowAllRelations)
		EVT_TOOL(save_tables, CustomDialog::SaveTables)
		EVT_TOOL(delete_table, CustomDialog::OnDeleteTable)
		EVT_LEFT_DCLICK(CustomDialog::OnLeftDoubleClick)
		EVT_PAINT(CustomDialog::OnPaint)
	wxEND_EVENT_TABLE() 

bool MyApp::OnInit()
{
	CustomDialog *custom = new CustomDialog(wxT("Relationship Visualization"));
    custom->Show(true);
    return true;
}
 
CustomDialog::CustomDialog(const wxString &title)
       : wxFrame((wxFrame *)NULL, -1, title, wxDefaultPosition, wxSize(wxSystemSettings::GetMetric(wxSYS_SCREEN_X), wxSystemSettings::GetMetric(wxSYS_SCREEN_Y)), 
	   wxDEFAULT_FRAME_STYLE | wxFULL_REPAINT_ON_RESIZE)
{
	Get.m_dragging = false;
	Get.m_field_dragging = false;
	Get.m_SomethingSelected = false;
	wxImage::AddHandler( new wxPNGHandler );
	wxBitmap show_table_img(wxT("show_table.png"), wxBITMAP_TYPE_PNG);
	wxBitmap show_clear_img(wxT("clear.png"), wxBITMAP_TYPE_PNG);
	//wxBitmap show_relationships_img(wxT("relationships.png"), wxBITMAP_TYPE_PNG);
	//wxBitmap show_all_relationships_img(wxT("allrelationships.png"), wxBITMAP_TYPE_PNG);
	wxBitmap open_img(wxT("openicon.png"), wxBITMAP_TYPE_PNG);
	wxBitmap save_img(wxT("save.png"), wxBITMAP_TYPE_PNG);
	wxBitmap delete_img(wxT("remove.png"), wxBITMAP_TYPE_PNG);

	wxToolBar *MainToolBar = new wxToolBar(this, wxID_ANY);
    wxSizer *MainSizer = new wxBoxSizer(wxVERTICAL);

	MainToolBar->AddTool(open_file, open_img, wxT("Select File"));
	MainToolBar->AddTool(Show_Table, show_table_img, wxT("Show Table"));
	MainToolBar->AddTool(delete_table, delete_img, wxT("Remove Table"));
	MainToolBar->AddTool(Clear_Layout, show_clear_img, wxT("Clear Layout"));
	//MainToolBar->AddTool(Show_Relationships, show_relationships_img, wxT("Show Direct Relationships"));
	//MainToolBar->AddTool(Show_All_Relationships, show_all_relationships_img, wxT("Show All Relationships"));
	MainToolBar->AddTool(save_tables, save_img, wxT("Save current painted canvas"));
	
    MainToolBar->Realize();
	
	MainSizer->Add(MainToolBar, 0, wxEXPAND);
	
	SetSizer(MainSizer);

	Show(true);
}
void CustomDialog::OnShowTables( wxCommandEvent &WXUNUSED(event)) 
{
	int *sizex = new int, 
		*sizey = new int,
		*posx = new int,
		*posy = new int;
	bool isChecked = false;
	GetWindowInformationAddTable(*sizex,*sizey, *posx, *posy, isChecked);
	wxDialog *dlg = new wxDialog(NULL, wxID_ANY, wxT("Show Tables"), wxPoint(*posx,*posy), wxSize(*sizex,*sizey), wxCAPTION|wxCLOSE_BOX|wxRESIZE_BORDER);

	wxBoxSizer *sShowTables = new wxBoxSizer(wxVERTICAL);

	int nTableCount = Get.m_tablenames.size();
	int ind = -1;
	m_pChoices.Clear();
	for( int i = 0; i < nTableCount; ++i )
	{
		if( GetCreatedTableIndex(CstringToWxString(Get.m_tablenames[i])) != -1){	
			continue;
		}
		ind++;
		CT2CA pszConvertedAnsiString (Get.m_tablenames[i]);
	    std::string strStd (pszConvertedAnsiString);
		m_pChoices.Insert(wxString::FromUTF8(_strdup(strStd.c_str() ) ), ind); 
	}
	m_pTables = new wxListBox(dlg, wxID_ANY, wxDefaultPosition, wxDefaultSize, m_pChoices);
	
	m_pAddbutton = new wxButton(dlg, add_table, "Add", wxDefaultPosition, wxSize(60,25));
	wxButton *closebutton = new wxButton(dlg, wxID_CANCEL, "Close", wxDefaultPosition, wxSize(60,25));
	m_pCheckbox = new wxCheckBox(dlg, add_checkbox, wxT("Exclude created tables"));
	m_pCheckbox->SetValue(isChecked);
	if ( !isChecked )
		UpdateTable();
	
	wxBoxSizer *horizontalSizer = new wxBoxSizer(wxHORIZONTAL);
	wxBoxSizer *verticalSizer = new wxBoxSizer(wxVERTICAL);

	verticalSizer->Add(m_pAddbutton, 0, wxTOP, 5);
	verticalSizer->Add(closebutton, 0, wxTOP, 5);

	horizontalSizer->Add(m_pTables, 1, wxEXPAND | wxALL, 5);
	horizontalSizer->Add(verticalSizer, 0, wxALIGN_RIGHT | wxRIGHT , 5);

	sShowTables->Add(horizontalSizer, 1, wxLEFT | wxTOP | wxEXPAND, 5);
	sShowTables->Add(m_pCheckbox, 0, wxLEFT|wxTOP|wxBOTTOM, 5);
	dlg->SetSizer(sShowTables);

	m_pAddbutton->Connect(add_table, wxEVT_BUTTON, wxCommandEventHandler(CustomDialog::OnAddTable), NULL, this);
	m_pCheckbox->Connect(add_checkbox, wxEVT_CHECKBOX, wxCommandEventHandler(CustomDialog::OnExcludeCb), NULL, this);

	if( dlg->ShowModal() == wxID_CANCEL)
	{
		isChecked = m_pCheckbox->IsChecked();
		dlg->GetSize(sizex,sizey);
		dlg->GetPosition(posx,posy);
		SaveWindowInformationAddTable(*sizex,*sizey, *posx, *posy, isChecked);
	}
	dlg->Destroy();
	delete sizex;
	delete sizey;
	delete posx;
	delete posy;
	/*delete addbutton;
	delete closebutton;
	delete m_pTables;*/
}
void CustomDialog::OnOpenFile( wxCommandEvent &WXUNUSED(event) )
{
	wxFileDialog dialog(this, wxT("Visualization file selection"), wxEmptyString, wxEmptyString, wxT("sqlite files (*.sqlite)|*.sqlite"));
	dialog.CentreOnParent();
	dialog.SetDirectory(wxGetHomeDir());
	wxString TFilePathLine;
	if ( dialog.ShowModal() == wxID_OK )
	{
		TFilePathLine = dialog.GetPath();
	    const char *Path = TFilePathLine.mb_str();
		m_dPath = std::string(Path);
		Get.m_deletefirstrelation.clear();
		Get.m_deletesecondrelation.clear();
		Get.m_deletetriggernames.clear();
		Get.m_updatetriggernames.clear();
	    Get.m_updatefirstrelation.clear();
		Get.m_updatesecondrelation.clear();
		Get.m_foreignkeyfirsttable.clear();
		Get.m_foreignkeysecondtable.clear();
		Get.m_foreignkeyfirstfield.clear();
		Get.m_foreignkeysecondfield.clear();
		Get.m_fieldnames.clear();
		Get.m_tablenames.clear();
		Get.m_tablenamesSQL.clear();
		Get.m_primarykeyfields.clear();
		Get.m_oneaddition.clear();
		Get.m_uniquefields.clear();

		Get.m_x.clear();
	    Get.m_createdtable.clear();
		Get.m_y.clear();
		Get.m_width.clear();
		Get.m_height.clear();
		Get.m_createdfields.clear();
		//Get.m_Drawnlines.clear();

	    ReadSqlite::ReadSqliteStatetements(Get.m_deletefirstrelation, Get.m_deletesecondrelation, Get.m_updatefirstrelation, Get.m_updatesecondrelation, 
										   Get.m_foreignkeyfirsttable, Get.m_foreignkeysecondtable, Get.m_foreignkeyfirstfield, Get.m_foreignkeysecondfield, 
										   Get.m_fieldnames, Get.m_tablenames, Get.m_primarykeyfields, Get.m_deletetriggernames, Get.m_updatetriggernames, 
										   Get.m_tablenamesSQL, Get.m_uniquefields, Get.m_createdtable, Get.m_x, Get.m_y, Get.m_createdfields,
										   Path);

		unsigned nSize = Get.m_createdtable.size();
		int nFieldSize;
		for( int i = 0; i < nSize; ++i ){
			nFieldSize = Get.m_createdfields[i].size();
			Get.m_width.push_back(8 * GetRectangleWidth(nFieldSize, Get.m_createdtable[i].Length(), i));
			Get.m_height.push_back(30 + 10 * nFieldSize);
		}
		Get.m_previous_mouse_x.resize(nSize);
	    Get.m_previous_mouse_y.resize(nSize);
		GetOneAdditionFields();
		//GetRelationLines();
        Refresh(); 
		Update();

	}
	else
	{
		wxMessageDialog dialog2(this, "File not selected", wxT("File not selected"));
		dialog2.ShowModal();
		dialog2.Destroy();
	}
	dialog.Destroy();
}
void CustomDialog::OnAddTable( wxCommandEvent &WXUNUSED(event) )
{
	
	int selection = GetIndex(m_pTables->GetString(m_pTables->GetSelection()));
	if( selection != wxNOT_FOUND )
	{
		int n = ::rand() % 700 + 20;
		int m = ::rand() % 700 + 20;
		Get.m_x.push_back(n);
		Get.m_y.push_back(m);
		//Get.m_createdtable.push_back(m_pChoices[m_pTables->GetSelection()]);
		Get.m_createdtable.push_back(m_pTables->GetString(m_pTables->GetSelection()));
		unsigned nSize = Get.m_createdtable.size();
		Get.m_previous_mouse_x.resize(nSize);
		Get.m_previous_mouse_y.resize(nSize);
		nSize = Get.m_fieldnames[selection].size();
		std::vector<wxString> temp2;

		int nRectangleWidth = GetRectangleWidth(nSize, Get.m_tablenames[selection].GetLength(), selection);
		Get.m_width.push_back(8 * nRectangleWidth);
		Get.m_height.push_back(30 + 10 * nSize);
		wxString temp;
		Get.m_createdfields.push_back(temp2);
		for(unsigned i = 0; i < nSize; ++i)
		{
			CT2CA pszConvertedAnsiString (Get.m_fieldnames[selection][i]);
	        std::string strStd (pszConvertedAnsiString);
		    temp = wxString::FromUTF8(_strdup(strStd.c_str() ) );
			Get.m_createdfields.back().push_back(temp);
		}
		GetOneAdditionFields();
		//GetRelationLines();
		if( m_pCheckbox->IsChecked() )
		{
		  UpdateAddTableData();
		}
        Refresh(); 
		Update();
	}	
}
void CustomDialog::OnDeleteTable( wxCommandEvent &WXUNUSED(event) )
{
	int *sizex = new int, 
		*sizey = new int,
		*posx = new int,
		*posy = new int;
	GetWindowInformationRemTable(*sizex,*sizey, *posx, *posy);
	wxDialog *dlg = new wxDialog(this, wxID_ANY, wxT("Remove Tables"), wxPoint(*posx,*posy), wxSize(*sizex,*sizey), wxCAPTION|wxCLOSE_BOX|wxRESIZE_BORDER);

	wxBoxSizer *sShowTables = new wxBoxSizer(wxVERTICAL);

	int nTableCount = Get.m_createdtable.size();
	 wxString *createdchoices = new wxString[nTableCount];
	for( int i = 0; i < nTableCount; ++i )
	{
		CT2CA pszConvertedAnsiString (Get.m_createdtable[i]);
	    std::string strStd (pszConvertedAnsiString);
		createdchoices[i] = wxString::FromUTF8(_strdup(strStd.c_str() ) );
	}
	m_pCreatedtableslist = new wxListBox(dlg, wxID_ANY, wxDefaultPosition, wxSize(150,240), nTableCount, createdchoices, wxLB_SORT);
	
	wxButton *removebutton = new wxButton(dlg, remove_table, "Remove", wxDefaultPosition, wxSize(60,25));
	wxButton *closebutton = new wxButton(dlg, wxID_CANCEL, "Close", wxDefaultPosition, wxSize(60,25));

	wxBoxSizer *horizontalSizer = new wxBoxSizer(wxHORIZONTAL);
	wxBoxSizer *verticalSizer = new wxBoxSizer(wxVERTICAL);

	verticalSizer->Add(removebutton, 0, wxLEFT|wxTOP|wxRIGHT|wxEXPAND, 5);
	verticalSizer->Add(closebutton, 0, wxLEFT|wxTOP|wxRIGHT|wxEXPAND, 5);

	horizontalSizer->Add(m_pCreatedtableslist, 1, wxLEFT|wxTOP|wxEXPAND, 5);
	horizontalSizer->Add(verticalSizer, 0);

	sShowTables->Add(horizontalSizer, 1, wxLEFT|wxTOP|wxEXPAND, 5);
	dlg->SetSizer(sShowTables);

	removebutton->Connect(remove_table, wxEVT_BUTTON, wxCommandEventHandler(CustomDialog::OnRemoveTable), NULL, this);

	if ( dlg->ShowModal() == wxID_CANCEL )
	{
		dlg->GetSize(sizex,sizey);
		dlg->GetPosition(posx,posy);
		SaveWindowInformationRemTable(*sizex,*sizey, *posx, *posy);
	}
	dlg->Destroy();
	delete [] createdchoices;
	delete posx;
	delete posy;
	delete sizex;
	delete sizey;
	/*delete removebutton;
	delete closebutton;
	delete m_pCreatedtableslist;*/
}
void CustomDialog::OnRemoveTable( wxCommandEvent &WXUNUSED(event) )
{
	if( m_pCreatedtableslist->GetSelection() != wxNOT_FOUND )
	{
		int selection = GetCreatedTableIndex(m_pCreatedtableslist->GetString(m_pCreatedtableslist->GetSelection()));
		Get.m_createdtable.erase(Get.m_createdtable.begin() + selection );
		Get.m_createdfields.erase(Get.m_createdfields.begin() + selection);
		Get.m_x.erase(Get.m_x.begin() + selection );
		Get.m_y.erase(Get.m_y.begin() + selection );
		Get.m_height.erase(Get.m_height.begin() + selection );
		Get.m_width.erase(Get.m_width.begin() + selection );
		Get.m_previous_mouse_x.erase(Get.m_previous_mouse_x.begin() + selection);
		Get.m_previous_mouse_y.erase(Get.m_previous_mouse_y.begin() + selection);
		m_pCreatedtableslist->Clear();
		int nTableCount = Get.m_createdtable.size();
		for( int i = 0; i < nTableCount; ++i )
		{
			m_pCreatedtableslist->Insert(Get.m_createdtable[i], i);
		}
		GetOneAdditionFields();
		Get.m_SomethingSelected = false;
        Refresh(); 
		Update();
	}
}
int CustomDialog::GetRectangleWidth(unsigned &nSize, const int &CurMax, const int &selection)
{	
	int maximum = CurMax;
	for(unsigned i4 = 0; i4 < nSize; ++i4)
	{
		if(Get.m_fieldnames[selection][i4].GetLength() > maximum )
			maximum = Get.m_fieldnames[selection][i4].GetLength();
	}
	return maximum;
}
int CustomDialog::GetRectangleWidth(int &nSize, const int &CurMax, const int &selection)
{
	int maximum = CurMax;
	for(unsigned i4 = 0; i4 < nSize; ++i4)
	{
		if(Get.m_createdfields[selection][i4].Length() > maximum )
			maximum = Get.m_createdfields[selection][i4].Length();
	}
	return maximum;
}
void CustomDialog::OnClear (wxCommandEvent &WXUNUSED( event)  )
{
	Get.m_SomethingSelected = false;
	Get.m_createdfields.clear();
	Get.m_createdfieldcoordy.clear();
	Get.m_createdtable.clear();
	Get.m_oneaddition.clear();
	Get.m_height.clear();
	Get.m_previous_mouse_x.clear();
	Get.m_previous_mouse_y.clear();
	Get.m_x.clear();
	Get.m_y.clear();
	Get.m_width.clear();
	//Get.m_Drawnlines.clear();
	Refresh();
	Update();
}
void CustomDialog::OnLeftMouseDown( wxMouseEvent &event )
{
	int nCurPosx = event.GetPosition().x;
	int nCurPosy = event.GetPosition().y;
	Get.m_SomethingSelected = false;
	int nSize = Get.m_createdtable.size();
	for(int i = nSize-1; i >= 0; --i)
	{
		if (nCurPosx >= Get.m_x[i] && nCurPosx <= Get.m_x[i] + Get.m_width[i] &&
            nCurPosy >= Get.m_y[i] && nCurPosy <= Get.m_y[i] + 15)
		{
			Get.m_rect_index = i;
			Get.m_SomethingSelected = true;
			break;
		}
	}
	if( Get.m_SomethingSelected )
	{
		 if (nCurPosx >= Get.m_x[Get.m_rect_index] && nCurPosx <= Get.m_x[Get.m_rect_index] + Get.m_width[Get.m_rect_index] &&
			nCurPosy >= Get.m_y[Get.m_rect_index] && nCurPosy <= Get.m_y[Get.m_rect_index] + 15)
			{
				Get.m_dragging = true;
				Get.m_previous_mouse_x[Get.m_rect_index] = event.GetPosition().x;
				Get.m_previous_mouse_y[Get.m_rect_index] = event.GetPosition().y;
			}
	}
	for(int i = nSize-1; i >= 0; --i)
	{
		if (nCurPosx >= Get.m_x[i] && nCurPosx <= Get.m_x[i] + Get.m_width[i] &&
			nCurPosy >= Get.m_y[i] + 30 && nCurPosy <= Get.m_y[i] + Get.m_height[i])
		{
			Get.m_SelectedFirstTable = Get.m_createdtable[i];
			unsigned nFieldSize = Get.m_createdfields[i].size();
			for( unsigned i1 = 0; i1 < nFieldSize ;++i1 )
			{
				if(nCurPosx >= Get.m_x[i] && nCurPosx <= Get.m_x[i] + Get.m_width[i] &&
					nCurPosy >= Get.m_createdfieldcoordy[i][i1] - 7 && nCurPosy <= Get.m_createdfieldcoordy[i][i1] + 6)
				{
					Get.m_SelectedFirstField = Get.m_createdfields[i][i1];
					break;
				}
			}
			if( !Get.m_dragging )
				Get.m_field_dragging = true;
			break;
		}
	}
}
void CustomDialog::OnLeftMouseReleased(wxMouseEvent &event)
{
    Get.m_dragging = false;
	wxString SelectedSecondTable;
	wxString SelectedSecondField;
	SetCursor(wxNullCursor);
	if( Get.m_field_dragging )
	{
		int nCurPosx = event.GetPosition().x;
		int nCurPosy = event.GetPosition().y;
		int nSize = Get.m_createdtable.size();
		for( int i = nSize-1; i >= 0; --i )
		{
			if (nCurPosx >= Get.m_x[i] && nCurPosx <= Get.m_x[i] + Get.m_width[i] &&
			nCurPosy >= Get.m_y[i] + 15 && nCurPosy <= Get.m_y[i] + Get.m_height[i])
			{
				SelectedSecondTable = Get.m_createdtable[i];
				unsigned nFieldSize = Get.m_createdfields[i].size();
				for( unsigned i1 = 0 ; i1 < nFieldSize; ++i1 )
				{
					if( nCurPosx >= Get.m_x[i] && nCurPosx <= Get.m_x[i] + Get.m_width[i] &&
					nCurPosy >= Get.m_createdfieldcoordy[i][i1] - 7 && nCurPosy <= Get.m_createdfieldcoordy[i][i1] + 6 )
					{
						SelectedSecondField = Get.m_createdfields[i][i1];
						EditRelationShips(Get.m_SelectedFirstTable, Get.m_SelectedFirstField, SelectedSecondTable, SelectedSecondField, false, false, false);
						break;
					}
				}
				break;
			}
		}
	}
	Get.m_field_dragging = false;
}
void CustomDialog::OnMouseMoved(wxMouseEvent& event)
{
	int nCurPosx = event.GetPosition().x;
	int nCurPosy = event.GetPosition().y;
    if ( Get.m_dragging && event.Dragging() )
    {
        int delta_x = nCurPosx - Get.m_previous_mouse_x[Get.m_rect_index];
        int delta_y = nCurPosy - Get.m_previous_mouse_y[Get.m_rect_index];
 
        Get.m_x[Get.m_rect_index] += delta_x;
        Get.m_y[Get.m_rect_index] += delta_y;
 
        Get.m_previous_mouse_x[Get.m_rect_index] = event.GetPosition().x;
        Get.m_previous_mouse_y[Get.m_rect_index] = event.GetPosition().y;
        Refresh(); 
		Update();
    }
	if ( Get.m_field_dragging && event.Dragging() )
	{
		SetCursor(wxCURSOR_NO_ENTRY);
		int nSize = Get.m_createdtable.size();
		for( int i = nSize-1; i >= 0; --i )
			{
				if (nCurPosx >= Get.m_x[i] && nCurPosx <= Get.m_x[i] + Get.m_width[i] &&
				nCurPosy >= Get.m_y[i] + 30 && nCurPosy <= Get.m_y[i] + Get.m_height[i])
				{
					SetCursor(wxCURSOR_HAND);
					break;
				}
			}
	}
}
void CustomDialog::OnMouseLeftWindow(wxMouseEvent &WXUNUSED(event))
{
  Get.m_dragging = false;
  Get.m_field_dragging = false;
}

void CustomDialog::OnPaint(wxPaintEvent &WXUNUSED (event))
{ 
	wxBufferedPaintDC dc(this);
	Connect(wxID_ANY, wxEVT_ERASE_BACKGROUND, wxEraseEventHandler(CustomDialog::OnEraseBackGround), NULL, this);
	PrepareDC(dc);
	PaintBackground(dc);
	Get.m_linebottomleft.clear();
	Get.m_linebottomright.clear();
	Get.m_linetopright.clear();
	Get.m_linetopleft.clear();
	Get.m_createdfieldcoordy.clear();
	Get.m_boundaringlineindexes.clear();
	Get.m_RelationIndex.clear();
	wxColour col1;
	col1.Set(wxT("#0c0c0c"));
	wxBrush brush(wxColour(255, 255, 255), wxSOLID);
	dc.SetBrush(brush);
	dc.SetPen(wxPen(col1, 1, wxSOLID));
	
	unsigned nFieldSize;
	int yCoord;
	int maximum = 0;
	wxString temp;

	unsigned nSize = Get.m_createdtable.size();
	std::vector<int> temp2;
	for(unsigned i = 0; i < nSize; ++i)
	{
		dc.DrawRectangle( Get.m_x[i], Get.m_y[i], Get.m_width[i], Get.m_height[i] );
		dc.DrawLine(Get.m_x[i], Get.m_y[i] + 15, Get.m_x[i] + Get.m_width[i], Get.m_y[i] + 15);
		dc.DrawText(Get.m_createdtable[i], Get.m_x[i] + 3, Get.m_y[i]); 
		nFieldSize = Get.m_createdfields[i].size();
		Get.m_createdfieldcoordy.push_back(temp2);
		yCoord = Get.m_y[i] + 15;
		if(nFieldSize > maximum)
			maximum = nFieldSize;
		for(unsigned i1 = 0; i1 < nFieldSize; ++i1)
		{
			CT2CA pszConvertedAnsiString (Get.m_createdfields[i][i1]);
	        std::string strStd (pszConvertedAnsiString);
		    temp = wxString::FromUTF8(_strdup(strStd.c_str() ) );
			dc.DrawText(temp, Get.m_x[i] + 3, yCoord +=10);
			Get.m_createdfieldcoordy.back().push_back(yCoord+8);
		}
	}
	if ( Get.m_SomethingSelected )
		MakeTableSelected();

	std::vector<std::pair<int,int> > Drawnline;
	std::pair<int,int> FieldIndexes;

	GetRelationLines(Drawnline, nSize);

	unsigned nDrawnLineCount = Drawnline.size();
	for( unsigned i = 0; i < nDrawnLineCount; ++i )
	{
		FieldIndexes = GetFieldIndex(Drawnline[i].first, Drawnline[i].second, Get.m_RelationIndex[i].first, Get.m_RelationIndex[i].second);
		if(FieldIndexes.first < 0 || FieldIndexes.first > maximum || FieldIndexes.second < 0 || FieldIndexes.second > maximum)
			continue;
		dc.SetPen( wxPen( wxColor(0,0,0), 3 ) );
		dc.DrawLine(Get.m_x[Drawnline[i].first], Get.m_createdfieldcoordy[Drawnline[i].first][FieldIndexes.first], Get.m_x[Drawnline[i].first] - 15, Get.m_createdfieldcoordy[Drawnline[i].first][FieldIndexes.first]);
		if( Get.m_oneaddition[Drawnline[i].first][FieldIndexes.first])
		{
			dc.DrawText(_T("1"), Get.m_x[Drawnline[i].first] - 7, Get.m_createdfieldcoordy[Drawnline[i].first][FieldIndexes.first] - 15);
		}
		dc.DrawLine(Get.m_x[Drawnline[i].second], Get.m_createdfieldcoordy[Drawnline[i].second][FieldIndexes.second], Get.m_x[Drawnline[i].second] - 15, Get.m_createdfieldcoordy[Drawnline[i].second][FieldIndexes.second]);
		if( Get.m_oneaddition[Drawnline[i].second][FieldIndexes.second] ) 
		{
			dc.DrawText(_T("1"), Get.m_x[Drawnline[i].second] - 7, Get.m_createdfieldcoordy[Drawnline[i].second][FieldIndexes.second] - 15);
		}
		dc.SetPen( wxPen( wxColor(0,0,0), 2 ) );
		dc.DrawLine(Get.m_x[Drawnline[i].first] - 15, Get.m_createdfieldcoordy[Drawnline[i].first][FieldIndexes.first], Get.m_x[Drawnline[i].second] - 15, Get.m_createdfieldcoordy[Drawnline[i].second][FieldIndexes.second]);
		Get.m_linetopleft.push_back(std::make_pair(Get.m_x[Drawnline[i].first] - 18, Get.m_createdfieldcoordy[Drawnline[i].first][FieldIndexes.first]));
		Get.m_linetopright.push_back(std::make_pair(Get.m_x[Drawnline[i].first] - 13, Get.m_createdfieldcoordy[Drawnline[i].first][FieldIndexes.first]));
		Get.m_linebottomleft.push_back(std::make_pair(Get.m_x[Drawnline[i].second] - 18, Get.m_createdfieldcoordy[Drawnline[i].second][FieldIndexes.second]));
		Get.m_linebottomright.push_back(std::make_pair(Get.m_x[Drawnline[i].second] - 13, Get.m_createdfieldcoordy[Drawnline[i].second][FieldIndexes.second]));
		Get.m_boundaringlineindexes.push_back(std::make_pair(Drawnline[i].first, Drawnline[i].second) );
	}
}
void CustomDialog::PaintBackground(wxDC& dc)
{
    wxColour backgroundColour = GetBackgroundColour();
    if (!backgroundColour.Ok())
        backgroundColour =
            wxSystemSettings::GetColour(wxSYS_COLOUR_3DFACE);

    dc.SetBrush(wxBrush(backgroundColour));
    dc.SetPen(wxPen(backgroundColour, 1));

    wxRect windowRect(wxPoint(0, 0), GetClientSize());    
    dc.DrawRectangle(windowRect);
}
std::pair<int,int> CustomDialog::GetFieldIndex(int &FirstTableIndex, int &SecondTableIndex, int &RelationshipIndex, int &RelationFlag)
{
	int FirstField;
	int SecondField;
	unsigned nFirstFieldCount = Get.m_createdfields[FirstTableIndex].size();
	unsigned nSecondFieldCount = Get.m_createdfields[SecondTableIndex].size();
	
	if(RelationFlag & 1)
	{
		wxString DeleteFirstRelationField = CstringToWxString(Get.m_deletefirstrelation[RelationshipIndex].second);
		wxString DeleteSecondRelationField = CstringToWxString(Get.m_deletesecondrelation[RelationshipIndex].second);
		for( unsigned i2 = 0; i2 < nFirstFieldCount; ++i2 )
		{
			if( DeleteFirstRelationField == Get.m_createdfields[FirstTableIndex][i2])
			{
				FirstField = i2;
				break;
			}
		}
		for( unsigned i2 = 0; i2 < nSecondFieldCount; ++i2 )
		{
			if( DeleteSecondRelationField == Get.m_createdfields[SecondTableIndex][i2])
			{
				SecondField = i2;
				break;
			}
		}
		return std::make_pair(FirstField,SecondField);
	}
	else if(RelationFlag & 2)
	{
		wxString UpdateFirstRelationField = CstringToWxString(Get.m_updatefirstrelation[RelationshipIndex].second);
		wxString UpdateSecondRelationField = CstringToWxString(Get.m_updatesecondrelation[RelationshipIndex].second);
		for( unsigned i2 = 0; i2 < nFirstFieldCount; ++i2 )
		{
			if( UpdateFirstRelationField == Get.m_createdfields[FirstTableIndex][i2])
			{
				FirstField = i2;
				break;
			}
		}
		for( unsigned i2 = 0; i2 < nSecondFieldCount; ++i2 )
		{
			if( UpdateSecondRelationField == Get.m_createdfields[SecondTableIndex][i2])
			{
				SecondField = i2;
				break;
			}
		}
		return std::make_pair(FirstField,SecondField);
	}
	else
	{
		wxString ForeignKeyFirstRelationField = CstringToWxString(Get.m_foreignkeyfirstfield[RelationshipIndex]);
		wxString ForeignKeySecondRelationField = CstringToWxString(Get.m_foreignkeysecondfield[RelationshipIndex]);
		for( unsigned i2 = 0; i2 < nFirstFieldCount; ++i2 )
		{
			if( ForeignKeyFirstRelationField == Get.m_createdfields[FirstTableIndex][i2])
			{
				FirstField = i2;
				break;
			}
		}
		for( unsigned i2 = 0; i2 < nSecondFieldCount; ++i2 )
		{
			if( ForeignKeySecondRelationField == Get.m_createdfields[SecondTableIndex][i2])
			{
				SecondField = i2;
				break;
			}
		}
		return std::make_pair(FirstField,SecondField);
	}
}
void CustomDialog::OnEraseBackGround(wxEraseEvent &WXUNUSED(event))
{
}
//void CustomDialog::OnShowRelations(wxCommandEvent &WXUNUSED (event) )
//{	
//	unsigned nSize = Get.m_createdtable.size();
//	unsigned nDeleteRelationCount = Get.m_deletefirstrelation.size();
//	unsigned nUpdateRelationCount = Get.m_updatefirstrelation.size();
//	unsigned nForeignKeyCount = Get.m_foreignkeyfirsttable.size();
//	unsigned nFieldSize;
//	unsigned ind;
//	std::vector <wxString> temp;
//	for(unsigned i = 0; i < nSize; ++i)
//	{
//		for(unsigned i1 = 0; i1 < nDeleteRelationCount; ++i1)
//			{
//				if( !(Get.m_createdtable[i].Cmp(Get.m_deletefirstrelation[i1].first)) )
//				{
//					if ( isCreated(Get.m_deletesecondrelation[i1].first) )
//						continue;
//					Get.m_createdtable.push_back( CstringToWxString(Get.m_deletesecondrelation[i1].first) );
//					Get.m_x.push_back ( ::rand() % 700  + 20);
//					Get.m_y.push_back( ::rand() % 700 + 20);
//					Get.m_createdfields.push_back( temp );
//					ind = GetIndex(Get.m_deletesecondrelation[i1].first);
//					nFieldSize = Get.m_fieldnames[ind].size();
//					for( unsigned i2 = 0; i2 < nFieldSize; ++i2 )
//						Get.m_createdfields.back().push_back( CstringToWxString(Get.m_fieldnames[ind][i2]) );
//					Get.m_width.push_back( 8 * GetRectangleWidth( nFieldSize, Get.m_tablenames[ind].GetLength(), ind ) );
//					Get.m_height.push_back(28 + 10 * nFieldSize);
//
//				}
//
//
//				if( !(Get.m_createdtable[i].Cmp(Get.m_deletesecondrelation[i1].first)) )
//				{
//					if ( isCreated(Get.m_deletefirstrelation[i1].first) )
//						continue;
//					Get.m_createdtable.push_back( CstringToWxString(Get.m_deletefirstrelation[i1].first) );
//					Get.m_x.push_back ( ::rand() % 700  + 20);
//					Get.m_y.push_back( ::rand() % 700 + 20);
//					Get.m_createdfields.push_back( temp );
//					ind = GetIndex(Get.m_deletefirstrelation[i1].first);
//					nFieldSize = Get.m_fieldnames[ind].size();
//					for( unsigned i2 = 0; i2 < nFieldSize; ++i2 )
//						Get.m_createdfields.back().push_back( CstringToWxString(Get.m_fieldnames[ind][i2]) );
//					Get.m_width.push_back( 8 * GetRectangleWidth( nFieldSize, Get.m_tablenames[ind].GetLength(), ind ) );
//					Get.m_height.push_back(28 + 10 * nFieldSize);
//
//				}
//			}
//	}
//
//	for(unsigned i = 0; i < nSize; ++i)
//	{
//		for(unsigned i1 = 0; i1 < nUpdateRelationCount; ++i1)
//			{
//				if( !(Get.m_createdtable[i].Cmp(Get.m_updatefirstrelation[i1].first)) )
//				{
//					if ( isCreated(Get.m_updatesecondrelation[i1].first) )
//						continue;
//					Get.m_createdtable.push_back( CstringToWxString(Get.m_updatesecondrelation[i1].first) );
//					Get.m_x.push_back ( ::rand() % 700 + 20);
//					Get.m_y.push_back( ::rand() % 700 + 20);
//					Get.m_createdfields.push_back( temp );
//					ind = GetIndex(Get.m_updatesecondrelation[i1].first);
//					nFieldSize = Get.m_fieldnames[ind].size();
//					for( unsigned i2 = 0; i2 < nFieldSize; ++i2 )
//						Get.m_createdfields.back().push_back( CstringToWxString(Get.m_fieldnames[ind][i2]) );
//					Get.m_width.push_back( 8 * GetRectangleWidth( nFieldSize, Get.m_tablenames[ind].GetLength(), ind ) );
//					Get.m_height.push_back(28 + 10 * nFieldSize);
//				}
//
//				if( !(Get.m_createdtable[i].Cmp(Get.m_updatesecondrelation[i1].first)) )
//				{
//					if ( isCreated(Get.m_updatefirstrelation[i1].first) )
//						continue;
//					Get.m_createdtable.push_back( CstringToWxString(Get.m_updatefirstrelation[i1].first) );
//					Get.m_x.push_back ( ::rand() % 700 + 20);
//					Get.m_y.push_back( ::rand() % 700 + 20);
//					Get.m_createdfields.push_back( temp );
//					ind = GetIndex(Get.m_updatefirstrelation[i1].first);
//					nFieldSize = Get.m_fieldnames[ind].size();
//					for( unsigned i2 = 0; i2 < nFieldSize; ++i2 )
//						Get.m_createdfields.back().push_back( CstringToWxString(Get.m_fieldnames[ind][i2]) );
//					Get.m_width.push_back( 8 * GetRectangleWidth( nFieldSize, Get.m_tablenames[ind].GetLength(), ind ) );
//					Get.m_height.push_back(28 + 10 * nFieldSize);
//				}
//			}
//	}
//
//	for(unsigned i = 0; i < nSize; ++i)
//	{
//		for(unsigned i1 = 0; i1 < nForeignKeyCount; ++i1)
//			{
//				if( !(Get.m_createdtable[i].Cmp(Get.m_foreignkeyfirsttable[i1])) )
//				{
//					if ( isCreated(Get.m_foreignkeysecondtable[i1]) )
//						continue;
//					Get.m_createdtable.push_back( CstringToWxString(Get.m_foreignkeysecondtable[i1]) );
//					Get.m_x.push_back ( ::rand() % 700 + 20);
//					Get.m_y.push_back( ::rand() % 700 + 20);
//					Get.m_createdfields.push_back( temp );
//					ind = GetIndex(Get.m_foreignkeysecondtable[i1]);
//					nFieldSize = Get.m_fieldnames[ind].size();
//					for( unsigned i2 = 0; i2 < nFieldSize; ++i2 )
//						Get.m_createdfields.back().push_back( CstringToWxString(Get.m_fieldnames[ind][i2]) );
//					Get.m_width.push_back( 8 * GetRectangleWidth( nFieldSize, Get.m_tablenames[ind].GetLength(), ind ) );
//					Get.m_height.push_back(28 + 10 * nFieldSize);
//				}
//
//				if( !(Get.m_createdtable[i].Cmp(Get.m_foreignkeysecondtable[i1])) )
//				{
//					if ( isCreated(Get.m_foreignkeyfirsttable[i1]) )
//						continue;
//					Get.m_createdtable.push_back( CstringToWxString(Get.m_foreignkeyfirsttable[i1]) );
//					Get.m_x.push_back ( ::rand() % 700 + 20);
//					Get.m_y.push_back( ::rand() % 700 + 20);
//					Get.m_createdfields.push_back( temp );
//					ind = GetIndex(Get.m_foreignkeyfirsttable[i1]);
//					nFieldSize = Get.m_fieldnames[ind].size();
//					for( unsigned i2 = 0; i2 < nFieldSize; ++i2 )
//						Get.m_createdfields.back().push_back( CstringToWxString(Get.m_fieldnames[ind][i2]) );
//					Get.m_width.push_back( 8 * GetRectangleWidth( nFieldSize, Get.m_tablenames[ind].GetLength(), ind ) );
//					Get.m_height.push_back(28 + 10 * nFieldSize);
//				}
//			}
//	}
//	nSize = Get.m_createdtable.size();
//	Get.m_previous_mouse_x.resize(nSize);
//	Get.m_previous_mouse_y.resize(nSize);
//	GetOneAdditionFields();
//	//GetRelationLines();
//    Refresh(); 
//	Update();
//}
void CustomDialog::OnExcludeCb(wxCommandEvent &WXUNUSED(event))
{
	if( m_pCheckbox->IsChecked() )
	{
		UpdateAddTableData();
		return;
	}
    UpdateTable();
}
void CustomDialog::UpdateTable()
{
	int nTableCount = Get.m_tablenames.size();
	int ind = -1;
	m_pTables->Clear();
	m_pChoices.Clear();
	for( int i = 0; i < nTableCount; ++i )
		{
		  ind++;
		  CT2CA pszConvertedAnsiString (Get.m_tablenames[i]);
	      std::string strStd (pszConvertedAnsiString);
		  m_pTables->Insert(wxString::FromUTF8(_strdup(strStd.c_str() ) ), ind); 
		  m_pChoices.Insert(wxString::FromUTF8(_strdup(strStd.c_str() ) ), ind); 
		}
	Refresh(); 
	Update();
}

void CustomDialog::GetRelationLines(std::vector<std::pair<int, int>> &Drawnline, unsigned &nSize)
{
	wxString temp;
	///Get.m_Drawnline.clear();
	unsigned nDeleteRelationCount = Get.m_deletefirstrelation.size();
	unsigned nUpdateRelationCount = Get.m_updatefirstrelation.size();
	unsigned nForeignKeyCount = Get.m_foreignkeyfirsttable.size();

	for(unsigned i = 0; i < nSize; ++i)
	{
		for(unsigned i1 = 0; i1 < nDeleteRelationCount; ++i1)
			{
				if( !(Get.m_createdtable[i].Cmp(Get.m_deletefirstrelation[i1].first)) )
				{
					temp = CstringToWxString(Get.m_deletesecondrelation[i1].first);
					for( unsigned i2 = 0; i2 < nSize; ++i2 )
					{
						if(!(temp.Cmp(Get.m_createdtable[i2]) ) )
						{
							Drawnline.push_back(std::make_pair(i,i2));
							Get.m_RelationIndex.push_back(std::make_pair(i1,1));
						}
					}
				}
			}
	}


	for(unsigned i = 0; i < nSize; ++i)
	{
		for(unsigned i1 = 0; i1 < nUpdateRelationCount; ++i1)
			{
				if( !(Get.m_createdtable[i].Cmp(Get.m_updatefirstrelation[i1].first)) )
				{
					temp = CstringToWxString(Get.m_updatesecondrelation[i1].first);
					for( unsigned i2 = 0; i2 < nSize; ++i2 )
					{
						if(!(temp.Cmp(Get.m_createdtable[i2]) ) )
						{
							Drawnline.push_back(std::make_pair(i,i2));
							Get.m_RelationIndex.push_back(std::make_pair(i1,2));
						}
					}
				}
			}
	}


	for(unsigned i = 0; i < nSize; ++i)
	{
		for(unsigned i1 = 0; i1 < nForeignKeyCount; ++i1)
			{
				if( !(Get.m_createdtable[i].Cmp(Get.m_foreignkeyfirsttable[i1])) )
				{
					temp = CstringToWxString(Get.m_foreignkeysecondtable[i1]);
					for( unsigned i2 = 0; i2 < nSize; ++i2 )
					{
						if(!(temp.Cmp(Get.m_createdtable[i2]) ) )
						{
							Drawnline.push_back(std::make_pair(i,i2));
							Get.m_RelationIndex.push_back(std::make_pair(i1,4));
						}
					}
				}
			}
	}
}
void CustomDialog::MakeTableSelected()
{
	Get.m_createdtable.push_back(Get.m_createdtable[Get.m_rect_index]);
	Get.m_createdtable.erase(Get.m_createdtable.begin() + Get.m_rect_index );
	Get.m_x.push_back(Get.m_x[Get.m_rect_index]);
	Get.m_x.erase(Get.m_x.begin() + Get.m_rect_index );
	Get.m_y.push_back(Get.m_y[Get.m_rect_index]);
	Get.m_y.erase(Get.m_y.begin() + Get.m_rect_index );
	Get.m_width.push_back(Get.m_width[Get.m_rect_index]);
	Get.m_width.erase(Get.m_width.begin() + Get.m_rect_index );
	Get.m_height.push_back(Get.m_height[Get.m_rect_index]);
	Get.m_height.erase(Get.m_height.begin() + Get.m_rect_index );
	Get.m_previous_mouse_x.push_back(Get.m_previous_mouse_x[Get.m_rect_index]);
	Get.m_previous_mouse_x.erase(Get.m_previous_mouse_x.begin() + Get.m_rect_index );
	Get.m_previous_mouse_y.push_back(Get.m_previous_mouse_y[Get.m_rect_index]);
	Get.m_previous_mouse_y.erase(Get.m_previous_mouse_y.begin() + Get.m_rect_index );
	std::vector <wxString> temporary;
	unsigned nFieldSize = Get.m_createdfields[Get.m_rect_index].size();
	for(unsigned i = 0; i < nFieldSize; ++i)
		temporary.push_back(Get.m_createdfields[Get.m_rect_index][i]);
	std::vector <int> temporary2;
	for( unsigned i = 0; i < nFieldSize; ++i )
		temporary2.push_back(Get.m_createdfieldcoordy[Get.m_rect_index][i]);
	Get.m_createdfieldcoordy.push_back(temporary2);
	Get.m_createdfieldcoordy.erase(Get.m_createdfieldcoordy.begin() + Get.m_rect_index);
	std::vector <bool> temporary3;
	for(unsigned i = 0; i < nFieldSize; ++i)
		temporary3.push_back(Get.m_oneaddition[Get.m_rect_index][i]);
	Get.m_oneaddition.push_back(temporary3);
	Get.m_oneaddition.erase(Get.m_oneaddition.begin() + Get.m_rect_index);
	Get.m_createdfields.push_back(temporary);
	Get.m_createdfields.erase(Get.m_createdfields.begin() + Get.m_rect_index);
	Get.m_rect_index = Get.m_createdtable.size() - 1;
}
wxString CustomDialog::CstringToWxString(CString &String4Conversion)
{
	CT2CA pszConvertedAnsiString (String4Conversion);
	std::string strStd (pszConvertedAnsiString);
    return wxString::FromUTF8(_strdup(strStd.c_str() ) );
}
int CustomDialog::GetIndex(CString sSearch)
{
	unsigned nSize = Get.m_tablenames.size();
	for(unsigned i3 = 0; i3 < nSize; ++i3)
	{
		if( !(sSearch.Compare(Get.m_tablenames[i3])) )
			return i3;
	}
	return -1;
}
int CustomDialog::GetIndex(wxString sSearch)
{
	unsigned nSize = Get.m_tablenames.size();
	for(unsigned i3 = 0; i3 < nSize; ++i3)
	{
		if( sSearch == CstringToWxString(Get.m_tablenames[i3]) )
			return i3;
	}
	return wxNOT_FOUND;
}
int CustomDialog::GetCreatedTableIndex(wxString sSearch)
{
	unsigned nSize = Get.m_createdtable.size();
	for(unsigned i = 0; i < nSize; ++i)
	{
		if( sSearch == Get.m_createdtable[i] )
			return i;
	}
	return -1;
}
bool CustomDialog::isCreated(CString &sSearch)
{
	unsigned nSize = Get.m_createdtable.size();
	for(unsigned i2 = 0; i2 < nSize; ++i2 )
	{
		if( !(sSearch.Compare(Get.m_createdtable[i2]) ) )
			return true;
	}
	return false;
}
//void CustomDialog::OnShowAllRelations(wxCommandEvent &WXUNUSED(event) )
//{
//	Get.m_createdfields.clear();
//	Get.m_createdfieldcoordy.clear();
//	Get.m_createdtable.clear();
//	Get.m_height.clear();
//	Get.m_oneaddition.clear();
//	Get.m_previous_mouse_x.clear();
//	Get.m_previous_mouse_y.clear();
//	Get.m_x.clear();
//	Get.m_y.clear();
//	Get.m_width.clear();
//	unsigned nDeleteRelationCount = Get.m_deletefirstrelation.size();
//	unsigned nUpdateRelationCount = Get.m_updatefirstrelation.size();
//	unsigned nForeignKeyCount = Get.m_foreignkeyfirsttable.size();
//	std::vector<wxString> temp;
//	int ind;
//	unsigned nFieldSize;
//	for( unsigned i = 0; i < nDeleteRelationCount; ++i )
//	{
//		if ( !(isCreated(Get.m_deletefirstrelation[i].first)) )
//		{
//			Get.m_createdtable.push_back( CstringToWxString(Get.m_deletefirstrelation[i].first) );
//			Get.m_x.push_back ( ::rand() % 700 + 20);
//			Get.m_y.push_back( ::rand() % 700 + 20);
//			Get.m_createdfields.push_back( temp );
//			ind = GetIndex(Get.m_deletefirstrelation[i].first);
//			nFieldSize = Get.m_fieldnames[ind].size();
//			for( unsigned i1 = 0; i1 < nFieldSize; ++i1 )
//				Get.m_createdfields.back().push_back( CstringToWxString(Get.m_fieldnames[ind][i1]) );
//			Get.m_width.push_back( 8 * GetRectangleWidth( nFieldSize, Get.m_tablenames[ind].GetLength(), ind ) );
//			Get.m_height.push_back(28 + 10 * nFieldSize);
//		}
//
//		if ( !(isCreated(Get.m_deletesecondrelation[i].first)) )
//		{
//			Get.m_createdtable.push_back( CstringToWxString(Get.m_deletesecondrelation[i].first) );
//			Get.m_x.push_back ( ::rand() % 700 + 20);
//			Get.m_y.push_back( ::rand() % 700 + 20);
//			Get.m_createdfields.push_back( temp );
//			ind = GetIndex(Get.m_deletesecondrelation[i].first);
//			nFieldSize = Get.m_fieldnames[ind].size();
//			for( unsigned i1 = 0; i1 < nFieldSize; ++i1 )
//				Get.m_createdfields.back().push_back( CstringToWxString(Get.m_fieldnames[ind][i1]) );
//			Get.m_width.push_back( 8 * GetRectangleWidth( nFieldSize, Get.m_tablenames[ind].GetLength(), ind ) );
//			Get.m_height.push_back(28 + 10 * nFieldSize);
//		}
//	}
//
//
//
//	for( unsigned i = 0; i < nUpdateRelationCount; ++i )
//	{
//		if ( !(isCreated(Get.m_updatefirstrelation[i].first)) )
//		{
//			Get.m_createdtable.push_back( CstringToWxString(Get.m_updatefirstrelation[i].first) );
//			Get.m_x.push_back ( ::rand() % 700 + 20);
//			Get.m_y.push_back( ::rand() % 700 + 20);
//			Get.m_createdfields.push_back( temp );
//			ind = GetIndex(Get.m_updatefirstrelation[i].first);
//			nFieldSize = Get.m_fieldnames[ind].size();
//			for( unsigned i1 = 0; i1 < nFieldSize; ++i1 )
//				Get.m_createdfields.back().push_back( CstringToWxString(Get.m_fieldnames[ind][i1]) );
//			Get.m_width.push_back( 8 * GetRectangleWidth( nFieldSize, Get.m_tablenames[ind].GetLength(), ind ) );
//			Get.m_height.push_back(28 + 10 * nFieldSize);
//		}
//
//		if ( !(isCreated(Get.m_updatesecondrelation[i].first)) )
//		{
//			Get.m_createdtable.push_back( CstringToWxString(Get.m_updatesecondrelation[i].first) );
//			Get.m_x.push_back ( ::rand() % 700 + 20);
//			Get.m_y.push_back( ::rand() % 700 + 20);
//			Get.m_createdfields.push_back( temp );
//			ind = GetIndex(Get.m_updatesecondrelation[i].first);
//			nFieldSize = Get.m_fieldnames[ind].size();
//			for( unsigned i1 = 0; i1 < nFieldSize; ++i1 )
//				Get.m_createdfields.back().push_back( CstringToWxString(Get.m_fieldnames[ind][i1]) );
//			Get.m_width.push_back( 8 * GetRectangleWidth( nFieldSize, Get.m_tablenames[ind].GetLength(), ind ) );
//			Get.m_height.push_back(28 + 10 * nFieldSize);
//		}
//	}
//
//
//	for( unsigned i = 0; i < nForeignKeyCount; ++i )
//	{
//		if ( !(isCreated(Get.m_foreignkeyfirsttable[i])) )
//		{
//			Get.m_createdtable.push_back( CstringToWxString(Get.m_foreignkeyfirsttable[i]) );
//			Get.m_x.push_back ( ::rand() % 700 + 20);
//			Get.m_y.push_back( ::rand() % 700 + 20);
//			Get.m_createdfields.push_back( temp );
//			ind = GetIndex(Get.m_foreignkeyfirsttable[i]);
//			nFieldSize = Get.m_fieldnames[ind].size();
//			for( unsigned i1 = 0; i1 < nFieldSize; ++i1 )
//				Get.m_createdfields.back().push_back( CstringToWxString(Get.m_fieldnames[ind][i1]) );
//			Get.m_width.push_back( 8 * GetRectangleWidth( nFieldSize, Get.m_tablenames[ind].GetLength(), ind ) );
//			Get.m_height.push_back(28 + 10 * nFieldSize);
//		}
//
//		if ( !(isCreated(Get.m_foreignkeysecondtable[i])) )
//		{
//			Get.m_createdtable.push_back( CstringToWxString(Get.m_foreignkeysecondtable[i]) );
//			Get.m_x.push_back ( ::rand() % 700 + 20);
//			Get.m_y.push_back( ::rand() % 700 + 20);
//			Get.m_createdfields.push_back( temp );
//			ind = GetIndex(Get.m_foreignkeysecondtable[i]);
//			nFieldSize = Get.m_fieldnames[ind].size();
//			for( unsigned i1 = 0; i1 < nFieldSize; ++i1 )
//				Get.m_createdfields.back().push_back( CstringToWxString(Get.m_fieldnames[ind][i1]) );
//			Get.m_width.push_back( 8 * GetRectangleWidth( nFieldSize, Get.m_tablenames[ind].GetLength(), ind ) );
//			Get.m_height.push_back(28 + 10 * nFieldSize);
//		}
//	}
//	unsigned nSize = Get.m_createdtable.size();
//	Get.m_previous_mouse_x.resize(nSize);
//	Get.m_previous_mouse_y.resize(nSize);
//	GetOneAdditionFields();
//    Refresh(); 
//	Update();
//}
void CustomDialog::UpdateAddTableData()
 {
	 int nTableCount = Get.m_tablenames.size();
		int ind = -1;
		m_pTables->Clear();
		m_pChoices.Clear();
		for( int i = 0; i < nTableCount; ++i )
		{
			if( GetCreatedTableIndex(CstringToWxString(Get.m_tablenames[i])) != -1){	
				continue;
				}
		  ind++;
		  CT2CA pszConvertedAnsiString (Get.m_tablenames[i]);
	      std::string strStd (pszConvertedAnsiString);
		  m_pTables->Insert(wxString::FromUTF8(_strdup(strStd.c_str() ) ), ind); 
		  m_pChoices.Insert(wxString::FromUTF8(_strdup(strStd.c_str() ) ), ind); 
		}
	Refresh(); 
	Update();
 }
void CustomDialog::OnLeftDoubleClick(wxMouseEvent &event)
{
	wxClientDC dc(this);
	wxColour GetColour;
	wxColour SearchedColour(0, 0 ,0);
	int nCurPosx = event.GetPosition().x;
	int nCurPosy = event.GetPosition().y;
	double RectangleArea;
	double TriangleEdgeA;
	double TriangleEdgeB;
	double TriangleEdgeC;
	double TriangleEdgeD;
	double Rectanglem_width;
	double Rectanglem_height;
	bool bDeleteCascade = false;
	bool bUpdateCascade = false;
	bool bReferentialIntegrity = false;
	wxString FirstRelationTable;
	wxString SecondRelationTable;
	wxString FirstRelationField;
	wxString SecondRelationField;
	dc.GetPixel(nCurPosx, nCurPosy, &GetColour);
	if(GetColour == SearchedColour)
	{
	  unsigned nLines = Get.m_boundaringlineindexes.size();
	  for( unsigned i5 = 0; i5 < nLines; ++i5 )
	  {
			 TriangleEdgeA = sqrt((double)(nCurPosx - Get.m_linetopleft[i5].first) * (nCurPosx - Get.m_linetopleft[i5].first) + (nCurPosy - Get.m_linetopleft[i5].second) * (nCurPosy - Get.m_linetopleft[i5].second));
			 TriangleEdgeB = sqrt((double)(nCurPosx - Get.m_linetopright[i5].first) * (nCurPosx - Get.m_linetopright[i5].first) + (nCurPosy - Get.m_linetopright[i5].second) * (nCurPosy - Get.m_linetopright[i5].second));
			 TriangleEdgeC = sqrt((double)(nCurPosx - Get.m_linebottomright[i5].first) * (nCurPosx - Get.m_linebottomright[i5].first) + (nCurPosy - Get.m_linebottomright[i5].second) * (nCurPosy - Get.m_linebottomright[i5].second));
			 TriangleEdgeD = sqrt((double)(nCurPosx - Get.m_linebottomleft[i5].first) * (nCurPosx - Get.m_linebottomleft[i5].first) + (nCurPosy - Get.m_linebottomleft[i5].second) * (nCurPosy - Get.m_linebottomleft[i5].second));
			 Rectanglem_width = sqrt((double)(Get.m_linetopright[i5].first - Get.m_linetopleft[i5].first) * (Get.m_linetopright[i5].first - Get.m_linetopleft[i5].first) + (Get.m_linetopright[i5].second - Get.m_linetopleft[i5].second) * (Get.m_linetopright[i5].second - Get.m_linetopleft[i5].second));
			 Rectanglem_height = sqrt((double)(Get.m_linetopright[i5].first - Get.m_linebottomright[i5].first) * (Get.m_linetopright[i5].first - Get.m_linebottomright[i5].first) + (Get.m_linetopright[i5].second - Get.m_linebottomright[i5].second) * (Get.m_linetopright[i5].second - Get.m_linebottomright[i5].second));
			 RectangleArea = Rectanglem_height * Rectanglem_width;
			 if(Triangle(TriangleEdgeA, TriangleEdgeB, Rectanglem_width) + Triangle(TriangleEdgeB, TriangleEdgeC, Rectanglem_height) + Triangle(TriangleEdgeC, TriangleEdgeD, Rectanglem_width) + 
				Triangle(TriangleEdgeD , TriangleEdgeA, Rectanglem_height) <= RectangleArea)
			 {
				 FirstRelationTable = RelationshipView(Get.m_boundaringlineindexes[i5].first, Get.m_boundaringlineindexes[i5].second, Get.m_RelationIndex[i5].first, Get.m_RelationIndex[i5].second);
				 if(Get.m_RelationIndex[i5].second & 1)
				 {
					 bDeleteCascade = true; 
					 FirstRelationField = Get.m_deletefirstrelation[Get.m_RelationIndex[i5].first].second;
					 SecondRelationField = Get.m_deletesecondrelation[Get.m_RelationIndex[i5].first].second;
				 }
				 else if(Get.m_RelationIndex[i5].second & 2)
				 {
					 bUpdateCascade = true;
					 FirstRelationField = Get.m_updatefirstrelation[Get.m_RelationIndex[i5].first].second;
					 SecondRelationField = Get.m_updatesecondrelation[Get.m_RelationIndex[i5].first].second;
				 }
				 else
					 { 
						 bReferentialIntegrity = true;
						 FirstRelationField = Get.m_foreignkeyfirstfield[Get.m_RelationIndex[i5].first];
						 SecondRelationField = Get.m_foreignkeysecondfield[Get.m_RelationIndex[i5].first];
					 }
				 if( FirstRelationTable == Get.m_createdtable[Get.m_boundaringlineindexes[i5].first] )
					 SecondRelationTable = Get.m_createdtable[Get.m_boundaringlineindexes[i5].second];
				 else
					 SecondRelationTable = Get.m_createdtable[Get.m_boundaringlineindexes[i5].first];
			 }
	  }
	    EditRelationShips(FirstRelationTable, FirstRelationField, SecondRelationTable, SecondRelationField, bUpdateCascade, bDeleteCascade, bReferentialIntegrity);
	}
}
wxString CustomDialog::RelationshipView(int &FirstTableIndex, int &SecondTableIndex, int &RelationshipIndex, int &RelationFlag)
{
	wxString CreatedFirstTable = Get.m_createdtable[FirstTableIndex];
	wxString CreatedSecondTable = Get.m_createdtable[SecondTableIndex];
	wxString Temp;
	if( RelationFlag & 1 )
	{
		if(CreatedFirstTable == CstringToWxString(Get.m_deletesecondrelation[RelationshipIndex].first) && 
		    CreatedSecondTable == CstringToWxString(Get.m_deletefirstrelation[RelationshipIndex].first))
			{
					Temp = CreatedSecondTable;
					CreatedSecondTable = CreatedFirstTable;
					CreatedFirstTable = Temp;
			}
	}
	else if( RelationFlag & 2 )
	{
		if(CreatedFirstTable == CstringToWxString(Get.m_updatesecondrelation[RelationshipIndex].first) && 
		    CreatedSecondTable == CstringToWxString(Get.m_updatefirstrelation[RelationshipIndex].first))
			{
					Temp = CreatedSecondTable;
					CreatedSecondTable = CreatedFirstTable;
					CreatedFirstTable = Temp;
			}
	}
	else 
	{
		if(CreatedFirstTable == CstringToWxString(Get.m_foreignkeysecondfield[RelationshipIndex]) && 
		    CreatedSecondTable == CstringToWxString(Get.m_foreignkeyfirstfield[RelationshipIndex]))
			{
					Temp = CreatedSecondTable;
					CreatedSecondTable = CreatedFirstTable;
					CreatedFirstTable = Temp;
			}
	}
	return CreatedFirstTable;
}
double CustomDialog::Triangle(double a, double b, double c)
 {
   double p = (a+b+c)/2;
   double area = sqrt((double)p*(p-a)*(p-b)*(p-c));
   return area;
 }
void CustomDialog::EditRelationShips(wxString &FirstRelationTable, wxString &FirstRelationField, wxString &SecondRelationTable, wxString &SecondRelationField, const bool &bUpdateCascade,
	                                 const bool &bDeleteCascade, const bool &bReferentialIntegrity)
{
	wxDialog *dlg = new wxDialog(this, wxID_ANY, wxT("Relationship View"), wxDefaultPosition, wxSize(220,260));
	wxSizer *TopSizer = new wxBoxSizer(wxVERTICAL);
	wxTextCtrl *Table = new wxTextCtrl(dlg, wxID_ANY, FirstRelationTable, wxDefaultPosition, wxSize(150,20), wxTE_READONLY);
	wxTextCtrl *TableField = new wxTextCtrl(dlg, wxID_ANY, FirstRelationField, wxDefaultPosition, wxSize(150,20), wxTE_READONLY);
	wxTextCtrl *RelatedTable = new wxTextCtrl(dlg, wxID_ANY, SecondRelationTable, wxDefaultPosition, wxSize(150,20), wxTE_READONLY);
	wxTextCtrl *RelatedTableField = new wxTextCtrl(dlg, wxID_ANY, SecondRelationField, wxDefaultPosition, wxSize(150,20), wxTE_READONLY);
	wxCheckBox *EnforceReferentialIntegrity = new wxCheckBox(dlg, wxID_ANY, wxT("Enforce Referential Integrity"));
	wxCheckBox *UpdateCascade = new wxCheckBox(dlg, wxID_ANY, wxT("Update Related Records"));
	wxCheckBox *DeleteCascade = new wxCheckBox(dlg, wxID_ANY, wxT("Delete Related Records"));
	EnforceReferentialIntegrity->SetToolTip(wxT("FOREIGN KEY (table_field) REFERENCES related_table(related_table_field)"));
	DeleteCascade->SetToolTip(wxT("CREATE TRIGGER trigger_name AFTER DELETE ON table FOR EACH ROW BEGIN DELETE FROM related_table WHERE related_table_field = OLD.table_field; END"));
	UpdateCascade->SetToolTip(wxT("CREATE TRIGGER trigger_name AFTER UPDATE OF table_field ON table BEGIN UPDATE related_table SET related_table_field = new.table_field WHERE related_table_field = OLD.table_field; END"));
	if( bUpdateCascade )
		UpdateCascade->SetValue(true);
	if( bDeleteCascade )
		DeleteCascade->SetValue(true);
	if( bReferentialIntegrity )
		EnforceReferentialIntegrity->SetValue(true);
	unsigned nPrimaryKeySize = Get.m_primarykeyfields.size();
	wxString ComparisonFirstTable = FirstRelationTable + FirstRelationField;
	wxString ComparisonSecondTable = SecondRelationTable + SecondRelationField;
	wxCheckBox *PrimaryKeyFirstField = new wxCheckBox(dlg, wxID_ANY, wxT("PK"));
	wxCheckBox *PrimaryKeySecondField = new wxCheckBox(dlg, wxID_ANY, wxT("PK"));
	for( unsigned i1 = 0; i1 < nPrimaryKeySize; ++i1 )
		{
			if( !(ComparisonFirstTable.Cmp(CstringToWxString(Get.m_primarykeyfields[i1]))) )
			{
				PrimaryKeyFirstField->SetValue(true);
			}
			if( !(ComparisonSecondTable.Cmp(CstringToWxString(Get.m_primarykeyfields[i1]))) )
			{
				PrimaryKeySecondField->SetValue(true);
			}
		}
	PrimaryKeyFirstField->Enable(false);
	PrimaryKeySecondField->Enable(false);
	wxSizer *FirstField = new wxBoxSizer(wxHORIZONTAL);
	wxSizer *SecondField = new wxBoxSizer(wxHORIZONTAL);
	wxSizer *hbox = new wxBoxSizer(wxHORIZONTAL);
	hbox->Add(new wxButton(dlg, wxID_OK, "OK", wxDefaultPosition, wxSize(60,25)), 0);
	hbox->AddSpacer(5);
	hbox->Add(new wxButton(dlg, wxID_CANCEL, "Cancel", wxDefaultPosition, wxSize(60,25)), 0);

	TopSizer->Add(new wxStaticText(dlg, wxID_ANY, "&Table:"), 0, wxLEFT, 3);
	TopSizer->Add(Table, 0, wxEXPAND | wxLEFT | wxRIGHT, 3);
	TopSizer->AddSpacer(5);
	FirstField->Add(TableField);
	FirstField->AddSpacer(5);
	FirstField->Add(PrimaryKeyFirstField);
	TopSizer->Add(FirstField, 0, wxEXPAND | wxLEFT | wxRIGHT, 3);
	TopSizer->AddSpacer(5);
	TopSizer->Add(new wxStaticText(dlg, wxID_ANY, "&Related Table:"), 0, wxLEFT, 3);
	TopSizer->AddSpacer(5);
	TopSizer->Add(RelatedTable, 0, wxEXPAND | wxLEFT | wxRIGHT, 3);
	TopSizer->AddSpacer(5);
	SecondField->Add(RelatedTableField);
	SecondField->AddSpacer(5);
	SecondField->Add(PrimaryKeySecondField);
	TopSizer->Add(SecondField, 0, wxEXPAND | wxLEFT | wxRIGHT, 3);
	TopSizer->AddSpacer(10);
	TopSizer->Add(EnforceReferentialIntegrity, 0, wxLEFT, 3);
	TopSizer->AddSpacer(5);
	TopSizer->Add(UpdateCascade, 0, wxLEFT, 3);
	TopSizer->AddSpacer(5);
	TopSizer->Add(DeleteCascade, 0, wxLEFT, 3);
	TopSizer->Add(hbox, 0, wxLEFT | wxTOP, 5);
	dlg->SetSizer(TopSizer);
	if( dlg->ShowModal() == wxID_OK )
	{
		CString firstrelationtable;
		CString firstrelationfield;
		CString secondrelationtable;
		CString secondrelationfield;
		const char *temp = SecondRelationTable.mb_str();
		std::string str(temp);
		secondrelationtable.Format(_T("%S"), str.c_str());
		const char *temp2 = FirstRelationTable.mb_str();
		std::string str2(temp2);
		firstrelationtable.Format(_T("%S"), str2.c_str());
		const char *temp3 = SecondRelationField.mb_str();
		std::string str3(temp3);
		secondrelationfield.Format(_T("%S"), str3.c_str());
		const char *temp4 = FirstRelationField.mb_str();
		std::string str4(temp4);
		firstrelationfield.Format(_T("%S"), str4.c_str());
		if(bUpdateCascade == false &&  UpdateCascade->GetValue() == true)
		{
			cExecuteSqlite::ExecuteSqliteStatements(firstrelationtable, secondrelationtable, firstrelationfield, secondrelationfield, m_dPath.c_str(), 
				                                    Get.m_deletefirstrelation, Get.m_deletesecondrelation, Get.m_updatefirstrelation, Get.m_updatesecondrelation, 
													Get.m_deletetriggernames, Get.m_updatetriggernames, Get.m_foreignkeyfirsttable, Get.m_foreignkeysecondtable, 
													Get.m_foreignkeyfirstfield, Get.m_foreignkeysecondfield, Get.m_tablenamesSQL, Get.m_tablenames, 1);
		}
		if(bDeleteCascade == false &&  DeleteCascade->GetValue() == true)
		{
			cExecuteSqlite::ExecuteSqliteStatements(firstrelationtable, secondrelationtable, firstrelationfield, secondrelationfield, m_dPath.c_str(), 
				                                    Get.m_deletefirstrelation, Get.m_deletesecondrelation, Get.m_updatefirstrelation, Get.m_updatesecondrelation, 
													Get.m_deletetriggernames, Get.m_updatetriggernames, Get.m_foreignkeyfirsttable, Get.m_foreignkeysecondtable, 
													Get.m_foreignkeyfirstfield, Get.m_foreignkeysecondfield, Get.m_tablenamesSQL, Get.m_tablenames, 2);
		}
		if(bReferentialIntegrity == false &&  EnforceReferentialIntegrity->GetValue() == true )
		{
			cExecuteSqlite::ExecuteSqliteStatements(firstrelationtable, secondrelationtable, firstrelationfield, secondrelationfield, m_dPath.c_str(), 
				                                    Get.m_deletefirstrelation, Get.m_deletesecondrelation, Get.m_updatefirstrelation, Get.m_updatesecondrelation, 
													Get.m_deletetriggernames, Get.m_updatetriggernames, Get.m_foreignkeyfirsttable, Get.m_foreignkeysecondtable, 
													Get.m_foreignkeyfirstfield, Get.m_foreignkeysecondfield, Get.m_tablenamesSQL, Get.m_tablenames, 4);
		}

		if(bUpdateCascade == true &&  UpdateCascade->GetValue() == false)
		{
			cExecuteSqlite::ExecuteSqliteStatements(firstrelationtable, secondrelationtable, firstrelationfield, secondrelationfield, m_dPath.c_str(), 
				                                    Get.m_deletefirstrelation, Get.m_deletesecondrelation, Get.m_updatefirstrelation, Get.m_updatesecondrelation, 
													Get.m_deletetriggernames, Get.m_updatetriggernames, Get.m_foreignkeyfirsttable, Get.m_foreignkeysecondtable, 
													Get.m_foreignkeyfirstfield, Get.m_foreignkeysecondfield, Get.m_tablenamesSQL, Get.m_tablenames, 8);
		}
		if(bDeleteCascade == true &&  DeleteCascade->GetValue() == false)
		{
			cExecuteSqlite::ExecuteSqliteStatements(firstrelationtable, secondrelationtable, firstrelationfield, secondrelationfield, m_dPath.c_str(), 
				                                    Get.m_deletefirstrelation, Get.m_deletesecondrelation, Get.m_updatefirstrelation, Get.m_updatesecondrelation, 
													Get.m_deletetriggernames, Get.m_updatetriggernames, Get.m_foreignkeyfirsttable, Get.m_foreignkeysecondtable, 
													Get.m_foreignkeyfirstfield, Get.m_foreignkeysecondfield, Get.m_tablenamesSQL, Get.m_tablenames, 16);
		}
		if(bReferentialIntegrity == true &&  EnforceReferentialIntegrity->GetValue() == false )
		{
			cExecuteSqlite::ExecuteSqliteStatements(firstrelationtable, secondrelationtable, firstrelationfield, secondrelationfield, m_dPath.c_str(), 
				                                    Get.m_deletefirstrelation, Get.m_deletesecondrelation, Get.m_updatefirstrelation, Get.m_updatesecondrelation, 
													Get.m_deletetriggernames, Get.m_updatetriggernames, Get.m_foreignkeyfirsttable, Get.m_foreignkeysecondtable, 
													Get.m_foreignkeyfirstfield, Get.m_foreignkeysecondfield, Get.m_tablenamesSQL, Get.m_tablenames, 32);
		}
	}
	dlg->Destroy();
	/*delete Table; 
	delete TableField;
	delete RelatedTable;
	delete RelatedTableField;
	delete EnforceReferentialIntegrity;
	delete UpdateCascade;
	delete DeleteCascade;
	delete PrimaryKeyFirstField;
	delete PrimaryKeySecondField;*/
	GetOneAdditionFields();
	//GetRelationLines();
	Refresh(); 
	Update();
}
void CustomDialog::SaveTables(wxCommandEvent &WXUNUSED(event))
{
	cExecuteSqlite::SaveCurrentDrawing(Get.m_createdtable, Get.m_x, Get.m_y, m_dPath.c_str());
}
void CustomDialog::GetOneAdditionFields()
{
	Get.m_oneaddition.clear();
	int nSize = Get.m_createdtable.size();
	int nFieldSize;
	bool isPrimary;
	wxString Temp;
	std::vector<bool> Temporary;
	int nPrimaryKeyCount = Get.m_primarykeyfields.size();
	int nUniqueCount = Get.m_uniquefields.size();
	for( int i = 0; i < nSize; ++i )
	{
		Get.m_oneaddition.push_back(Temporary);
		nFieldSize = Get.m_createdfields[i].size();
		for( int i1 = 0; i1 < nFieldSize; ++i1 )
		{
			isPrimary = false;
			Temp = Get.m_createdtable[i] + Get.m_createdfields[i][i1];
			for( int i2 = 0; i2 < nPrimaryKeyCount; ++i2 )
			{
				if( Temp == CstringToWxString(Get.m_primarykeyfields[i2]) )
				{
					isPrimary = true;
					break;
				}
			}
			if ( !isPrimary )
			{
				for( int i2 = 0; i2 < nUniqueCount; ++i2 )
				{
					if( Temp == CstringToWxString(Get.m_uniquefields[i2]) )
					{
						isPrimary = true;
						break;
					}
				}
			}
			Get.m_oneaddition[i].push_back(isPrimary);
		}
	}
}
void CustomDialog::SaveWindowInformationAddTable(int &sizex, int &sizey, int &posx, int &posy, bool &isChecked)
{
	std::ofstream WindowPositionFile("WindowInformationAddTable.ini");
	WindowPositionFile << "[WindowSize]" << std::endl;
	WindowPositionFile << "x = " << sizex << std::endl;
	WindowPositionFile << "y = " << sizey << std::endl;
	WindowPositionFile << "[WindowPosition]" << std::endl;
	WindowPositionFile << "x_coord = " << posx << std::endl;
	WindowPositionFile << "y_coord = " << posy << std::endl;
	WindowPositionFile << "[Other]" << posy << std::endl;
	WindowPositionFile << "exclude = ";
	 if( isChecked )
		 WindowPositionFile << "true" << std::endl;
	 else WindowPositionFile << "false" << std::endl;
	WindowPositionFile.close();
}
void CustomDialog::GetWindowInformationAddTable(int &sizex, int &sizey, int &posx, int &posy, bool &isChecked)
{
	CSimpleIni ini;
    ini.SetUnicode();
    ini.LoadFile("WindowInformationAddTable.ini");
	sizex = ini.GetLongValue(_T("WindowSize"), _T("x"), 235);
	sizey = ini.GetLongValue(_T("WindowSize"), _T("y"), 290);
	posx = ini.GetLongValue(_T("WindowPosition"), _T("x_coord"), 200);
	posy = ini.GetLongValue(_T("WindowPosition"), _T("y_coord"), 200);
	isChecked = ini.GetBoolValue(_T("Other"), _T("exclude"), false);
}

void CustomDialog::SaveWindowInformationRemTable(int &sizex, int &sizey, int &posx, int &posy)
{
	std::ofstream WindowPositionFile("WindowInformationRemTable.ini");
	WindowPositionFile << "[WindowSize]" << std::endl;
	WindowPositionFile << "x = " << sizex << std::endl;
	WindowPositionFile << "y = " << sizey << std::endl;
	WindowPositionFile << "[WindowPosition]" << std::endl;
	WindowPositionFile << "x_coord = " << posx << std::endl;
	WindowPositionFile << "y_coord = " << posy << std::endl;
	WindowPositionFile.close();
}
void CustomDialog::GetWindowInformationRemTable(int &sizex, int &sizey, int &posx, int &posy)
{
	CSimpleIni ini;
    ini.SetUnicode();
    ini.LoadFile("WindowInformationRemTable.ini");
	sizex = ini.GetLongValue(_T("WindowSize"), _T("x"), 235);
	sizey = ini.GetLongValue(_T("WindowSize"), _T("y"), 290);
	posx = ini.GetLongValue(_T("WindowPosition"), _T("x_coord"), 200);
	posy = ini.GetLongValue(_T("WindowPosition"), _T("y_coord"), 200);
}
