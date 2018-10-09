#include "GraphClass.h"
#include <wx/msgdlg.h>

wxString glGetwxString(GLenum name);

GraphClass::GraphClass(wxWindow* parent, int Dim)
    : GraphBaseClass(parent)
{
    int stereoAttribList[] = { WX_GL_RGBA, WX_GL_DOUBLEBUFFER, WX_GL_STEREO, 0 };
    bool stereoWindow = false;
    glc = new MyGLCanvas(this, Dim, stereoWindow ? stereoAttribList : NULL);
    
    TopPanel->GetContainingSizer()->Add(glc,wxSizerFlags().Proportion(1).Expand().Border(wxALL, 0));
    // test IsDisplaySupported() function:
    static const int attribs[] = { WX_GL_RGBA, WX_GL_DOUBLEBUFFER, 0 };
    wxLogStatus("Double-buffered display %s supported",
                wxGLCanvas::IsDisplaySupported(attribs) ? "is" : "not");

    if ( stereoWindow )
    {
        const wxString vendor = glGetwxString(GL_VENDOR).Lower();
        const wxString renderer = glGetwxString(GL_RENDERER).Lower();
        //if ( vendor.find("nvidia") != wxString::npos &&
            //    renderer.find("quadro") == wxString::npos )
            //ShowFullScreen(true);
    }
}

GraphClass::~GraphClass()
{
    DiscardAtomicStructure();
}

