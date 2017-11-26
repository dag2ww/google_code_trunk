// SDK_DEMODlg.cpp : implementation file
//

#include "stdafx.h"
#include "SDK_DEMO.h"
#include "SDK_DEMODlg.h"
#include "Mass_StorageApi.h"
#include "Protocol.h"



#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

#define TIMEOUT 10000
#define HSIZE 240
#define VSIZE 216
#define ImageX 60
#define ImageY 60

/************************************************************************/
/*                                                                      */
/************************************************************************/

inline void ui_polling()
{
	MSG msg;

	while ( PeekMessage(&msg,NULL,0,0,PM_REMOVE)) 
	{
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}
}


unsigned long  GetFileLength ( FILE * fileName)
{
     unsigned long pos = ftell(fileName);
     unsigned long len = 0;
   
     fseek ( fileName, 0L, SEEK_END );
     len = ftell ( fileName );
     fseek ( fileName, pos, SEEK_SET );
     return len;
}

void InvertImage(BYTE *OutBuf,int Width , int Height )
{
	int i,Line;
	long Offset,Offset1,dwCount;
	BYTE tempbuf[640];

	dwCount=(long)Width*Height;
	Line=Height/2;
	for(i=0;i<Line;i++)
    {
		Offset=(long)i*Width;
		Offset1=dwCount- Offset-Width;
		memcpy(tempbuf,OutBuf+Offset,Width);   
		memcpy(OutBuf+Offset,OutBuf+Offset1,Width);   
		memcpy(OutBuf+Offset1,tempbuf,Width);   
   }
}

void ShowImage(HDC hDC,int  StartX,int  StartY,int nDestWidth ,int nDestHeight,
			   int  nSrcWidth ,int  nSrcHeight,BYTE *Buffer,BYTE Mode)				//set HDC Display device, start point(x,y) distination point(x',y') 
{                          
	BYTE lpDib[1078];
	BITMAPINFOHEADER InfoHeader;			
	RGBQUAD    aRGB[256];
	int i;

	InfoHeader.biSize	   =  40;						//Specifies the number of bytes required by the structure. 
	InfoHeader.biWidth	   = nSrcWidth;					//Specifies the width of the bitmap, in pixels. 
	InfoHeader.biHeight	   = nSrcHeight;				//Specifies the height of the bitmap, in pixels.
	InfoHeader.biPlanes	   = 1;							//Specifies the number of planes for the target device. This value must be set to 1. 
	InfoHeader.biBitCount	   = 8;						//Specifies the number of bits-per-pixel.
	InfoHeader.biCompression   = 0L;					//Specifies the type of compression for a compressed bottom-up bitmap (top-down DIBs cannot be compressed). 
	InfoHeader.biSizeImage	   = (long)(nSrcWidth)*(nSrcHeight);//Specifies the size, in bytes, of the image. This may be set to zero for BI_RGB bitmaps.
	InfoHeader.biXPelsPerMeter = 19685L;				//Specifies the horizontal resolution, in pixels-per-meter, of the target device for the bitmap.   500dpi
	InfoHeader.biYPelsPerMeter = 19685L;				//Specifies the vertical resolution, in pixels-per-meter, of the target device for the bitmap.     500dpi
	InfoHeader.biClrUsed	   = 256L;					//Specifies the number of color indexes in the color table that are actually used by the bitmap.
	InfoHeader.biClrImportant  = 256L;					//Specifies the number of color indexes that are required for displaying the bitmap.
	switch(Mode)
	{
		case 8:
			for ( i=0; i<256; i++ )						//color 0~255
			{
				aRGB[i].rgbRed      = i;
				aRGB[i].rgbGreen    = i;
				aRGB[i].rgbBlue     = i;
				aRGB[i].rgbReserved = 0;
			}
			
			break;
		case 2:
			for ( i=0; i<256; i++)
			{
				if (i<128)
				{
					aRGB[i].rgbRed      = 255;
					aRGB[i].rgbGreen    = 255;
					aRGB[i].rgbBlue     = 255;
					aRGB[i].rgbReserved = 0;
				}
				else
				{
					aRGB[i].rgbRed      = 0;
					aRGB[i].rgbGreen    = 0;
					aRGB[i].rgbBlue     = 0;
					aRGB[i].rgbReserved = 0;
				}
			}
		default:										//color 0 and 255
			aRGB[0].rgbRed      = 0;
			aRGB[0].rgbGreen    = 0;
			aRGB[0].rgbBlue     = 0;
			aRGB[0].rgbReserved = 0;
			aRGB[1].rgbRed      = 255;
			aRGB[1].rgbGreen    = 255;
			aRGB[1].rgbBlue     = 255;
			aRGB[1].rgbReserved = 0;
			aRGB[255].rgbRed      = 255;
			aRGB[255].rgbGreen    = 255;
			aRGB[255].rgbBlue     = 255;
			aRGB[255].rgbReserved = 0;
			break;
	}
	InvertImage(Buffer,nSrcWidth , nSrcHeight );
	/*----------consist InfoHeader and RGB to lpDib*/
	memcpy(lpDib,(BYTE*)&InfoHeader,sizeof(BITMAPINFOHEADER));
    memcpy( lpDib+sizeof(BITMAPINFOHEADER), (BYTE *)aRGB, 256*sizeof(RGBQUAD));
	/*--------------------------------------------------------*/
	SetStretchBltMode (hDC,STRETCH_DELETESCANS) ;								//Deletes the pixels. This mode deletes all eliminated lines of pixels without trying to preserve their information.
	StretchDIBits (hDC,StartX,StartY,nDestWidth ,nDestHeight,0,0,nSrcWidth,nSrcHeight,
		                	(LPSTR) Buffer,(LPBITMAPINFO)lpDib,DIB_RGB_COLORS,SRCCOPY);
	InvertImage(Buffer,nSrcWidth , nSrcHeight );
}

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
// CSDK_DEMODlg dialog

