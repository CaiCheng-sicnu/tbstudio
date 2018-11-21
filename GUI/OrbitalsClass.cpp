#include "OrbitalsClass.h"

OrbitalsClass::OrbitalsClass(wxWindow* parent, Sec30* sec30var, wxWindowID id, const wxPoint& pos, const wxSize& size, long style)
    : wxPanel(parent, id, pos, size, style)
{
    sec30=sec30var;
    /**********************************************************************************************************************************************/
    wxBoxSizer* BaseSizer = new wxBoxSizer(wxVERTICAL);
    SetSizer(BaseSizer);
    SetBackgroundColour(wxColour(wxT("rgb(255,255,255)")));
    SetName(wxT("OrbitalsClass"));
    SetSize(wxDLG_UNIT(this, wxSize(-1,-1)));
    GetSizer()->Fit(this);
    /**********************************************************************************************************************************************/
    sec30->AddGroupBox(this,_("Atom Species in the Unit-Cell"),wxColour(wxT("rgb(153,180,209)")));
    sec30->AddVarVector(this, 1, _("NewType"), _("wxString"), _("New TB Atom Type"), 110, 120,false);
    wxString Labels1[2] = {_("Add TB Atom Type") , _("Remove TB Atom Type")};
    wxObjectEventFunction Funcs1[2] = { wxCommandEventHandler(OrbitalsClass::Btn_AddAtom_OnClick), wxCommandEventHandler(OrbitalsClass::Btn_RemoveAtom_OnClick)};
    sec30->AddButton(this, 2, Labels1, Funcs1);
    sec30->AddListBox(this, _("AtomSpeciesList"), 340, 120);
    wxString Labels2[2] = {_("Add Shell") , _("Remove the Last Shell")};
    wxObjectEventFunction Funcs2[2] = { wxCommandEventHandler(OrbitalsClass::Btn_AddShell_OnClick), wxCommandEventHandler(OrbitalsClass::Btn_RemoveShell_OnClick)};
    sec30->AddButton(this, 2, Labels2, Funcs2);
    /**********************************************************************************************************************************************/
    sec30->AddGroupBox(this,_("Orbitals in Different Atom Species"),wxColour(wxT("rgb(153,180,209)")));
    sec30->AddTreeCtrl(this, _("Orbitals"), 340, 600, false);
    /**********************************************************************************************************************************************/
    sec30->AddGroupBox(this,_(""),wxColour(wxT("rgb(153,180,209)")));
    /**********************************************************************************************************************************************/
    
    //wxListBox* listctr = sec30->GetListObject(_("AtomSpeciesList"));
    //listctr->Append(_("[C]"));
    //listctr->Append(_("[H]"));
    //listctr->Append(_("[Al]"));
    
    wxCheckTree* ctr = sec30->GetTreeObject(_("Orbitals"));
    wxTreeItemId rootID=ctr->AddRoot("TB-Model Atom Species");

    //ctr->ExpandAll();
}
    
OrbitalsClass::~OrbitalsClass()
{
    //try{}
    //catch(std::exception& ex)
    //{wxMessageBox(ex.what());}
}

void OrbitalsClass::Btn_AddAtom_OnClick(wxCommandEvent& event)
{
    wxString newtypename;
    sec30->GetVar(_("NewType[0]"),newtypename);
    if (newtypename == _("")) {wxMessageBox(_("The 'New TB Atom Type' is empty. Please fill out this field and try again."),_("Error")); return;}
    wxListBox* listctr = sec30->GetListObject(_("AtomSpeciesList"));
    
    int nItems = listctr->GetCount();
    for (int i=0; i<nItems; i++)
    {
        wxString itemname= listctr->GetString(i);
        if (newtypename.CompareTo(itemname) == 0) {wxMessageBox(_("There is such an item with the name '") + newtypename + _("'."),_("Error")); return;}
    }
    
    listctr->Append(newtypename);
    listctr->Update();
    listctr->Refresh(true);
    
    wxCheckTree* treectr = sec30->GetTreeObject(_("Orbitals"));
    wxTreeItemId rootID;
    if (treectr->GetItemCount() < 1)
        rootID = treectr->AddRoot(_("TB-Model Atom Species"));
    else
        rootID = treectr->GetRootItem();
    
    wxTreeItemId shellID = treectr->tree_add(rootID,newtypename,false,true);
    treectr->Expand(rootID);
    treectr->Update();
    treectr->Refresh(true);
    
    sec30->SendUpdateEvent(this->GetName(),0);
}