void GraphClass::CreateAtomicStructure(Sec30* sec30Var, bool IsNewAllocate)
{
    bool isCountingMode = IsNewAllocate;
    sec30 = sec30Var;
    int nUnitcellAtoms = 0;
    sec30->GetVar(_("nAtoms[0]"),nUnitcellAtoms);
    
    int kind[nUnitcellAtoms];
    double XArray[nUnitcellAtoms];
    double YArray[nUnitcellAtoms];
    double ZArray[nUnitcellAtoms];
    
    for (int icell=0; icell<nUnitcellAtoms; icell++)
    {
        sec30->GetVar(_("KABC_Coords"), icell, 0, kind[icell]);
        sec30->GetVar(_("XYZ_Coords"), icell, 0, XArray[icell]);
        sec30->GetVar(_("XYZ_Coords"), icell, 1, YArray[icell]);
        sec30->GetVar(_("XYZ_Coords"), icell, 2, ZArray[icell]);
    }
    
    double a[3],b[3],c[3];
    sec30->GetVar(_("a[0]"), a[0]);
    sec30->GetVar(_("a[1]"), a[1]);
    sec30->GetVar(_("a[2]"), a[2]);
    sec30->GetVar(_("b[0]"), b[0]);
    sec30->GetVar(_("b[1]"), b[1]);
    sec30->GetVar(_("b[2]"), b[2]);
    sec30->GetVar(_("c[0]"), c[0]);
    sec30->GetVar(_("c[1]"), c[1]);
    sec30->GetVar(_("c[2]"), c[2]);

    int ma[2],mb[2],mc[2],TBl,TBm,TBn;
    sec30->GetVar(_("ma[0]"), ma[0]);
    sec30->GetVar(_("ma[1]"), ma[1]);
    sec30->GetVar(_("mb[0]"), mb[0]);
    sec30->GetVar(_("mb[1]"), mb[1]);
    sec30->GetVar(_("mc[0]"), mc[0]);
    sec30->GetVar(_("mc[1]"), mc[1]);
    sec30->GetVar(_("TBl[0]"), TBl);
    sec30->GetVar(_("TBm[0]"), TBm);
    sec30->GetVar(_("TBn[0]"), TBn);
    
    bool CustomViewmode,TBViewmode,TBEssentialViewmode,WorkingViewmode;
    sec30->GetRadioVar(_("CustomViewmode[0]"),CustomViewmode);
    sec30->GetRadioVar(_("TBViewmode[0]"),TBViewmode);
    sec30->GetRadioVar(_("TBEssentialViewmode[0]"),TBEssentialViewmode);
    sec30->GetCheckVar(_("WorkingViewmode[0]"), WorkingViewmode);
    
    wxListBox* ess = sec30->GetListObject(_("EssentialUnitcellList"));
    bool isSellected = false;
    int essselectedind = ess->GetSelection();
    int isellected = 0;
    int jsellected = 0;
    int ksellected = 0;
    if (essselectedind >= 0)
    {
        isSellected = true;
        wxString itemtxt = ess->GetString(essselectedind);
        GetUnitcellInfo(itemtxt, isellected, jsellected, ksellected);
    }
    
    FindEssentials();
    SetWorkingList(isellected, jsellected, ksellected);
    
    int lmin,lmax,mmin,mmax,nmin,nmax;
    if (CustomViewmode)
    {
        lmin = ma[0]; lmax = ma[1];
        mmin = mb[0]; mmax = mb[1];
        nmin = mc[0]; nmax = mc[1];
    }
    else if (TBViewmode || TBEssentialViewmode)
    {
        lmin = -TBl; lmax = TBl;
        mmin = -TBm; mmax = TBm;
        nmin = -TBn; nmax = TBn;
    }

    if (lmin>lmax) {int dummy = lmax; lmax = lmin; lmin = dummy;}
    if (mmin>mmax) {int dummy = mmax; mmax = mmin; mmin = dummy;}
    if (nmin>nmax) {int dummy = nmax; nmax = nmin; nmin = dummy;}
    
    /************************************************/
    if (!isCountingMode)
    {
        int ndoublearr[11]={nShowingAtoms,nShowingAtoms,nShowingAtoms,nShowingAtoms,nShowingBonds,nShowingBonds,nShowingBonds,nShowingBonds,nShowingBonds,nShowingBonds,nShowingBonds};
        int nintarr[6]={nShowingAtoms,nShowingAtoms,nShowingAtoms,nShowingBonds,nShowingBonds,nShowingBonds};
        glc->CreateDoubleArray(11,ndoublearr);
        glc->CreateIntArray(6,nintarr);
    }
    else
    {
        nShowingAtoms = 0;
        nShowingBonds = 0;
    }
    /************************************************/
    if ( !WorkingViewmode && (CustomViewmode || TBViewmode))
    {
        /*************atoms************/
        int iAtom = -1;
        for (int i=lmax; i>=lmin; i--)
            for (int j=mmax; j>=mmin; j--)
                for (int k=nmax; k>=nmin; k--)
                    for (int icell=0; icell<nUnitcellAtoms; icell++)
                    {
                        iAtom++;
                        if (!isCountingMode)
                        {
                            glc->DoubleArray[0][iAtom] = XArray[icell] + i*a[0] + j*b[0] + k*c[0];
                            glc->DoubleArray[1][iAtom] = YArray[icell] + i*a[1] + j*b[1] + k*c[1];
                            glc->DoubleArray[2][iAtom] = ZArray[icell] + i*a[2] + j*b[2] + k*c[2];
                            glc->DoubleArray[3][iAtom] = GetAtomRadius(kind[icell]);
                            int atomKindindex = kind[icell];
                            if (atomKindindex < 1) atomKindindex = 1;
                            if (atomKindindex > 118) atomKindindex = 118;
                            wxColourPickerCtrl* cctrl = sec30->GetColorObject(_("AColor") + wxString::Format(wxT("%d"),atomKindindex));
                            wxColour c = cctrl->GetColour();
                            int r = c.Red();
                            int g = c.Green();
                            int b = c.Blue();
                            glc->IntArray[0][iAtom] = r;
                            glc->IntArray[1][iAtom] = g;
                            glc->IntArray[2][iAtom] = b;
                        }
                        else
                            nShowingAtoms = iAtom + 1;
                    }
                    
        /*************bonds************/
        int iBond = -1;
        for (int i=lmax; i>=lmin; i--)
            for (int j=mmax; j>=mmin; j--)
                for (int k=nmax; k>=nmin; k--)
                {
                    for (int icell=0; icell<nUnitcellAtoms; icell++)
                    {
                        int iMyess, jMyess, kMyess;
                        std::list<int>::iterator ii = EssentialListi.begin();
                        std::list<int>::iterator ij = EssentialListj.begin();
                        std::list<int>::iterator ik = EssentialListk.begin();
                        
                        for (int s=0; s!=EssentialListi.size(); s++)
                        {
                            /**Relatively, My Essential index in list**/
                            iMyess = *ii++;
                            jMyess = *ij++;
                            kMyess = *ik++;
                            /******************************************/
                            wxCheckTree* treectr = sec30->GetTreeObject(_("Bonds"));
                            wxTreeItemId rootID = treectr->GetRootItem();
                            wxString cellItem = wxString::Format(wxT("cell(0,0,0)-cell(%d,%d,%d)"), iMyess, jMyess, kMyess);
                            wxTreeItemId lmnID = treectr->FindItemIn(rootID,cellItem);
                            
                            if (lmnID)
                            {
                                wxTreeItemIdValue cookie;
                                wxTreeItemId bond = treectr->GetFirstChild(lmnID, cookie);
                                bool isBondInRange = (iMyess + i <= lmax && iMyess + i >= lmin) && (jMyess + j <= mmax && jMyess + j >= mmin) && (kMyess + k <= nmax && kMyess + k >= nmin);
                                while (bond.IsOk() && isBondInRange)
                                {
                                    iBond++;
                                    if (!isCountingMode)
                                    {
                                        wxString BondInfo = treectr->GetItemText(bond);
                                        int iAtomIndex,nShellIndex,jAtomIndex,mShellIndex,bondtype;
                                        GetBondInfo(BondInfo, iAtomIndex,nShellIndex,jAtomIndex,mShellIndex,bondtype);
                                        //wxMessageBox(wxString::Format(wxT("%d,%d,%d,%d,%d"),iAtomIndex,nShellIndex,jAtomIndex,mShellIndex,bondtype));
                                        bool isBondValid = (iAtomIndex >= 0 && iAtomIndex < nUnitcellAtoms) && (jAtomIndex >= 0 && jAtomIndex < nUnitcellAtoms);
                                        if (isBondValid)
                                        {
                                            glc->DoubleArray[4][iBond] = XArray[iAtomIndex] + i*a[0] + j*b[0] + k*c[0];
                                            glc->DoubleArray[5][iBond] = YArray[iAtomIndex] + i*a[1] + j*b[1] + k*c[1];
                                            glc->DoubleArray[6][iBond] = ZArray[iAtomIndex] + i*a[2] + j*b[2] + k*c[2];
                                            glc->DoubleArray[7][iBond] = XArray[jAtomIndex] + (iMyess + i)*a[0] + (jMyess + j)*b[0] + (kMyess + k)*c[0];
                                            glc->DoubleArray[8][iBond] = YArray[jAtomIndex] + (iMyess + i)*a[1] + (jMyess + j)*b[1] + (kMyess + k)*c[1];
                                            glc->DoubleArray[9][iBond] = ZArray[jAtomIndex] + (iMyess + i)*a[2] + (jMyess + j)*b[2] + (kMyess + k)*c[2];
                                            double radius = GetAtomRadius(kind[iAtomIndex]);
                                            double radius2 = GetAtomRadius(kind[jAtomIndex]);
                                            if (radius > radius2) radius = radius2;
                                            glc->DoubleArray[10][iBond] = radius / 2.3;
                                            wxColourPickerCtrl* cctrl = sec30->GetColorObject(_("BColor") + wxString::Format(wxT("%d"),bondtype));
                                            wxColour c = cctrl->GetColour();
                                            int r = c.Red();
                                            int g = c.Green();
                                            int b = c.Blue();
                                            glc->IntArray[3][iBond] = r;
                                            glc->IntArray[4][iBond] = g;
                                            glc->IntArray[5][iBond] = b;
                                        }
                                    }
                                    else
                                        nShowingBonds = iBond + 1;
                                    bond = treectr->GetNextSibling(bond);
                                }
                            }
                        }
                    }
                }
    }
    
    if ((!WorkingViewmode && TBEssentialViewmode) || WorkingViewmode)
    {
        int i, j ,k;
        int nList0 = 0;
        /*************atoms************/
        std::list<int>::iterator ii0, ij0, ik0;
        if (!WorkingViewmode)
        {
            ii0 = EssentialListi.begin();
            ij0 = EssentialListj.begin();
            ik0 = EssentialListk.begin();
            nList0 = EssentialListi.size();
        }
        else
        {
            ii0 = WorkingListi.begin();
            ij0 = WorkingListj.begin();
            ik0 = WorkingListk.begin();
            nList0 = WorkingListi.size();
        }
        
        int iAtom = -1;
        for (int s0=0; s0!=nList0; s0++)
        {
            i = *ii0++;
            j = *ij0++;
            k = *ik0++;
            for (int icell=0; icell<nUnitcellAtoms; icell++)
            {
                iAtom++;
                if (!isCountingMode)
                {
                    glc->DoubleArray[0][iAtom] = XArray[icell] + i*a[0] + j*b[0] + k*c[0];
                    glc->DoubleArray[1][iAtom] = YArray[icell] + i*a[1] + j*b[1] + k*c[1];
                    glc->DoubleArray[2][iAtom] = ZArray[icell] + i*a[2] + j*b[2] + k*c[2];
                    glc->DoubleArray[3][iAtom] = GetAtomRadius(kind[icell]);
                    int atomKindindex = kind[icell];
                    if (atomKindindex < 1) atomKindindex = 1;
                    if (atomKindindex > 118) atomKindindex = 118;
                    wxColourPickerCtrl* cctrl = sec30->GetColorObject(_("AColor") + wxString::Format(wxT("%d"),atomKindindex));
                    wxColour c = cctrl->GetColour();
                    int r = c.Red();
                    int g = c.Green();
                    int b = c.Blue();
                    glc->IntArray[0][iAtom] = r;
                    glc->IntArray[1][iAtom] = g;
                    glc->IntArray[2][iAtom] = b;
                }
                else
                    nShowingAtoms = iAtom + 1;
            }
        }
        
        /*************bonds************/
        nList0 = 0;
        if (!WorkingViewmode)
        {
            ii0 = EssentialListi.begin();
            ij0 = EssentialListj.begin();
            ik0 = EssentialListk.begin();
            nList0 = EssentialListi.size();
        }
        else
        {
            ii0 = WorkingListi.begin();
            ij0 = WorkingListj.begin();
            ik0 = WorkingListk.begin();
            nList0 = WorkingListi.size();
        }
        
        int iBond = -1;
        for (int s0=0; s0!=nList0; s0++)
        {
            i = *ii0++;
            j = *ij0++;
            k = *ik0++;
            for (int icell=0; icell<nUnitcellAtoms; icell++)
            {
                int iMyess, jMyess, kMyess;
                std::list<int>::iterator ii,ij,ik;
                int nList = 0;
                if (!WorkingViewmode)
                {
                    ii = EssentialListi.begin();
                    ij = EssentialListj.begin();
                    ik = EssentialListk.begin();
                    nList = EssentialListi.size();
                }
                else
                {
                    ii = WorkingListi.begin();
                    ij = WorkingListj.begin();
                    ik = WorkingListk.begin();
                    nList = WorkingListi.size();
                }
                
                for (int s=0; s!=nList; s++)
                {
                    /**Relatively, My Essential index in list**/
                    iMyess = *ii++;
                    jMyess = *ij++;
                    kMyess = *ik++;
                    /******************************************/
                    wxCheckTree* treectr = sec30->GetTreeObject(_("Bonds"));
                    wxTreeItemId rootID = treectr->GetRootItem();
                    wxString cellItem = wxString::Format(wxT("cell(0,0,0)-cell(%d,%d,%d)"), iMyess, jMyess, kMyess);
                    wxTreeItemId lmnID = treectr->FindItemIn(rootID,cellItem);
                    
                    if (lmnID)
                    {
                        wxTreeItemIdValue cookie;
                        wxTreeItemId bond = treectr->GetFirstChild(lmnID, cookie);
                        bool isBondInRange = (iMyess + i <= lmax && iMyess + i >= lmin) && (jMyess + j <= mmax && jMyess + j >= mmin) && (kMyess + k <= nmax && kMyess + k >= nmin);
                        while (bond.IsOk() && isBondInRange)
                        {
                            iBond++;
                            if (!isCountingMode)
                            {
                                wxString BondInfo = treectr->GetItemText(bond);
                                int iAtomIndex,nShellIndex,jAtomIndex,mShellIndex,bondtype;
                                GetBondInfo(BondInfo, iAtomIndex,nShellIndex,jAtomIndex,mShellIndex,bondtype);
                                //wxMessageBox(wxString::Format(wxT("%d,%d,%d,%d,%d"),iAtomIndex,nShellIndex,jAtomIndex,mShellIndex,bondtype));
                                bool isBondValid = (iAtomIndex >= 0 && iAtomIndex < nUnitcellAtoms) && (jAtomIndex >= 0 && jAtomIndex < nUnitcellAtoms);
                                if (isBondValid)
                                {
                                    glc->DoubleArray[4][iBond] = XArray[iAtomIndex] + i*a[0] + j*b[0] + k*c[0];
                                    glc->DoubleArray[5][iBond] = YArray[iAtomIndex] + i*a[1] + j*b[1] + k*c[1];
                                    glc->DoubleArray[6][iBond] = ZArray[iAtomIndex] + i*a[2] + j*b[2] + k*c[2];
                                    glc->DoubleArray[7][iBond] = XArray[jAtomIndex] + (iMyess + i)*a[0] + (jMyess + j)*b[0] + (kMyess + k)*c[0];
                                    glc->DoubleArray[8][iBond] = YArray[jAtomIndex] + (iMyess + i)*a[1] + (jMyess + j)*b[1] + (kMyess + k)*c[1];
                                    glc->DoubleArray[9][iBond] = ZArray[jAtomIndex] + (iMyess + i)*a[2] + (jMyess + j)*b[2] + (kMyess + k)*c[2];
                                    double radius = GetAtomRadius(kind[iAtomIndex]);
                                    double radius2 = GetAtomRadius(kind[jAtomIndex]);
                                    if (radius > radius2) radius = radius2;
                                    glc->DoubleArray[10][iBond] = radius / 2.3;
                                    wxColourPickerCtrl* cctrl = sec30->GetColorObject(_("BColor") + wxString::Format(wxT("%d"),bondtype));
                                    wxColour c = cctrl->GetColour();
                                    int r = c.Red();
                                    int g = c.Green();
                                    int b = c.Blue();
                                    glc->IntArray[3][iBond] = r;
                                    glc->IntArray[4][iBond] = g;
                                    glc->IntArray[5][iBond] = b;
                                }
                            }
                            else
                                nShowingBonds = iBond + 1;
                            bond = treectr->GetNextSibling(bond);
                        }
                    }
                    /*******************************************/
                }
            }
        }
        
    }
    
   
    
    if (isCountingMode)
        CreateAtomicStructure(sec30, false);
    else
        glc->LoadToCanvas();
}

