// IDNDemoDlg.cpp : implementation file
//

#include "stdafx.h"
#include "IDNDemo.h"
#include "IDNDemoDlg.h"
#include "Rigolusb.h"
#include <stdio.h>

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

#define MAX_DEVICE 10
#define MAXINFOLENGTH 100
unsigned char bTag=1;

void SendToUSB(CString cmd);
unsigned char* Read_USB( void );

/////////////////////////////////////////////////////////////////////////////
// CAboutDlg dialog used for App About

class CAboutDlg : public CDialog
{
public:
	CAboutDlg();

// Dialog Data
	//{{AFX_DATA(CAboutDlg)
	enum { IDD = IDD_ABOUTBOX };
	//}}AFX_DATA

	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CAboutDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	//{{AFX_MSG(CAboutDlg)
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

CAboutDlg::CAboutDlg() : CDialog(CAboutDlg::IDD)
{
	//{{AFX_DATA_INIT(CAboutDlg)
	//}}AFX_DATA_INIT
}

void CAboutDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CAboutDlg)
	//}}AFX_DATA_MAP
}

BEGIN_MESSAGE_MAP(CAboutDlg, CDialog)
	//{{AFX_MSG_MAP(CAboutDlg)
		// No message handlers
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CIDNDemoDlg dialog

CIDNDemoDlg::CIDNDemoDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CIDNDemoDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CIDNDemoDlg)
	m_receive = _T("");
	//}}AFX_DATA_INIT
	// Note that LoadIcon does not require a subsequent DestroyIcon in Win32
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

void CIDNDemoDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CIDNDemoDlg)
	DDX_Control(pDX, IDC_COMBO1, m_combox);
	DDX_Text(pDX, IDC_EDIT1, m_receive);
	//}}AFX_DATA_MAP
}

BEGIN_MESSAGE_MAP(CIDNDemoDlg, CDialog)
	//{{AFX_MSG_MAP(CIDNDemoDlg)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_CBN_EDITCHANGE(IDC_COMBO1, OnEditchangeCombo1)
	ON_BN_CLICKED(IDC_BUTTON1, OnButton1)
	ON_WM_DESTROY()
	ON_EN_CHANGE(IDC_EDIT1, OnChangeEdit1)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CIDNDemoDlg message handlers

BOOL CIDNDemoDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	// Add "About..." menu item to system menu.

	// IDM_ABOUTBOX must be in the system command range.
	ASSERT((IDM_ABOUTBOX & 0xFFF0) == IDM_ABOUTBOX);
	ASSERT(IDM_ABOUTBOX < 0xF000);

	CMenu* pSysMenu = GetSystemMenu(FALSE);
	if (pSysMenu != NULL)
	{
		CString strAboutMenu;
		strAboutMenu.LoadString(IDS_ABOUTBOX);
		if (!strAboutMenu.IsEmpty())
		{
			pSysMenu->AppendMenu(MF_SEPARATOR);
			pSysMenu->AppendMenu(MF_STRING, IDM_ABOUTBOX, strAboutMenu);
		}
	}

	// Set the icon for this dialog.  The framework does this automatically
	//  when the application's main window is not a dialog
	SetIcon(m_hIcon, TRUE);			// Set big icon
	SetIcon(m_hIcon, FALSE);		// Set small icon
	
	// TODO: Add extra initialization here
	InitFunction();

	return TRUE;  // return TRUE  unless you set the focus to a control
}

void CIDNDemoDlg::OnSysCommand(UINT nID, LPARAM lParam)
{
	if ((nID & 0xFFF0) == IDM_ABOUTBOX)
	{
		CAboutDlg dlgAbout;
		dlgAbout.DoModal();
	}
	else
	{
		CDialog::OnSysCommand(nID, lParam);
	}
}

// If you add a minimize button to your dialog, you will need the code below
//  to draw the icon.  For MFC applications using the document/view model,
//  this is automatically done for you by the framework.

void CIDNDemoDlg::OnPaint() 
{
	if (IsIconic())
	{
		CPaintDC dc(this); // device context for painting

		SendMessage(WM_ICONERASEBKGND, (WPARAM) dc.GetSafeHdc(), 0);

		// Center icon in client rectangle
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		// Draw the icon
		dc.DrawIcon(x, y, m_hIcon);
	}
	else
	{
		CDialog::OnPaint();
	}
}

// The system calls this to obtain the cursor to display while the user drags
//  the minimized window.
HCURSOR CIDNDemoDlg::OnQueryDragIcon()
{
	return (HCURSOR) m_hIcon;
}

