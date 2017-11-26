// IDNDemoDlg.h : header file
//

#if !defined(AFX_IDNDEMODLG_H__AF0D1941_288C_4B47_A850_EA93123CDA54__INCLUDED_)
#define AFX_IDNDEMODLG_H__AF0D1941_288C_4B47_A850_EA93123CDA54__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

/////////////////////////////////////////////////////////////////////////////
// CIDNDemoDlg dialog

typedef bool (*P_ReadUSB)(ULONG devIndex,
									 PULONG wLength,
								     PUCHAR pBuffer);
typedef bool (*P_WriteUSB)(ULONG devIndex,
									 UCHAR MsgID,
									 UCHAR bTag,
									 ULONG wLength,
								     PUCHAR pBuffer);

class CIDNDemoDlg : public CDialog
{
// Construction
public:
	CIDNDemoDlg(CWnd* pParent = NULL);	// standard constructor

// Dialog Data
	//{{AFX_DATA(CIDNDemoDlg)
	enum { IDD = IDD_IDNDEMO_DIALOG };
	CComboBox	m_combox;
	CString	m_receive;
	//}}AFX_DATA

	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CIDNDemoDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	HICON m_hIcon;

	// Generated message map functions
	//{{AFX_MSG(CIDNDemoDlg)
	virtual BOOL OnInitDialog();
	afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	afx_msg void OnEditchangeCombo1();
	afx_msg void OnButton1();
	virtual void OnOK();
	afx_msg void OnDestroy();
	afx_msg void OnChangeEdit1();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

private:
	void DeInitFunction();
	void InitFunction();
	P_WriteUSB pWriteUsb;
	P_ReadUSB pReadUsb;

	HMODULE hLib;

};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_IDNDEMODLG_H__AF0D1941_288C_4B47_A850_EA93123CDA54__INCLUDED_)