void GraphClass::GetBondInfo(const wxString& bondtextvar, int& i, int& n, int& j, int& m, int& bondtype)
{
    wxString bondtext = bondtextvar;
    bondtext.Replace(wxString(" "), wxString(""));
    bondtext.Replace(wxString("["), wxString(""));
    bondtext.Replace(wxString("]"), wxString(""));
    bondtext.Replace(wxString("("), wxString(""));
    bondtext.Replace(wxString(")"), wxString(""));
    bondtext.Replace(wxString("Bond"), wxString(""));
    bondtext.Replace(wxString("="), wxString(","));
    
    wxStringTokenizer tokenizer(bondtext, ",");
    tokenizer.GetNextToken();
    tokenizer.GetNextToken();
    long i0,n0,j0,m0,b0;
    tokenizer.GetNextToken().ToLong(&i0);
    tokenizer.GetNextToken().ToLong(&n0);
    tokenizer.GetNextToken();
    tokenizer.GetNextToken();
    tokenizer.GetNextToken().ToLong(&j0);
    tokenizer.GetNextToken().ToLong(&m0);
    tokenizer.GetNextToken().ToLong(&b0);
    i=i0-1;
    n=n0;
    j=j0-1;
    m=m0;
    bondtype=b0;
}

void GraphClass::GetUnitcellInfo(const wxString& unitcelltextvar, int& l, int& m, int& n)
{
    wxString txt = unitcelltextvar;
    txt.Replace(wxString("("), wxString(""));
    txt.Replace(wxString(")"), wxString(""));
    wxStringTokenizer tokenizer(txt, ",");
    long i0,j0,k0;
    tokenizer.GetNextToken().ToLong(&i0);
    tokenizer.GetNextToken().ToLong(&j0);
    tokenizer.GetNextToken().ToLong(&k0);
    l=i0;
    m=j0;
    n=k0;
}

