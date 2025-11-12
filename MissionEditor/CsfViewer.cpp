#include "stdafx.h"
#include "CsfViewer.h"
#include "variables.h"
#include "functions.h"

BEGIN_MESSAGE_MAP(CCsfViewer, CDialog)
    ON_WM_CLOSE()
    ON_BN_CLICKED(Controls::Reload, onReload)
    ON_CBN_KILLFOCUS(IDC_CSF_VIEW_SELECTED, onEditchangeSearch)
    ON_NOTIFY(LVN_ITEMCHANGED, IDC_CSF_VIEW_LIST, onViewerSelectedChange)
    ON_NOTIFY(NM_DBLCLK, IDC_CSF_VIEW_LIST, OnListDblClk)
END_MESSAGE_MAP()

#if 0
LRESULT CALLBACK CCsfViewer::ListViewSubclassProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    //return DarkTheme::MyCallWindowProcA(g_pOriginalListViewProc, hWnd, uMsg, wParam, lParam);
}
#endif

CCsfViewer::CCsfViewer(CWnd* pParent) 
    : CDialog(CCsfViewer::IDD, pParent)
{
}
CCsfViewer::~CCsfViewer()
{
}

BOOL CCsfViewer::OnInitDialog()
{
    if (!CDialog::OnInitDialog()) {
        return FALSE;
    }

    translateUI();

    if (!m_selectedCSFLabel.IsEmpty()) {
        m_searchEdit.SetWindowText(m_selectedCSFLabel);
    }
    onEditchangeSearch();

    return TRUE;  // return TRUE unless you set the focus to a control
}

void CCsfViewer::DoDataExchange(CDataExchange* pDX)
{
    CDialog::DoDataExchange(pDX);
    DDX_Control(pDX, Controls::CSFViewer, m_stringList);
    DDX_Control(pDX, Controls::ItemDetailViewer, m_richEditCtrl);
    DDX_Control(pDX, Controls::SetLabel, m_selectedLabel);
    DDX_Control(pDX, Controls::SelectedCSF, m_searchEdit);
}

void CCsfViewer::translateUI()
{
    SetWindowText(GetLanguageStringACP("CsfViewerTitle"));
    GetDlgItem(1003)->SetWindowText(GetLanguageStringACP("CsfViewerSearchLabelText"));
    GetDlgItem(1010)->SetWindowText(GetLanguageStringACP("CsfViewerDescription1"));
    GetDlgItem(1012)->SetWindowText(GetLanguageStringACP("CsfViewerSetLabelName"));
    GetDlgItem(Controls::Reload)->SetWindowText(GetLanguageStringACP("CsfViewerReload"));

    SetDlgItemText(IDOK, GetLanguageStringACP("OK"));
    //SetDlgItemText(IDCANCEL, GetLanguageStringACP("Cancel")); // TODO: add cancel
    m_stringList.SetExtendedStyle(LVS_EX_FULLROWSELECT);
    m_richEditCtrl.SetReadOnly();

    //ExtraWindow::SetEditControlFontSize(m_richEditCtrl, 1.4f, true);
    CFont font;
    font.CreatePointFont(100, _T("Tahoma"));
    m_richEditCtrl.SetFont(&font);

#if 0
    if (ExtConfigs::EnableDarkMode)
    {
        ::SendMessage(m_richEditCtrl, EM_SETBKGNDCOLOR, (WPARAM)FALSE, (LPARAM)RGB(32, 32, 32));
        CHARFORMAT cf = { 0 };
        cf.cbSize = sizeof(cf);
        cf.dwMask = CFM_COLOR;
        cf.crTextColor = RGB(220, 220, 220);
        ::SendMessage(m_richEditCtrl, EM_SETCHARFORMAT, SCF_ALL, (LPARAM)&cf);
        ::SendMessage(hCSFViewer, LVM_SETTEXTBKCOLOR, 0, RGB(32, 32, 32));
        ::SendMessage(hCSFViewer, LVM_SETTEXTCOLOR, 0, RGB(220, 220, 220));

        DarkTheme::SubclassListViewHeader(hCSFViewer);
        g_pOriginalListViewProc = (WNDPROC)GetWindowLongPtr(hCSFViewer, GWLP_WNDPROC);
        if (g_pOriginalListViewProc)
        {
            SetWindowLongPtr(hCSFViewer, GWLP_WNDPROC, (LONG_PTR)ListViewSubclassProc);
        }
        InvalidateRect(hCSFViewer, NULL, TRUE);
    }

    resetControls();
#endif
}

void CCsfViewer::OnClose()
{
    EndDialog(IDCANCEL);
}

void CCsfViewer::onReload()
{
    last_succeeded_operation = 9;

    resetControls();
    // this is modal window, no need to update others
    //((CFinalSunDlg*)theApp.m_pMainWnd)->UpdateDialogs(TRUE);
}

void CCsfViewer::SetSelectedString(CString str)
{
    if (str == m_selectedCSFLabel) {
        return;
    }
    // NOTE: it does not reset dialog, call applySearch to make it work
    m_selectedCSFLabel = str;
    m_selectedCSFContent = ""; // reset this value to avoid unwanted cache
}