CSDK_DEMODlg::CSDK_DEMODlg(CWnd* pParent /*=NULL*/)
	: CDialog(CSDK_DEMODlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CSDK_DEMODlg)
	m_nPortNumber = 0;
	m_nBaudrate = 4;
	m_strResult = _T("");
	//}}AFX_DATA_INIT
	// Note that LoadIcon does not require a subsequent DestroyIcon in Win32

	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);

	m_bImageShow = TRUE;
	m_bContinue = FALSE;
	m_ing = FALSE;
}

void CSDK_DEMODlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CSDK_DEMODlg)
	DDX_CBIndex(pDX, IDC_COMBO_COM, m_nPortNumber);
	DDX_CBIndex(pDX, IDC_COMBO_BAUDRATE, m_nBaudrate);
	DDX_Control(pDX, IDC_SPIN_ID, m_spinID);
	DDX_Control(pDX, IDC_PROGRESS1, m_prog);
	DDX_Text(pDX, IDC_STATIC_RESULT, m_strResult);
	//}}AFX_DATA_MAP
}

BEGIN_MESSAGE_MAP(CSDK_DEMODlg, CDialog)
	//{{AFX_MSG_MAP(CSDK_DEMODlg)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_WM_CLOSE()
	ON_WM_TIMER()
	ON_BN_CLICKED(IDC_BUTT_OPEN, OnButtOpen)
	ON_BN_CLICKED(IDC_BUTT_CLOSE, OnButtClose)
	ON_BN_CLICKED(IDC_BUTT_ENROLL, OnButtEnroll)
	ON_BN_CLICKED(IDC_BUTT_VERIFY, OnButtVerify)
	ON_BN_CLICKED(IDC_BUTT_IDENTIFY, OnButtIdentify)
	ON_BN_CLICKED(IDC_BUTT_VERIFY_TEMPLATE, OnButtVerifyTemplate)
	ON_BN_CLICKED(IDC_BUTT_IDENTIFY_TEMPLATE, OnButtIdentifyTemplate)
	ON_BN_CLICKED(IDC_BUTT_ISPRESSFP, OnButtIspressfp)
	ON_BN_CLICKED(IDC_BUTT_GET_IMAGE, OnButtGetImage)
	ON_BN_CLICKED(IDC_BUTT_GET_RAWIMAGE, OnButtGetRawimage)
	ON_BN_CLICKED(IDC_BUTT_GET_USER_COUNT, OnButtGetUserCount)
	ON_BN_CLICKED(IDC_BUTT_DELETE, OnButtDelete)
	ON_BN_CLICKED(IDC_BUTT_DELETE_ALL, OnButtDeleteAll)
	ON_BN_CLICKED(IDC_BUTT_GET_TEMPLATE, OnButtGetTemplate)
	ON_BN_CLICKED(IDC_BUTT_SET_TEMPLATE, OnButtSetTemplate)
	ON_BN_CLICKED(IDC_BUTT_GET_DATABASE, OnButtGetDatabase)
	ON_BN_CLICKED(IDC_BUTT_SET_DATABASE, OnButtSetDatabase)
	ON_BN_CLICKED(IDC_BUTT_FW_UPGRADE, OnButtFwUpgrade)
	ON_BN_CLICKED(IDC_BUTT_ISO_UPGRADE, OnButtIsoUpgrade)
	ON_BN_CLICKED(IDC_BUTT_GET_LIVEIMAGE, OnButtGetLiveimage)
	ON_BN_CLICKED(IDC_BUTT_SAVE_IMAGE, OnButtSaveImage)
	ON_BN_CLICKED(IDC_BUTT_CANCEL, OnButtCancel)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CSDK_DEMODlg message handlers

BOOL CSDK_DEMODlg::OnInitDialog()
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
	SetRect(&m_DisplayRect,ImageX,ImageY,ImageX+320,ImageX+240);
	m_pDC = GetDC();
	EnableBtn(FALSE);
	GetDlgItem(IDC_BUTT_CANCEL)->EnableWindow(FALSE);
	
	//m_spinID.SetBuddy(GetDlgItem(IDC_EDIT_ID));
	m_spinID.SetRange32(0, FP_MAX_USERS - 1);
	m_spinID.SetPos32(0);

	m_prog.SetRange32(0, 10000);
	return TRUE;  // return TRUE  unless you set the focus to a control
}

void CSDK_DEMODlg::OnSysCommand(UINT nID, LPARAM lParam)
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

void CSDK_DEMODlg::OnPaint() 
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
// 		CDialog::OnPaint();
		
		CPaintDC dc(this); // device context for painting
	}
}

// The system calls this to obtain the cursor to display while the user drags
//  the minimized window.
HCURSOR CSDK_DEMODlg::OnQueryDragIcon()
{
	return (HCURSOR) m_hIcon;
}

void CSDK_DEMODlg::OnClose() 
{
	// TODO: Add your message handler code here and/or call default
	
	if (m_ing)
		return;

	CDialog::OnClose();
}

void CSDK_DEMODlg::OnTimer(UINT nIDEvent) 
{
	// TODO: Add your message handler code here and/or call default


	CDialog::OnTimer(nIDEvent);
}