void OrbitalsClass::Btn_RemoveAtom_OnClick(wxCommandEvent& event)
{
    wxListBox* listctr = sec30->GetListObject(_("AtomSpeciesList"));
    int item = listctr->GetSelection();
    if (item<0) {wxMessageBox(_("Select one of the species from the list."),_("Error")); return;}
    wxString SelectedAtomSpecies = listctr->GetString(item);
    listctr->Delete(item);
    listctr->Update();
    listctr->Refresh(true);
    
    wxCheckTree* treectr = sec30->GetTreeObject(_("Orbitals"));
    wxTreeItemId rootID;
    rootID = treectr->GetRootItem();
    wxTreeItemId selectedID = treectr->FindItemIn(rootID,SelectedAtomSpecies);
    treectr->Delete(selectedID);
    //if (treectr->GetItemCount() == 1) treectr->DeleteAllItems();
    treectr->Expand(rootID);
    treectr->Update();
    treectr->Refresh(true);
    
    sec30->SendUpdateEvent(this->GetName(),0);
}

void OrbitalsClass::Btn_AddShell_OnClick(wxCommandEvent& event)
{
    wxListBox* listctr = sec30->GetListObject(_("AtomSpeciesList"));
    int item = listctr->GetSelection();
    if (item<0) {wxMessageBox(_("Select one of the species from the list."),_("Error")); return;}
    wxString SelectedAtomSpecies = listctr->GetString(item);
    
    wxCheckTree* treectr = sec30->GetTreeObject(_("Orbitals"));
    wxTreeItemId rootID = treectr->GetRootItem();
    wxTreeItemId selectedID = treectr->FindItemIn(rootID, SelectedAtomSpecies);
    int nChild = treectr->GetChildrenCount(selectedID, false);
    wxString cnt = wxString::Format(wxT("%d"), nChild + 1);
    wxTreeItemId shellID = treectr->tree_add(selectedID, _("Shell ") + cnt, true, true);
    
    treectr->tree_add(shellID,_("s"),true,true);
    treectr->tree_add(shellID,_("px"),true,true);
    treectr->tree_add(shellID,_("py"),true,true);
    treectr->tree_add(shellID,_("pz"),true,true);
    treectr->tree_add(shellID,_("dxy"),true,false);
    treectr->tree_add(shellID,_("dyz"),true,false);
    treectr->tree_add(shellID,_("dzx"),true,false);
    treectr->tree_add(shellID,_("dx2y2"),true,false);
    treectr->tree_add(shellID,_("dz2"),true,false);
    
    treectr->Expand(rootID);
    treectr->Expand(selectedID);
    treectr->Update();
    treectr->Refresh(true);
}

void OrbitalsClass::Btn_RemoveShell_OnClick(wxCommandEvent& event)
{
    wxListBox* listctr = sec30->GetListObject(_("AtomSpeciesList"));
    int item = listctr->GetSelection();
    if (item<0) {wxMessageBox(_("Select one of the species from the list."),_("Error")); return;}
    wxString SelectedAtomSpecies = listctr->GetString(item);
    
    wxCheckTree* treectr = sec30->GetTreeObject(_("Orbitals"));
    wxTreeItemId rootID = treectr->GetRootItem();
    wxTreeItemId selectedID = treectr->FindItemIn(rootID, SelectedAtomSpecies);
    int nChild = treectr->GetChildrenCount(selectedID, false);
    if (nChild < 1) return;
    wxString cnt = wxString::Format(wxT("%d"), nChild);
    wxTreeItemId shellID = treectr->FindItemIn(selectedID, _("Shell ") + cnt);
    treectr->Delete(shellID);
    treectr->Expand(rootID);
    treectr->Expand(selectedID);
    treectr->Update();
    treectr->Refresh(true);
}