void GraphClass::FindEssentials()
{
    int TBl,TBm,TBn,lmin,lmax,mmin,mmax,nmin,nmax;
    sec30->GetVar(_("TBl[0]"), TBl);
    sec30->GetVar(_("TBm[0]"), TBm);
    sec30->GetVar(_("TBn[0]"), TBn);
    lmin = -TBl; lmax = TBl;
    mmin = -TBm; mmax = TBm;
    nmin = -TBn; nmax = TBn;
    if (lmin>lmax) {int dummy = lmax; lmax = lmin; lmin = dummy;}
    if (mmin>mmax) {int dummy = mmax; mmax = mmin; mmin = dummy;}
    if (nmin>nmax) {int dummy = nmax; nmax = nmin; nmin = dummy;}
    
    EssentialListi.clear();
    EssentialListj.clear();
    EssentialListk.clear();
    if (lmin>lmax) {int dummy = lmax; lmax = lmin; lmin = dummy;}
    if (mmin>mmax) {int dummy = mmax; mmax = mmin; mmin = dummy;}
    if (nmin>nmax) {int dummy = nmax; nmax = nmin; nmin = dummy;}
    for (int i=lmax; i>=lmin; i--)
        for (int j=mmax; j>=mmin; j--)
            for (int k=nmax; k>=nmin; k--)
                isItNew(i,j,k);
}