BOOL CSDK_DEMODlg::PreTranslateMessage(MSG* pMsg) 
{
	// TODO: Add your specialized code here and/or call the base class
	
	if ((pMsg->message == WM_KEYDOWN || pMsg->message == WM_KEYUP) &&
		pMsg->wParam == VK_ESCAPE)
	{
		return TRUE;
	}

	return CDialog::PreTranslateMessage(pMsg);
}

BOOL CSDK_DEMODlg::OnCommand(WPARAM wParam, LPARAM lParam) 
{
	// TODO: Add your specialized code here and/or call the base class
	
	if (wParam == IDOK)
		return FALSE;

	return CDialog::OnCommand(wParam, lParam);
}


void CSDK_DEMODlg::EnableBtn(BOOL bEnable, BOOL bDisableAll)
{
	GetDlgItem(IDC_BUTT_OPEN)->EnableWindow(!bEnable && !bDisableAll);
    
	for (UINT id=IDC_BUTT_CLOSE; id<=IDC_BUTT_SAVE_IMAGE; id++)
	{
		try
		{
			GetDlgItem(id)->EnableWindow(bEnable && !bDisableAll);
		}
		catch (...)
		{
		}
	}
}

int CSDK_DEMODlg::Capturing(BOOL bBest/* = FALSE*/)
{
	return 0;
}

int CSDK_DEMODlg::LoadingImage(void)
{
	return 0;
}

int CSDK_DEMODlg::LoadingImageRaw(void)
{
	return 0;
}

int CSDK_DEMODlg::LoadingImageLive(void)
{
	return 0;
}

void CSDK_DEMODlg::DisplayErr(int nNackInfo, int nPos)
{
	if( nNackInfo > NACK_NONE)
	{
		switch(nNackInfo)
		{
		case NACK_TIMEOUT:
			m_strResult = _T("Time Out!");
			break;
		case NACK_INVALID_BAUDRATE:
			m_strResult = _T("Invalid baudrate");
			break;
		case NACK_INVALID_POS:
			m_strResult = _T("Invalid ID");
			break;
		case NACK_IS_NOT_USED:
			m_strResult.Format(_T("ID = %d: is not used!"), nPos);
			break;
		case NACK_IS_ALREADY_USED:
			m_strResult.Format(_T("ID = %d: is already used!"), nPos);
			break;
		case NACK_COMM_ERR:
			m_strResult = _T("Communication error!");
			break;
		case NACK_VERIFY_FAILED:
			m_strResult.Format(_T("ID = %d: NG!"), nPos);
			break;
		case NACK_IDENTIFY_FAILED:
			m_strResult = _T("NG!");
			break;
		case NACK_DB_IS_FULL:
			m_strResult = _T("Database is full");
			break;
		case NACK_DB_IS_EMPTY:
			m_strResult = _T("Database is empty");
			break;
		case NACK_TURN_ERR:
			m_strResult = _T("The order of enrollment is incorrect!");
			break;
		case NACK_BAD_FINGER:
			m_strResult = _T("Bad finger!");
			break;
		case NACK_ENROLL_FAILED:
			m_strResult = _T("The enrollment is failed!");
			break;
		case NACK_IS_NOT_SUPPORTED:
			m_strResult = _T("The command is not supported");
			break;
		case NACK_DEV_ERR:
			m_strResult = _T("The device error!");
			break;
		case NACK_CAPTURE_CANCELED:
			m_strResult = _T("Canceled!");
			break;
		case NACK_INVALID_PARAM:
			m_strResult = _T("Invalid Firmware Image!");
			break;
		case NACK_FINGER_IS_NOT_PRESSED:
			m_strResult = _T("Finger is not pressed!");
			break;
		default:
			m_strResult = _T("Unknown Error");
			break;
		}
	}
	else if (nNackInfo<FP_MAX_USERS)
	{
		m_strResult.Format(_T("ID = %d: is already enrolled!"), nNackInfo);
	}
}

void CSDK_DEMODlg::SetBmp(BOOL bIsRaw/* = FALSE*/)
{


	ui_polling();
}

void CSDK_DEMODlg::OnButtOpen() 
{
	int rtn;
	unsigned char DataBuffer[12];
	int Flag = 1;
	CString str;
	ucImageData = new BYTE[320*HSIZE];
	rtn = MyConnect();
	if(rtn != OK)
	{
		m_strResult = _T("USB Connect Fail!!");
		UpdateData(FALSE);
		delete ucImageData; 
		return;
	}
	rtn = Open(Flag,&DataBuffer[0]);
	if(rtn != OK)
	{
		m_strResult = _T("USB Open Fail!!");
		UpdateData(FALSE);
		MyDisconnect();
		delete ucImageData;
		return;
	}
	
	//	UART only
	/*rtn = ChangeBaudRate(9600);
	if(rtn != OK)
	{
		m_strResult = _T("ChangeBaudRate Fail!!");
		UpdateData(FALSE);
		return;
	}*/
	
	m_strResult = _T("USB Connect Success!!");
	if(Flag)
	{
		str.Format(_T("\nFirmware Version = %02d%02d%02d%02d"),DataBuffer[4],DataBuffer[5],DataBuffer[6],DataBuffer[7]);
		m_strResult += str;
	}
	UpdateData(FALSE);
	EnableBtn(TRUE);
}