void CIDNDemoDlg::OnEditchangeCombo1() 
{
	// TODO: Add your control notification handler code here
	m_combox.AddString("COM1");
}

void CIDNDemoDlg::OnButton1() 
{
	// TODO: Add your control notification handler code here
	
}

void CIDNDemoDlg::OnOK() 
{
	// TODO: Add extra validation here
	unsigned char* readInfo;
	CString strInput;
	//char* stringTemp;
	CString s,strTemp;

	m_combox.GetLBText(m_combox.GetCurSel(),strTemp); 
   
    strInput = (char *)(LPCTSTR)strTemp;  

    SendToUSB(strInput);
	readInfo = Read_USB();

	//�������ʾ����
	UpdateData (TRUE);  

	m_receive = readInfo;

	UpdateData (FALSE);

}

//������������
void SendToUSB(CString cmd)
{
	int temp,j;
	CString cmdstr;
	cmdstr = cmd;
	unsigned char * strInput;
    
	//RigolTMCUsb_UI.dll�е�GetDeviceNum��WriteUSB����
	HMODULE module = LoadLibrary("RigolTMCUsb_UI.dll");

    typedef bool (CALLBACK* pWrite)(unsigned long, unsigned char,unsigned char,unsigned long,unsigned char*);
    typedef int (CALLBACK* pGetDeviceNum)();
	
	temp = cmdstr.GetLength();
	strInput = (unsigned char*)cmdstr.GetBuffer(10);

	if(module)
	{
		pGetDeviceNum GetDevice = (pGetDeviceNum)GetProcAddress(module,"GetTMCDeviceNum");
		pWrite Write = (pWrite)GetProcAddress(module,"WriteUSB");
        
        j = GetDevice();
        
		if( j > 0 )
		{
			//��ʾ������������
			//�ӿڵĶ�����Rigolusb.h��ע��	
			Write(0,1,1,temp,strInput);
			
		}
	}
}

//��ʾ������ȡ����ֵ
unsigned char* Read_USB( void )
{
	unsigned long infoSize;
    int len=100;
	//���ٿռ䣬���ڴ��ʾ�����ķ���ֵ
    unsigned char* strInput = new unsigned char [len];
	CString str;

	//RigolTMCUsb_UI.dll�е�ReadUSB��WriteUSB����
	HMODULE module = LoadLibrary("RigolTMCUsb_UI.dll");

    typedef bool (CALLBACK* pWrite)(unsigned long, unsigned char,unsigned char,unsigned long,unsigned char*);
    typedef bool (CALLBACK* pRead)(unsigned long,unsigned long*,unsigned char*);

	if(module)
	{
		pWrite Write = (pWrite)GetProcAddress(module,"WriteUSB");
		pRead Read = (pRead)GetProcAddress(module,"ReadUSB");
       
		//��ʾ��������ȡֵ������ʹ��������Դ�ʾ���������ֵ
		Write(0,2,2,len,0);
        //��ʾ�����ж�ȡ����ֵ
		Read(0,&infoSize,strInput);
	}
	//���ٿռ����ڴ��ʾ��������Ч����ֵ
	unsigned char *buffer = new unsigned char[infoSize];

	memcpy(buffer,strInput,infoSize);

    buffer[infoSize] = 0;
	delete []strInput;

	//���ش�ʾ������õ��ַ���
	return buffer;
}


void CIDNDemoDlg::InitFunction()
{
	hLib = NULL;
	pReadUsb = NULL;
	pWriteUsb = NULL;
	
	hLib = (HMODULE)LoadLibrary("RigolTMCUsb_UI.dll");

	//���뺯��ָ��
	pWriteUsb= (P_WriteUSB)GetProcAddress(hLib,"WriteUSB");
	pReadUsb= (P_ReadUSB)GetProcAddress(hLib,"ReadUSB");
	
}

void CIDNDemoDlg::DeInitFunction()
{
	if( hLib!=NULL )
	{
		FreeLibrary( hLib );
		hLib = NULL;
	}
}

void CIDNDemoDlg::OnDestroy() 
{
	CDialog::OnDestroy();
	
	// TODO: Add your message handler code here
	DeInitFunction();
}

void CIDNDemoDlg::OnChangeEdit1() 
{
	// TODO: If this is a RICHEDIT control, the control will not
	// send this notification unless you override the CDialog::OnInitDialog()
	// function and call CRichEditCtrl().SetEventMask()
	// with the ENM_CHANGE flag ORed into the mask.
	
	// TODO: Add your control notification handler code here
	
}