void GraphClass::SetWorkingList(int l, int m, int n)
{
    WorkingListi.clear();
    WorkingListj.clear();
    WorkingListk.clear();
    std::list<int>::iterator ii = WorkingListi.begin();
    std::list<int>::iterator ij = WorkingListj.begin();
    std::list<int>::iterator ik = WorkingListk.begin();
    WorkingListi.insert(ii,0);
    WorkingListj.insert(ij,0);
    WorkingListk.insert(ik,0);
    if ( !(l==0 && m==0 && n==0) )
    {
        WorkingListi.insert(ii,l);
        WorkingListj.insert(ij,m);
        WorkingListk.insert(ik,n);
    }
}

void GraphClass::DiscardAtomicStructure()
{
    glc->DiscardDoubleArrays();
    glc->DiscardIntArrays();
}

bool GraphClass::isItNew(int i,int j,int k)
{
    std::list<int>::iterator ii = EssentialListi.begin();
    std::list<int>::iterator ij = EssentialListj.begin();
    std::list<int>::iterator ik = EssentialListk.begin();
    bool isOK = true;
    
    for (int s=0; s!=EssentialListi.size(); s++)
    {
        if (*ii == -i && *ij == -j && *ik == -k) {isOK = false; break;}
        ii++;ij++;ik++;
    }
    
    if (isOK)
    {
        EssentialListi.insert(ii,i);
        EssentialListj.insert(ij,j);
        EssentialListk.insert(ik,k);
    }
    return isOK;
}

bool GraphClass::isAtomIndexNew(int i,int j)
{
    std::list<int>::iterator ii = AtomIndexListi.begin();
    std::list<int>::iterator ij = AtomIndexListj.begin();
    bool isOK = true;
    
    for (int s=0; s!=AtomIndexListi.size(); s++)
    {
        if (*ii == i && *ij == j) {isOK = false; break;}
        ii++;ij++;
    }
    
    if (isOK)
    {
        AtomIndexListi.insert(ii,i);
        AtomIndexListj.insert(ij,j);
    }
    return isOK;
}

void GraphClass::graph_OnPaint(wxPaintEvent& event)
{
    
}


wxString glGetwxString(GLenum name)
{
    const GLubyte *v = glGetString(name);
    if ( v == 0 )
    {
        // The error is not important. It is GL_INVALID_ENUM.
        // We just want to clear the error stack.
        glGetError();

        return wxString();
    }

    return wxString((const char*)v);
}

double GraphClass::GetAtomRadius(int kind)
{
    double k=abs(kind);
    if (k < 1) k=1;
    return (10.0 + pow(log(abs(k) + 1),2.75)/2.0 )/100.0;
}