void CSDK_DEMODlg::OnButtClose() 
{
	// TODO: Add your control notification handler code here
	/*UpdateData();*/
	m_strResult = _T("");
	UpdateData(FALSE);
	MyDisconnect();
	delete ucImageData; 
	EnableBtn(FALSE);
	GetDlgItem(IDC_BUTT_CANCEL)->EnableWindow(FALSE);
	GetDlgItem(IDC_BUTT_OPEN)->SetFocus();
}

void CSDK_DEMODlg::OnButtEnroll() 
{
	int rtn;
	int nID = m_spinID.GetPos32();
	DWORD DTime;
	if (nID < -1)
		nID = -1;
	if (nID > FP_MAX_USERS - 1)
		nID = FP_MAX_USERS - 1;
	
	LED(1);
	rtn = Enroll_Start(nID);

	if(rtn != OK)
	{
		DisplayErr(rtn, nID);
		UpdateData(FALSE);
		LED(0);
		return;
	}
	for(int i=1; i<4; i++)
	{
		m_strResult.Format(_T("Input finger %d !"), i);
		UpdateData(FALSE);
		DTime = GetTickCount();
		while(GetTickCount() - DTime < TIMEOUT)
		{
			rtn = CaptureFinger(TRUE);
			if(rtn == OK)
				break;
		}
		if(GetTickCount() - DTime > TIMEOUT)
		{
			m_strResult = _T("TimeOut...");
			UpdateData(FALSE);
			LED(0);
			return;
		}
		if(rtn == OK)
		{
			rtn = ReceiveCommand(ucImageData,120*512);
			ShowImage(m_pDC->m_hDC,m_DisplayRect.left,m_DisplayRect.top,HSIZE,VSIZE,HSIZE,VSIZE,ucImageData,8);
		}
		/*if(m_bImageShow && gn_comm_type == COMM_MODE_USB)
		{
			if(LoadingImage() < 0){
				return;
			}
			SetBmp();
		}*/
		
		m_strResult = _T("Processing fingerprint...");
		UpdateData(FALSE);
	
		rtn = Enroll_nth(i);
		if(rtn != OK)
		{
			DisplayErr(rtn, 0);
			UpdateData(FALSE);
			LED(0);
			return;
		}

		m_strResult.Format( _T("Take off finger, please...") );
		UpdateData(FALSE);
		if( i<3 )
		{
			while(1)
			{
				rtn = RemoveFinger();
				if(rtn == NACK_FINGER_IS_NOT_PRESSED)
					break;
			}
		}
		else if(rtn == OK)
		{
			m_spinID.SetPos32(nID+1);
			m_strResult.Format(_T("Enroll OK (ID = %d)!"), nID);
			UpdateData(FALSE);
		}
	}
	LED(0);
}

void CSDK_DEMODlg::OnButtVerify() 
{
	int rtn;
	DWORD DTime,DTime1;
	GetDlgItem(IDC_BUTT_CANCEL)->EnableWindow(TRUE);
	GetDlgItem(IDC_BUTT_CLOSE)->EnableWindow(FALSE);
	ui_polling();
	m_bContinue = TRUE;
	int nID = m_spinID.GetPos32();
	if (nID < 0 || nID >= FP_MAX_USERS)
	{
		nID = 0;
		m_spinID.SetPos32(nID);
	}

	rtn = CheckEnroll(nID);
	if(rtn != OK)
	{
		DisplayErr(rtn, nID);
		UpdateData(FALSE);
		return;
	}
	m_strResult = _T("; Input finger !");
	UpdateData(FALSE);
	LED(1);
	DTime = GetTickCount();
	while(m_bContinue)
	{
		while(rtn = CaptureFinger(TRUE) !=OK)
		{
			ui_polling();
			if(m_bContinue == FALSE)
			{
				m_strResult = _T("");
				UpdateData(FALSE);
				GetDlgItem(IDC_BUTT_CANCEL)->EnableWindow(FALSE);
				return;
			}
		}
		if(rtn == OK)
		{
			rtn = ReceiveCommand(ucImageData,120*512);
			ShowImage(m_pDC->m_hDC,m_DisplayRect.left,m_DisplayRect.top,HSIZE,VSIZE,HSIZE,VSIZE,ucImageData,8);
		}
		
		m_strResult = _T("Processing fingerprint...");
		UpdateData(FALSE);
		DTime1 = GetTickCount();
		rtn = Verify(nID);
		DTime1 = GetTickCount() - DTime1;
		if(rtn > FP_MAX_USERS)
		{
			DisplayErr(rtn, nID);
			UpdateData(FALSE);
			//return;
		}
		else
		{
			m_strResult.Format(_T("ID = %d	,	%dms	\nInput finger !"), nID,DTime1);
			UpdateData(FALSE);
		}
	}
	GetDlgItem(IDC_BUTT_CANCEL)->EnableWindow(FALSE);
	LED(0);
}

