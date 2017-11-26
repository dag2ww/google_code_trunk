// IDNDemo.h : main header file for the IDNDEMO application
//

#if !defined(AFX_IDNDEMO_H__F6FEAD60_F2B7_4B09_8A6A_C19C7B824F2E__INCLUDED_)
#define AFX_IDNDEMO_H__F6FEAD60_F2B7_4B09_8A6A_C19C7B824F2E__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#ifndef __AFXWIN_H__
	#error include 'stdafx.h' before including this file for PCH
#endif

#include "resource.h"		// main symbols

/////////////////////////////////////////////////////////////////////////////
// CIDNDemoApp:
// See IDNDemo.cpp for the implementation of this class
//

class CIDNDemoApp : public CWinApp
{
public:
	CIDNDemoApp();

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CIDNDemoApp)
	public:
	virtual BOOL InitInstance();
	//}}AFX_VIRTUAL

// Implementation

	//{{AFX_MSG(CIDNDemoApp)
		// NOTE - the ClassWizard will add and remove member functions here.
		//    DO NOT EDIT what you see in these blocks of generated code !
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};


/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_IDNDEMO_H__F6FEAD60_F2B7_4B09_8A6A_C19C7B824F2E__INCLUDED_)
