#pragma once

#include <map>
#include <vector>
#include <string>
#include <regex>
#include <functional>
#include <afxrich.h>
#include "structs.h"

// A static window class
class CCsfViewer : public CDialog
{
public:
    static LRESULT CALLBACK ListViewSubclassProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

    enum { IDD = IDD_CSF_VIEWER };

    CCsfViewer(CWnd* pParent = nullptr);
    ~CCsfViewer();

    CString CSFLabelSelected() const { return m_selectedCSFLabel; }
    void SetSelectedString(CString str);

    CString CSFContentSelected() const { return m_selectedCSFContent; }

protected:
    enum Controls {
        SelectedCSF = IDC_CSF_VIEW_SELECTED,
        Search = IDC_CSF_VIEW_SELECTED,
        CSFViewer = IDC_CSF_VIEW_LIST,
        ItemDetailViewer = IDC_CSF_VIEW_RICH_EDT,
        SetLabel = IDC_CSF_VIEW_CUR_LABEL,
        Reload = IDC_CSF_VIEW_RELOAD,
    };

    using RowSearchHandler = std::function<bool(const CString&)>;

    virtual BOOL OnInitDialog() override;
    virtual void DoDataExchange(CDataExchange* pDX) override;
    virtual BOOL PreTranslateMessage(MSG* pMsg) override;
    virtual void OnOK() override;

    void translateUI();


    afx_msg void OnClose();
    afx_msg void onReload();
    void resetControls();
    void displayCSFContent(const TranslationMap& csfMap, const RowSearchHandler handler);
    void onEditchangeSearch();
    void applySearch(const CString keyword = {});
    void onViewerSelectedChange(NMHDR* pNMHDR, LRESULT* pResult);
    void onViewerDoubleClickApply(NMHDR* pNMHDR, LRESULT* pResult);

    BOOL onMessageKeyDown(MSG* pMsg);

    DECLARE_MESSAGE_MAP()

private:
    CRichEditCtrl m_richEditCtrl;
    CListCtrl m_stringList;
    CEdit m_selectedLabel;
    CEdit m_searchEdit;
    CString m_selectedCSFLabel;
    CString m_selectedCSFContent;
};