void CSDK_DEMODlg::OnButtIdentify() 
{
	int rtn;
	DWORD DTime,DTime1;
	GetDlgItem(IDC_BUTT_CANCEL)->EnableWindow(TRUE);
	GetDlgItem(IDC_BUTT_CLOSE)->EnableWindow(FALSE);
	ui_polling();
	m_bContinue = TRUE;
	rtn = GetEnrollCount();
	if(rtn > FP_MAX_USERS)
	{
		DisplayErr(rtn, 0);
		UpdateData(FALSE);
		return;
	}
	m_strResult = _T("; Input finger !");
	UpdateData(FALSE);
	LED(1);
	while(m_bContinue)
	{
		
		while(rtn = CaptureFinger(TRUE) !=OK)
		{
			ui_polling();
			if(m_bContinue == FALSE)
			{
				m_strResult = _T("");
				UpdateData(FALSE);
				GetDlgItem(IDC_BUTT_CANCEL)->EnableWindow(FALSE);
				LED(0);
				return;
			}
		}
		if(rtn == OK)
		{
			rtn = ReceiveCommand(ucImageData,120*512);
			ShowImage(m_pDC->m_hDC,m_DisplayRect.left,m_DisplayRect.top,HSIZE,VSIZE,HSIZE,VSIZE,ucImageData,8);
		}
		
		
		m_strResult = _T("Processing fingerprint...");
		UpdateData(FALSE);
		DTime1 = GetTickCount();
		rtn = Identify();
		DTime1 = GetTickCount() - DTime1;
		if(rtn > FP_MAX_USERS)
		{
			DisplayErr(rtn, 0);
			UpdateData(FALSE);
			//return;
		}
		else
		{
			m_strResult.Format(_T("ID = %d	,	%dms	\nInput finger ! "), rtn,DTime1);
			UpdateData(FALSE);
		}
	
	}
	LED(0);
	GetDlgItem(IDC_BUTT_CANCEL)->EnableWindow(FALSE);
}

void CSDK_DEMODlg::OnButtVerifyTemplate() 
{
	int rtn,i;
	unsigned char DataBuffer[512];
	int nID = m_spinID.GetPos32();
	char szFilter[80];
	FILE *fptr;
	CString str;
	char String[256], Filename[256], CurrentDir[256];
	DWORD DTime,DTime1;
	if (nID < 0 || nID >= FP_MAX_USERS)
	{
		nID = 0;
		m_spinID.SetPos32(nID);
	}
	rtn = CheckEnroll(nID);
	if(rtn != OK)
	{
		DisplayErr(rtn, nID);
		UpdateData(FALSE);
		return;
	}
	strcpy(szFilter,"BIN *.bin|*.bin;|All Files(*.*)|*.*||");
	CFileDialog fdlg(TRUE,"bin",NULL,OFN_HIDEREADONLY,szFilter);
	while(1)
	{
		if(fdlg.DoModal()==IDOK)
		{
			strcpy(Filename,fdlg.GetPathName());
			if( (fptr=fopen(Filename ,"rb") )!=NULL)
			{
				fclose(fptr);
			}
			break;
		}
		else
		{
			return;
		}
	}
	for(i =0; i<256; i++)
	{
		str = str + Filename[i];
	}
	fptr = fopen(str,"rb");
	fread(DataBuffer,1,512,fptr);
	fclose(fptr);
	DTime1 = GetTickCount();
	rtn = VerifyTemplate(nID,DataBuffer);	
	DTime1 = GetTickCount() - DTime1;
	if(rtn > FP_MAX_USERS)
	{
		DisplayErr(rtn, nID);
		UpdateData(FALSE);
	}
	else
	{
		m_strResult.Format(_T("ID = %d	,	%dms"), nID,DTime1);
		UpdateData(FALSE);
	}
}

void CSDK_DEMODlg::OnButtIdentifyTemplate() 
{
	int rtn,i;
	unsigned char DataBuffer[512];
	int nID = m_spinID.GetPos32();
	char szFilter[80];
	FILE *fptr;
	CString str;
	char String[256], Filename[256], CurrentDir[256];
	DWORD DTime,DTime1;
	if (nID < 0 || nID >= FP_MAX_USERS)
	{
		nID = 0;
		m_spinID.SetPos32(nID);
	}
	rtn = CheckEnroll(nID);
	if(rtn != OK)
	{
		DisplayErr(rtn, nID);
		UpdateData(FALSE);
		return;
	}
	strcpy(szFilter,"BIN *.bin|*.bin;|All Files(*.*)|*.*||");
	CFileDialog fdlg(TRUE,"bin",NULL,OFN_HIDEREADONLY,szFilter);
	while(1)
	{
		if(fdlg.DoModal()==IDOK)
		{
			strcpy(Filename,fdlg.GetPathName());
			if( (fptr=fopen(Filename ,"rb") )!=NULL)
			{
				fclose(fptr);
			}
			break;
		}
		else
		{
			return;
		}
	}
	for(i =0; i<256; i++)
	{
		str = str + Filename[i];
	}
	fptr = fopen(str,"rb");
	fread(DataBuffer,1,512,fptr);
	fclose(fptr);
	DTime1 = GetTickCount();
	rtn = IdentifyTemplate(nID,DataBuffer);	
	DTime1 = GetTickCount() - DTime1;
	if(rtn > FP_MAX_USERS)
	{
		DisplayErr(rtn, nID);
		UpdateData(FALSE);
	}
	else
	{
		m_strResult.Format(_T("ID = %d	,	%dms"), nID,DTime1);
		UpdateData(FALSE);
	}
}

void CSDK_DEMODlg::OnButtIspressfp() 
{
	int rtn;
	LED(1);
	rtn = RemoveFinger();
	if( rtn != 0 )
	{
		DisplayErr( rtn, 0 );
		UpdateData(FALSE);
		LED(0);
		return;
	}
	LED(0);
	m_strResult = _T("Finger is pressed!");
	UpdateData(FALSE);
}