void CCsfViewer::resetControls()
{
    applySearch();
    m_richEditCtrl.SetWindowText("");
    m_selectedLabel.SetWindowText("");
    m_selectedCSFLabel = "";

    CString txt;
    m_searchEdit.GetWindowText(txt);
    if (!txt.IsEmpty()) {
        onEditchangeSearch();
    }
}

BOOL CCsfViewer::PreTranslateMessage(MSG* pMsg)
{
    int ret = -1;
    if (pMsg->message == WM_KEYDOWN) {
        ret = onMessageKeyDown(pMsg);
    }

    return ret < 0 ? this->CDialog::PreTranslateMessage(pMsg) : ret;
}

BOOL CCsfViewer::onMessageKeyDown(MSG* pMsg)
{
    switch (pMsg->wParam) {
    default:
        return -1;
    case VK_RETURN:
    {
        switch (::GetDlgCtrlID(pMsg->hwnd)) {
        default:
            break;// never exist window (default -1) even nothing did
        case Controls::Search: this->onEditchangeSearch();
            break;
        }
    }
    }
    return TRUE;
}

void CCsfViewer::OnListDblClk(NMHDR* pNMHDR, LRESULT* pResult)
{
    LPNMITEMACTIVATE pNMItemActivate = reinterpret_cast<LPNMITEMACTIVATE>(pNMHDR);

    int nItem = pNMItemActivate->iItem;
    if (nItem >= 0)
    {
        int nSelected = m_stringList.GetNextItem(-1, LVNI_SELECTED);
        if (nSelected == -1) {
            return;
        }
        CString selectedText = m_stringList.GetItemText(nSelected, 0);

        if (selectedText.IsEmpty()) {
            m_richEditCtrl.SetWindowText("");
            m_selectedLabel.SetWindowText("");
            m_selectedCSFLabel = "";
            return;
        }

		CString value = "";
        auto const it = AllStrings.find(selectedText);
        if (it == AllStrings.end()) {
            return;
        }

        m_richEditCtrl.SetWindowText(it->second.cString);
        m_selectedLabel.SetWindowText(selectedText);

        m_selectedCSFLabel = selectedText;
        m_selectedCSFContent = it->second.cString;
    }
    OnOK();

    *pResult = 0;
}

void CCsfViewer::onViewerSelectedChange(NMHDR* pNMHDR, LRESULT* pResult)
{
    int nSelected = m_stringList.GetNextItem(-1, LVNI_SELECTED);
    if (nSelected == -1) {
        return;
    }

    CString selectedText = m_stringList.GetItemText(nSelected, 0);

    if (selectedText.IsEmpty()) {
        m_richEditCtrl.SetWindowText("");
        m_selectedLabel.SetWindowText("");
        m_selectedCSFLabel = "";
        return;
    }

    CString value = "";
    auto const it = AllStrings.find(selectedText);
    if (it == AllStrings.end()) {
        return;
    }

    m_richEditCtrl.SetWindowText(it->second.cString);
    m_selectedLabel.SetWindowText(selectedText);

    m_selectedCSFLabel = selectedText;
    m_selectedCSFContent = it->second.cString;
    *pResult = 0;
}

void CCsfViewer::OnOK()
{
    if (m_selectedCSFLabel.IsEmpty()) {
        EndDialog(IDCANCEL);
        return;
    }
    EndDialog(IDOK);
}

void CCsfViewer::displayCSFContent(const TranslationMap& csfMap, const RowSearchHandler handler)
{
    m_stringList.DeleteAllItems();
    while (m_stringList.DeleteColumn(0));

    CString txt;
    txt = TranslateStringACP("CsfViewerColumnLabel");
    LVCOLUMN lvColumn = { 0 };
    lvColumn.mask = LVCF_TEXT | LVCF_WIDTH | LVCF_SUBITEM;
    lvColumn.pszText = const_cast<LPSTR>(txt.operator LPCSTR());
    lvColumn.cx = 100;
    m_stringList.InsertColumn(0, &lvColumn);

    txt = TranslateStringACP("CsfViewerColumnText");
    lvColumn.cx = 400;
    lvColumn.pszText = const_cast<LPSTR>(txt.operator LPCSTR());

    m_stringList.InsertColumn(1, &lvColumn);

    LVITEM lvItem = { 0 };
    lvItem.mask = LVIF_TEXT;

    int i = 0;
    for (auto const& [label, content] : csfMap) {
        if (!handler(label) && !handler(content.cString)) {
            continue;
        }

        lvItem.mask = LVIF_TEXT;
        lvItem.iItem = i;
        lvItem.iSubItem = 0;
        lvItem.pszText = const_cast<LPSTR>(label.operator LPCSTR());
        m_stringList.InsertItem(&lvItem);

        m_stringList.SetItemText(i, 1, const_cast<LPSTR>(content.cString.operator LPCSTR()));

        i++;
    }

    m_stringList.SetColumnWidth(1, LVSCW_AUTOSIZE_USEHEADER);
}

void CCsfViewer::onEditchangeSearch()
{
    CString buffer;
    m_searchEdit.GetWindowText(buffer);
    applySearch(buffer);
}

void CCsfViewer::applySearch(const CString keyword)
{
    displayCSFContent(AllStrings, [&keyword](const CString& content) {
        return keyword.IsEmpty() || content.Find(keyword) >= 0;
    });
}