void CSDK_DEMODlg::OnButtGetImage() 
{
	int rtn;
	m_strResult = _T("");
	UpdateData(FALSE);
	LED(1);
	rtn = CaptureFinger(TRUE);
	if(rtn == OK)
	{
		rtn = ReceiveCommand(ucImageData,120*512);
		ShowImage(m_pDC->m_hDC,m_DisplayRect.left,m_DisplayRect.top,HSIZE,VSIZE,HSIZE,VSIZE,ucImageData,8);
	}
	LED(0);
}

void CSDK_DEMODlg::OnButtGetRawimage() 
{
	int rtn;
	LED(1);
	rtn = GetRawImage();
	if(rtn == OK)
	{
		rtn = ReceiveCommand(ucImageData,120*512);
		ShowImage(m_pDC->m_hDC,m_DisplayRect.left,m_DisplayRect.top,HSIZE,VSIZE,HSIZE,VSIZE,ucImageData,8);
	}
	LED(0);
}

void CSDK_DEMODlg::OnButtGetUserCount() 
{
	int rtn;
	rtn = GetEnrollCount();
	if(rtn > FP_MAX_USERS)
	{
		DisplayErr(rtn, 0);
		UpdateData(FALSE);
		return;
	}
	else
	{
		m_strResult.Format(_T("Enroll count = %d !!"), rtn);
		UpdateData(FALSE);
	}
}

void CSDK_DEMODlg::OnButtDelete() 
{
	// TODO: Add your control notification handler code here
	int rtn;
	int nID = m_spinID.GetPos32();
	if (nID < 0 || nID >= FP_MAX_USERS)
	{
		nID = 0;
		m_spinID.SetPos32(nID);
	}

	rtn = DeleteID(nID);

	if(rtn != OK)
	{
		DisplayErr(rtn, nID);
		UpdateData(FALSE);
		return;
	}

	m_strResult = _T("Delete OK !");
	UpdateData(FALSE);
	
}

void CSDK_DEMODlg::OnButtDeleteAll() 
{
	int rtn;
	rtn = DeleteAll();
	if(rtn != OK)
	{
		DisplayErr(rtn, 0);
		UpdateData(FALSE);
		return;
	}
	m_strResult = _T("Delete all OK !");
	UpdateData(FALSE);
}

void CSDK_DEMODlg::OnButtGetTemplate() 
{
	int rtn;
	int i;
	unsigned char DataBuffer[512];
	int nID = m_spinID.GetPos32();
	char szFilter[80];
	FILE *fptr;
	CString str;
	char String[256], Filename[256], CurrentDir[256];
	if (nID < 0 || nID >= FP_MAX_USERS)
	{
		nID = 0;
		m_spinID.SetPos32(nID);
	}

	rtn = GetTemple(nID,&DataBuffer[0]);
	if(rtn != OK)
	{
		DisplayErr(rtn, 0);
		UpdateData(FALSE);
		return;
	}
	strcpy(szFilter,"BIN *.bin|*.bin;|All Files(*.*)|*.*||");
	CFileDialog fdlg(FALSE,"bin",NULL,OFN_HIDEREADONLY,szFilter);
	while(1)
	{
		if(fdlg.DoModal()==IDOK)
		{
			strcpy(Filename,fdlg.GetPathName());
			if( (fptr=fopen(Filename ,"rb") )!=NULL)
			{
				fclose(fptr);
				rtn=MessageBox("The file already exists !!\nOverwrite?",
											  "Warning",MB_YESNO | MB_ICONWARNING);
				if(rtn==IDNO)
					continue;
			}
			break;
		}
		else
		{
			return;
		}
	}
	for(i =0; i<256; i++)
	{
		str = str + Filename[i];
	}
	fptr = fopen(str,"wb");
	fwrite(DataBuffer,1,512,fptr);
	fclose(fptr);
	m_strResult = _T("Get Template OK !");
	UpdateData(FALSE);
}

void CSDK_DEMODlg::OnButtSetTemplate() 
{
	int rtn;
	int i;
	unsigned char DataBuffer[512];
	int nID = m_spinID.GetPos32();
	char szFilter[80];
	FILE *fptr;
	CString str;
	char String[256], Filename[256], CurrentDir[256];
	if (nID < 0 || nID >= FP_MAX_USERS)
	{
		nID = 0;
		m_spinID.SetPos32(nID);
	}

	strcpy(szFilter,"BIN *.bin|*.bin;|All Files(*.*)|*.*||");
	CFileDialog fdlg(TRUE,"bin",NULL,OFN_HIDEREADONLY,szFilter);
	while(1)
	{
		if(fdlg.DoModal()==IDOK)
		{
			strcpy(Filename,fdlg.GetPathName());
			if( (fptr=fopen(Filename ,"rb") )!=NULL)
			{
				fclose(fptr);
			}
			break;
		}
		else
		{
			return;
		}
	}
	for(i =0; i<256; i++)
	{
		str = str + Filename[i];
	}
	fptr = fopen(str,"rb");
	fread(DataBuffer,1,512,fptr);
	fclose(fptr);

	rtn = SetTemple(nID,&DataBuffer[0]);

	if(rtn == OK)
	{
		m_strResult = _T("Set Template OK !");
		UpdateData(FALSE);
	}
	else
	{
		DisplayErr(rtn, nID);
		UpdateData(FALSE);
	}
}

void CSDK_DEMODlg::OnButtGetDatabase() 
{
	int rtn;
	int i;
	unsigned char DataBaseBuffer[512 * FP_MAX_USERS];
	char szFilter[80];
	FILE *fptr;
	CString str;
	char String[256], Filename[256], CurrentDir[256];
	rtn = GetEnrollCount();
	if(rtn > FP_MAX_USERS)
	{
		DisplayErr(rtn, 0);
		UpdateData(FALSE);
		return;
	}
	rtn = GetDatabaseStart();
	if(rtn != OK)
	{
		DisplayErr(rtn, 0);
		UpdateData(FALSE);
		return;
	}
	for(i = 0; i < FP_MAX_USERS; i++)
	{
		m_strResult.Format(_T("Getting of %dth template..."), i);
		UpdateData(FALSE);
		ui_polling();

		rtn = GetTemple(i,&DataBaseBuffer[i*512]);
		if(rtn != OK)
		{
			DisplayErr(rtn, 0);
			UpdateData(FALSE);
			GetDatabaseEnd();
			return;
		}
	}
	GetDatabaseEnd();
	if(rtn != OK)
	{
		DisplayErr(rtn, 0);
		UpdateData(FALSE);
		return;
	}
	strcpy(szFilter,"BIN *.bin|*.bin;|All Files(*.*)|*.*||");
	CFileDialog fdlg(FALSE,"bin",NULL,OFN_HIDEREADONLY,szFilter);
	while(1)
	{
		if(fdlg.DoModal()==IDOK)
		{
			strcpy(Filename,fdlg.GetPathName());
			if( (fptr=fopen(Filename ,"rb") )!=NULL)
			{
				fclose(fptr);
				rtn=MessageBox("The file already exists !!\nOverwrite?",
											  "Warning",MB_YESNO | MB_ICONWARNING);
				if(rtn==IDNO)
					continue;
			}
			break;
		}
		else
		{
			return;
		}
	}
	for(i =0; i<256; i++)
	{
		str = str + Filename[i];
	}
	fptr = fopen(str,"wb");
	fwrite(DataBaseBuffer,1,512*FP_MAX_USERS,fptr);
	fclose(fptr);
	m_strResult = _T("Get Database OK !");
	UpdateData(FALSE);
}

void CSDK_DEMODlg::OnButtSetDatabase() 
{
	int rtn;
	int i;
	unsigned char DataBaseBuffer[512 * FP_MAX_USERS];
	char szFilter[80];
	FILE *fptr;
	CString str;
	char String[256], Filename[256], CurrentDir[256];
	int nAddCount = 0;
	strcpy(szFilter,"BIN *.bin|*.bin;|All Files(*.*)|*.*||");
	CFileDialog fdlg(TRUE,"bin",NULL,OFN_HIDEREADONLY,szFilter);
	while(1)
	{
		if(fdlg.DoModal()==IDOK)
		{
			strcpy(Filename,fdlg.GetPathName());
			if( (fptr=fopen(Filename ,"rb") )!=NULL)
			{
				fclose(fptr);
			}
			break;
		}
		else
		{
			return;
		}
	}
	for(i =0; i<256; i++)
	{
		str = str + Filename[i];
	}
	fptr = fopen(str,"rb");
	fread(DataBaseBuffer,1,512 * FP_MAX_USERS,fptr);
	fclose(fptr);

	
	for (int i=0; i<FP_MAX_USERS; i++)
	{
		m_strResult.Format(_T("Adding of %dth template..."), i);
		UpdateData(FALSE);
		ui_polling();
		rtn = SetTemple(i,&DataBaseBuffer[i * 512]);

		if(rtn != OK)
		{
			DisplayErr(rtn, 0);
			UpdateData(FALSE);
		}
		nAddCount++;
	}
	
	m_strResult.Format(_T("Set Database OK (Enroll count = %d)!"), nAddCount);
	UpdateData(FALSE);
}

void CSDK_DEMODlg::OnButtFwUpgrade() 
{
	int rtn;
	int i;
	unsigned char DataBuffer[180*512];
	unsigned char HeaderInfo[512];
	int nID = m_spinID.GetPos32();
	char szFilter[80];
	FILE *fptr;
	CString str;
	char String[256], Filename[256], CurrentDir[256];
	unsigned long filesize;
	strcpy(szFilter,"BIN *.bin|*.bin;|All Files(*.*)|*.*||");
	CFileDialog fdlg(TRUE,"bin",NULL,OFN_HIDEREADONLY,szFilter);
	while(1)
	{
		if(fdlg.DoModal()==IDOK)
		{
			strcpy(Filename,fdlg.GetPathName());
			if( (fptr=fopen(Filename ,"rb") )!=NULL)
			{
				fclose(fptr);
			}
			break;
		}
		else
		{
			return;
		}
	}
	for(i =0; i<256; i++)
	{
		str = str + Filename[i];
	}
	fptr = fopen(str,"rb");
	filesize = GetFileLength(fptr);
	fread(DataBuffer,1,filesize,fptr);
	fclose(fptr);
	rtn = UpgradeFirmware();
	if(rtn != OK)
	{
		DisplayErr(rtn, 0);
		UpdateData(FALSE);
		return;
	}
	MyDisconnect();
	Sleep(8000);
	rtn = MyConnect();
	if(rtn != OK)
	{
		m_strResult = _T("USB Connect Fail!!");
		UpdateData(FALSE);
		return;
	}

	m_prog.ShowWindow( SW_SHOW );
	m_prog.SetPos( 0 );
	m_prog.SetRange32( 0, filesize );

	//EnableBtn( FALSE );
	//GetDlgItem(IDC_BUTT_OPEN)->EnableWindow( FALSE );
	m_strResult = _T("Firmware Sending...  Don't power off the device.");
	UpdateData( FALSE );

	HeaderInfo[0] = (filesize & 0xFF000000) >> 24;
	HeaderInfo[1] = (filesize & 0xFF0000) >> 16;
	HeaderInfo[2] = (filesize & 0xFF00) >> 8;
	HeaderInfo[3] = filesize & 0xFF;
	rtn = SendCommand((unsigned char *)HeaderInfo,MASS_BUFFER);
	if(rtn == FAIL)
		return;
	rtn = ReceiveCommand((unsigned char *)HeaderInfo,MASS_BUFFER);
	if(rtn == FAIL)
		return;
	for(i = 0; i < (filesize / MAX_ENTRY_LEN) + 1 ; i++)
	{
		Sleep(20);
		rtn = SendCommand(&DataBuffer[i * MAX_ENTRY_LEN],MASS_BUFFER);
		if(rtn == FAIL)
			return;
		m_prog.SetPos( i*MAX_ENTRY_LEN );
	}
	rtn = ReceiveCommand((unsigned char *)HeaderInfo,MASS_BUFFER);
	if(rtn == FAIL)
		return;
	m_strResult = _T("Firmware Upgrade Success!.");
	UpdateData( FALSE );
	MyDisconnect();
	EnableBtn(FALSE);
	m_prog.ShowWindow( SW_HIDE );
}

void CSDK_DEMODlg::OnButtIsoUpgrade() 
{

}

void CSDK_DEMODlg::OnButtGetLiveimage() 
{
	int rtn;
	GetDlgItem(IDC_BUTT_CANCEL)->EnableWindow(TRUE);
	GetDlgItem(IDC_BUTT_CLOSE)->EnableWindow(FALSE);
	ui_polling();
	m_bContinue = TRUE;
	m_strResult = _T("Downloading Live Image...");
	UpdateData(FALSE);
	LED(1);
	while(m_bContinue)
	{
		
		rtn = GetRawImage();
		if(rtn == OK)
		{
			rtn = ReceiveCommand(ucImageData,120*512);
			ShowImage(m_pDC->m_hDC,m_DisplayRect.left,m_DisplayRect.top,HSIZE,VSIZE,HSIZE,VSIZE,ucImageData,8);
		}
		ui_polling();
	}
	LED(0);
	GetDlgItem(IDC_BUTT_CANCEL)->EnableWindow(FALSE);
	m_strResult = _T("");
	UpdateData(FALSE);
}

void CSDK_DEMODlg::OnButtSaveImage() 
{
	char szFilter[80];
    int    rtn; 
	char buf[80];
	int i;
	CString str;
	FILE *fptr;
	BITMAPFILEHEADER FileHeader;
	BITMAPINFOHEADER InfoHeader;
	RGBQUAD palRGB[256];
	char String[256], Filename[256], CurrentDir[256];	
	strcpy(szFilter,"BMP *.bmp|*.bmp;|All Files(*.*)|*.*||");
	CFileDialog fdlg(FALSE,"bmp",NULL,OFN_HIDEREADONLY,szFilter);
	while(1)
	{
		if(fdlg.DoModal()==IDOK)
		{
			strcpy(Filename,fdlg.GetPathName());
			if( (fptr=fopen(Filename ,"rb") )!=NULL)
			{
				fclose(fptr);
				rtn=MessageBox("The file already exists !!\nOverwrite?",
											  "Warning",MB_YESNO | MB_ICONWARNING);
				if(rtn==IDNO)
					continue;
			}
			break;
		}
		else
		{
			return;
		}
	}
	for(i =0; i<256; i++)
	{
		str = str + Filename[i];
	}
	FileHeader.bfType     = 0x4d42;
	FileHeader.bfReserved1 = 0;
	FileHeader.bfReserved2 = 0;
	InfoHeader.biSize	   = sizeof( BITMAPINFOHEADER);
	InfoHeader.biWidth	   = HSIZE;
	InfoHeader.biHeight	   = VSIZE;
	InfoHeader.biPlanes	   = 1;
	InfoHeader.biCompression   = BI_RGB;
	InfoHeader.biXPelsPerMeter = 19685L;
	InfoHeader.biYPelsPerMeter = 19685L;
	FileHeader.bfSize     = (unsigned long)HSIZE*VSIZE+1078L;
	FileHeader.bfOffBits  = 0x00000436;
	InfoHeader.biBitCount	   = 8;
	InfoHeader.biSizeImage	   = (unsigned long)HSIZE*VSIZE;
	InfoHeader.biClrUsed	   = 256L;
	InfoHeader.biClrImportant  = 256L;
	for ( i=0; i<256; i++ )
	{
		palRGB[i].rgbBlue  = (BYTE)i;
		palRGB[i].rgbGreen = (BYTE)i;
		palRGB[i].rgbRed   = (BYTE)i;
	}
	fptr = fopen(str,"wb");
	fwrite(&FileHeader, 1, sizeof(BITMAPFILEHEADER), fptr );
	fwrite( &InfoHeader, 1, sizeof(BITMAPINFOHEADER), fptr);
	fwrite( palRGB, 1, sizeof(RGBQUAD)*i, fptr );
	InvertImage(ucImageData,HSIZE , VSIZE );
	fwrite(ucImageData,1,(long)HSIZE*VSIZE,fptr);
	fclose(fptr);
}

void CSDK_DEMODlg::OnButtCancel() 
{
	// TODO: Add your control notification handler code here
	GetDlgItem(IDC_BUTT_CLOSE)->EnableWindow(TRUE);
	m_bContinue = FALSE;
	m_strResult = _T("");
	LED(0);
}
