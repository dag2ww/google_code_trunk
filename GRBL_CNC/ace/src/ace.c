#include <windows.h>
#include <stdio.h>
#include <dir.h>
#include <math.h>
#include <commctrl.h>
#include "ace.h"
#include "converter.h"

LRESULT CALLBACK WndProc (HWND, UINT, WPARAM, LPARAM) ;
LRESULT CALLBACK CodeWndProc (HWND, UINT, WPARAM, LPARAM) ;
BOOL PopFileOpenDlg (HWND, PSTR, PSTR, OPENFILENAME *) ;
BOOL PopFileSaveDlg (HWND, PSTR, PSTR, OPENFILENAME *) ;
BOOL CALLBACK LayerDlgProc (HWND, UINT, WPARAM, LPARAM) ;
BOOL CALLBACK PriorityDlgProc (HWND, UINT, WPARAM, LPARAM) ;
BOOL CALLBACK ConvertDlgProc (HWND, UINT, WPARAM, LPARAM) ;
BOOL CALLBACK LicenseDlgProc (HWND, UINT, WPARAM, LPARAM) ;
BOOL CALLBACK EndDlgProc (HWND, UINT, WPARAM, LPARAM) ;
BOOL CALLBACK SetupDlgProc (HWND, UINT, WPARAM, LPARAM) ;

int DelPriority (int, int);
int NewPriority (int);
int ReadLayer (char *);
void GetOption(char, char *);
void SaveOption(char, char *);
char *current_directory(char *path);
int check_key(char *, char *);
HWND hWindow;
HINSTANCE hInst;
char szAppName[]="ACEConverter";
char szAcePath[_MAX_PATH];
int demo=FALSE;
int precision=3;
double defaultzoffset=-1.5;
double defaultmaxzpass=100;
double defaultreleaseplane=30;
double defaultcloseenough=0.01;
int defaultpriorityoptimization=FALSE;

int WINAPI WinMain (HINSTANCE hInstance, HINSTANCE hPrevInstance,
                    PSTR szCmdLine, int iCmdShow)
     {
     HWND         hwnd ;
     MSG          msg ;
     WNDCLASSEX   wndclass ;
     hInst=hInstance;

     wndclass.cbSize        = sizeof (wndclass) ;
     wndclass.style         = CS_HREDRAW | CS_VREDRAW;
     wndclass.lpfnWndProc   = WndProc ;
     wndclass.cbClsExtra    = 0 ;
     wndclass.cbWndExtra    = DLGWINDOWEXTRA ;
     wndclass.hInstance     = hInstance ;
     wndclass.hIcon         = LoadIcon (hInstance, szAppName) ;
     wndclass.hCursor       = LoadCursor (NULL, IDC_ARROW) ;
     wndclass.hbrBackground = (HBRUSH) (COLOR_BACKGROUND) ;
     wndclass.lpszMenuName  = NULL ;
     wndclass.lpszClassName = szAppName ;
     wndclass.hIconSm       = LoadIcon (hInstance, szAppName) ;

     RegisterClassEx (&wndclass) ;

     hWindow = hwnd = CreateDialog (hInstance, szAppName, 0, NULL) ;
     EnableWindow(GetDlgItem(hwnd,IDD_CONVERT), FALSE);
     ShowWindow (hwnd, iCmdShow) ;

     while (GetMessage (&msg, NULL, 0, 0))
          {
          TranslateMessage (&msg) ;
          DispatchMessage (&msg) ;
          }
     return msg.wParam ;
     }

struct layer_obj *layer, *temp_lay ;
struct priority_obj *priority, *temp_pri ;
struct convert_obj convertop ;
int iLayerCount, iPrioCount;
char szDxfFile[_MAX_PATH], szToFileName[_MAX_PATH];

LRESULT CALLBACK WndProc (HWND hwnd, UINT iMsg, WPARAM wParam, LPARAM lParam)
     {
     static char szTitleName[_MAX_FNAME + _MAX_EXT] ;
     static char szDrive[MAXDRIVE], szDir[MAXDIR], szName[MAXFILE], szExt[MAXEXT] ;
     static char szDXFFilter[]="DXF Files (*.DXF)\0*.dxf\0" ;
     static char szNCFilter[]="NC Files (*.NC)\0*.nc\0" ;
     static char szString[50], status[100], szFileName[_MAX_PATH];
     static char szHelp[_MAX_PATH], szDirectory[_MAX_PATH] ;
     static char szPrecision;
     static char *conversionend;
     static int i, iID;
     static HINSTANCE hInstance ;
     static OPENFILENAME ofn ;

     switch (iMsg)
          {
          case WM_CREATE :
//               GetOption('1',szAcePath);
//               sprintf(szHelp,"%s\\index.html",szAcePath);
     				current_directory(szAcePath);
               GetOption('4',&szPrecision);
               precision=atoi(&szPrecision);
               GetOption('5',&szString);
               defaultzoffset=strtod(szString,&conversionend);
               GetOption('6',&szString);
               defaultmaxzpass=strtod(szString,&conversionend);
               GetOption('7',&szString);
               defaultreleaseplane=strtod(szString,&conversionend);
               GetOption('8',&szString);
               defaultcloseenough=strtod(szString,&conversionend);
               sprintf(szHelp,"%s\\doc\\index.html",szAcePath);
               hInstance = ((LPCREATESTRUCT) lParam)->hInstance ;
               ofn.lStructSize       = sizeof (OPENFILENAME) ;
               ofn.hwndOwner         = hwnd ;
               ofn.hInstance         = NULL ;
               ofn.lpstrCustomFilter = NULL ;
               ofn.nMaxCustFilter    = 0 ;
               ofn.nFilterIndex      = 0 ;
               ofn.lpstrFile         = NULL ;          // Set in Open and Close functions
               ofn.nMaxFile          = _MAX_PATH ;
               ofn.lpstrFileTitle    = NULL ;          // Set in Open and Close functions
               ofn.nMaxFileTitle     = _MAX_FNAME + _MAX_EXT ;
               ofn.lpstrTitle        = NULL ;
               ofn.Flags             = 0 ;             // Set in Open and Close functions
               ofn.nFileOffset       = 0 ;
               ofn.nFileExtension    = 0 ;
               ofn.lCustData         = 0L ;
               ofn.lpfnHook          = NULL ;
               ofn.lpTemplateName    = NULL ;
               ofn.lpstrInitialDir = NULL;
               //set default value for i and j converting option
               convertop.ijrel=TRUE;
               return 0 ;
          case WM_COMMAND :
               switch (LOWORD(wParam))
                    {
                    case IDD_OPEN :
                         GetOption('2',szDirectory);
                         ofn.lpstrInitialDir = szDirectory;
                         ofn.lpstrFilter = szDXFFilter ;
                         ofn.lpstrDefExt = "dxf" ;
                         if (PopFileOpenDlg (hwnd, szFileName, szTitleName, &ofn))
                           {GetCurrentDirectory(_MAX_PATH,szDirectory);
                            SaveOption('2',szDirectory);
                            EnableWindow(GetDlgItem(hwnd,IDD_CONVERT), TRUE);
                            SetWindowText (GetDlgItem(hwnd, IDD_FILE), ofn.lpstrFileTitle) ;
                            sprintf(status,"");
                            SetWindowText(GetDlgItem(hwnd, IDD_STATUS),status);
                            while(iPrioCount>0) iPrioCount=DelPriority (iPrioCount, iPrioCount) ;
                            if(iLayerCount>0) free(layer);
                            iLayerCount=ReadLayer (ofn.lpstrFile) ;
                            SendMessage(GetDlgItem(hwnd, IDD_PRIORITY), LB_RESETCONTENT, 0, 0);
                            SendMessage(GetDlgItem(hwnd, IDD_LAYER), LB_RESETCONTENT, 0, 0);
                            if(iLayerCount>0)
                              {iPrioCount=NewPriority (iPrioCount) ;
                               SendMessage(GetDlgItem(hwnd, IDD_PRIORITY), LB_ADDSTRING, 0, (LPARAM) "1");
                               SendMessage(GetDlgItem(hwnd, IDD_PRIORITY), LB_SETCURSEL, (WPARAM) 0, 0);
                               for(i=0;i<iLayerCount;i++)
                                 {sprintf(szString,"%s...%d",layer[i].name,layer[i].priority);
                                  SendMessage(GetDlgItem(hwnd, IDD_LAYER), LB_ADDSTRING, 0, (LPARAM) szString);
                                 }
                               SendMessage(GetDlgItem(hwnd, IDD_LAYER), LB_SETCURSEL, (WPARAM) 0, 0);
                              }
                           }
                         break ;
                    case IDD_CONVERT :
                         sprintf(szDxfFile,"%s",ofn.lpstrFile);
                         GetOption('3',szDirectory);
                         fnsplit (ofn.lpstrFile, szDrive, szDir, szName, szExt) ;
                         sprintf(ofn.lpstrFile,"%s\\%s.nc",szDirectory,szName);
                         sprintf(ofn.lpstrFileTitle,"%s.nc",szName);
                         ofn.lpstrInitialDir = szDirectory;
                         ofn.lpstrFilter = szNCFilter ;
                         ofn.lpstrDefExt = "nc" ;
                         DialogBox (hInstance, "ConvertBox", hwnd, ConvertDlgProc) ;
                         if (PopFileSaveDlg (hwnd, szFileName, szTitleName, &ofn))
                           {GetCurrentDirectory(_MAX_PATH,szDirectory);
                            SaveOption('3',szDirectory);
                            sprintf(szToFileName,"%s",szFileName);
                            EnableWindow(GetDlgItem(hwnd,IDD_CONVERT), FALSE);
                            EnableWindow(GetDlgItem(hwnd,IDD_OPEN), FALSE);
                            EnableWindow(GetDlgItem(hwnd,IDD_LAYER), FALSE);
                            EnableWindow(GetDlgItem(hwnd,IDD_PRIORITY), FALSE);
                            CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE) convert,	NULL,	0,	&iID);
                           }
                         fnmerge (ofn.lpstrFile, szDrive, szDir, szName, szExt) ;
                         fnmerge (ofn.lpstrFileTitle, NULL, NULL, szName, szExt) ;
                         break ;
	    case IDD_SETUP:
                         DialogBox (hInstance, "SetupBox", hwnd, SetupDlgProc) ;
                         break ;
                    case IDD_HELP :
                         ShellExecute(hwnd, "open", szHelp, NULL, NULL, SW_SHOWDEFAULT);
                         break ;
                    case IDD_LAYER :
                         if(HIWORD (wParam)==LBN_DBLCLK)
                           {DialogBox (hInstance, "LayerBox", hwnd, LayerDlgProc) ;
                            i=SendMessage(GetDlgItem(hwnd, IDD_LAYER), LB_GETCURSEL, 0, 0);
                            SendMessage(GetDlgItem(hwnd, IDD_LAYER), LB_DELETESTRING, (WPARAM) i, 0);
                            sprintf(szString,"%s...%d",layer[i].name,layer[i].priority);
                            SendMessage(GetDlgItem(hwnd, IDD_LAYER), LB_INSERTSTRING, (WPARAM) i, (LPARAM) szString);
                            SendMessage(GetDlgItem(hwnd, IDD_LAYER), LB_SETCURSEL, (WPARAM) i, 0);
                            for(i=0;i<iLayerCount;i++)
                               if(layer[i].priority==iPrioCount+1)
                                 {iPrioCount=NewPriority(iPrioCount);
                                  sprintf(szString,"%d",iPrioCount);
                                  SendMessage(GetDlgItem(hwnd, IDD_PRIORITY), LB_ADDSTRING, 0, (LPARAM) szString);
                                 }
                           }
                         break ;
                    case IDD_PRIORITY :
                         if(HIWORD (wParam)==LBN_DBLCLK) DialogBox (hInstance, "PriorityBox", hwnd, PriorityDlgProc) ;
                         break ;
                    }
               return 0 ;
          case WM_DESTROY :
               if(demo==TRUE) DialogBox (hInstance, "EndBox", hwnd, EndDlgProc) ;
               PostQuitMessage (0) ;
               return 0 ;
          }
     return DefWindowProc (hwnd, iMsg, wParam, lParam) ;
     }

BOOL PopFileOpenDlg (HWND hwnd, PSTR pstrFileName, PSTR pstrTitleName, OPENFILENAME *ofn)
     {
     ofn->hwndOwner         = hwnd ;
     ofn->lpstrFile         = pstrFileName ;
     ofn->lpstrFileTitle    = pstrTitleName ;
     ofn->Flags             = OFN_HIDEREADONLY | OFN_CREATEPROMPT ;

     return GetOpenFileName (ofn) ;
     }

BOOL PopFileSaveDlg (HWND hwnd, PSTR pstrFileName, PSTR pstrTitleName, OPENFILENAME *ofn)
     {
     ofn->hwndOwner         = hwnd ;
     ofn->lpstrFile         = pstrFileName ;
     ofn->lpstrFileTitle    = pstrTitleName ;
     ofn->Flags             = OFN_OVERWRITEPROMPT ;

     return GetSaveFileName (ofn) ;
     }


BOOL CALLBACK SetupDlgProc (HWND hDlg, UINT iMsg, WPARAM wParam, LPARAM lParam)
     {
    char sztmp[20];
    char *stopconv;

     switch (iMsg)
          {
          case WM_INITDIALOG :
	SendMessage(GetDlgItem(hDlg,IDC_PRECISION_UPDOWN),UDM_SETRANGE,0,(LPARAM) MAKELONG ((short) 8, (short) 0));
        	SendMessage(GetDlgItem(hDlg,IDC_PRECISION_UPDOWN),UDM_SETPOS,0,(LPARAM) MAKELONG ((short) precision, 0));
	memset(sztmp,0,sizeof(sztmp));
	sprintf(sztmp,"%.6f",defaultzoffset);
	SetDlgItemText(hDlg,IDD_ZOFFSET_EDIT,sztmp);
	memset(sztmp,0,sizeof(sztmp));
	sprintf(sztmp,"%.6f",defaultmaxzpass);
	SetDlgItemText(hDlg,IDD_DEFMAXZPASS_EDIT,sztmp);
	memset(sztmp,0,sizeof(sztmp));
	sprintf(sztmp,"%.6f",defaultreleaseplane);
	SetDlgItemText(hDlg,IDD_DEFRELEASEPLANE_EDIT,sztmp);
	memset(sztmp,0,sizeof(sztmp));
	sprintf(sztmp,"%.6f",defaultcloseenough);
	SetDlgItemText(hDlg,IDD_DEFCLOSEENOUGH_EDIT,sztmp);
               return FALSE ;

          case WM_COMMAND :
               switch (LOWORD (wParam))
                    {
	       case IDD_OK:
		precision=SendMessage(GetDlgItem(hDlg,IDC_PRECISION_UPDOWN),UDM_GETPOS,0,0); 
		sprintf(sztmp,"%d",precision);
		SaveOption('4',sztmp);
		GetDlgItemText(hDlg,IDD_ZOFFSET_EDIT,sztmp,sizeof(sztmp));
		SaveOption('5',sztmp);
		defaultzoffset=strtod(sztmp,&stopconv);
		GetDlgItemText(hDlg,IDD_DEFMAXZPASS_EDIT,sztmp,sizeof(sztmp));
		SaveOption('6',sztmp);
		defaultmaxzpass=strtod(sztmp,&stopconv);
		GetDlgItemText(hDlg,IDD_DEFRELEASEPLANE_EDIT,sztmp,sizeof(sztmp));
		SaveOption('7',sztmp);
		defaultreleaseplane=strtod(sztmp,&stopconv);
		GetDlgItemText(hDlg,IDD_DEFCLOSEENOUGH_EDIT,sztmp,sizeof(sztmp));
		SaveOption('8',sztmp);
		defaultcloseenough=strtod(sztmp,&stopconv);
                         EndDialog (hDlg, 0) ;
                         return TRUE;
	       case IDD_CANCEL :
                         EndDialog (hDlg, 0) ;
                         return TRUE;
                    }
               break ;
          }
     return FALSE ;
     }




BOOL CALLBACK LayerDlgProc (HWND hDlg, UINT iMsg, WPARAM wParam, LPARAM lParam)
     {
     static int iLayer, iArc, iTemp, iPrioCount, i;
     static char szString[30], *endptr;
     static float fTemp;

     switch (iMsg)
          {
          case WM_INITDIALOG :
               iLayer=SendMessage(GetDlgItem(hWindow, IDD_LAYER), LB_GETCURSEL, 0, 0);
               SetWindowText(GetDlgItem(hDlg, IDD_LAYERDISP),layer[iLayer].name);
               if(layer[iLayer].status==FALSE) SendMessage(GetDlgItem(hDlg, IDD_LAYEROFF), BM_SETCHECK, 1, 0);
               CheckRadioButton(hDlg, IDD_EITHERARC, IDD_CWARC, (iArc=layer[iLayer].arc));
               sprintf(szString,"%f",layer[iLayer].zoffset);
               SetWindowText(GetDlgItem(hDlg, IDD_ZOFFSET),szString);
               sprintf(szString,"%f",layer[iLayer].depth);
               SetWindowText(GetDlgItem(hDlg, IDD_DEPTH),szString);
               SendMessage(GetDlgItem(hDlg, IDD_ZCHAR), CB_ADDSTRING, 0, (LPARAM) "W");
               SendMessage(GetDlgItem(hDlg, IDD_ZCHAR), CB_ADDSTRING, 0, (LPARAM) "Z");
               SendMessage(GetDlgItem(hDlg, IDD_ZCHAR), CB_ADDSTRING, 0, (LPARAM) "A");
               SendMessage(GetDlgItem(hDlg, IDD_ZCHAR), CB_ADDSTRING, 0, (LPARAM) "C");
               sprintf(szString,"%c",layer[iLayer].zchar);
               SendMessage(GetDlgItem(hDlg, IDD_ZCHAR), CB_SETCURSEL,
                           SendMessage(GetDlgItem(hDlg, IDD_ZCHAR), CB_FINDSTRINGEXACT, 0, (LPARAM) szString),0);
               iPrioCount=SendMessage(GetDlgItem(hWindow, IDD_PRIORITY), LB_GETCOUNT, 0, 0);
               for(i=0;i<=iPrioCount;i++)
                 {sprintf(szString,"%d",i+1);
                  SendMessage(GetDlgItem(hDlg, IDD_PRIOR), CB_ADDSTRING, 0, (LPARAM) szString);
                 }
               sprintf(szString,"%d",layer[iLayer].priority);
               SendMessage(GetDlgItem(hDlg, IDD_PRIOR), CB_SETCURSEL,
                           SendMessage(GetDlgItem(hDlg, IDD_PRIOR), CB_FINDSTRINGEXACT, 0, (LPARAM) szString),0); 
               return FALSE ;

          case WM_COMMAND :
               switch (LOWORD (wParam))
                    {
		              case IDD_OK :
                         layer[iLayer].status=1-SendMessage(GetDlgItem(hDlg, IDD_LAYEROFF), BM_GETCHECK, 0, 0);
                         layer[iLayer].arc=iArc;
                         GetWindowText(GetDlgItem(hDlg, IDD_ZOFFSET), szString, 30);
                         endptr=NULL;
                         fTemp=(float) strtod(szString, &endptr);
                         if(*endptr==NULL) layer[iLayer].zoffset=fTemp;
                         GetWindowText(GetDlgItem(hDlg, IDD_DEPTH), szString, 30);
                         endptr=NULL;
                         fTemp=(float) fabs(strtod(szString, &endptr));
                         if(*endptr==NULL) layer[iLayer].depth=fTemp;
                         SendMessage(GetDlgItem(hDlg, IDD_ZCHAR), CB_GETLBTEXT,
                                     SendMessage(GetDlgItem(hDlg, IDD_ZCHAR), CB_GETCURSEL, 0, 0), (LPARAM) szString);
                         layer[iLayer].zchar=szString[0];
                         SendMessage(GetDlgItem(hDlg, IDD_PRIOR), CB_GETLBTEXT,
                                     SendMessage(GetDlgItem(hDlg, IDD_PRIOR), CB_GETCURSEL, 0, 0), (LPARAM) szString);
                         endptr=NULL;
                         iTemp=(int) strtol(szString, &endptr, 10);
                         if(*endptr==NULL) layer[iLayer].priority=iTemp;
		              case IDD_CANCEL :
                         EndDialog (hDlg, 0) ;
                         return TRUE;
                    case IDD_EITHERARC :
                    case IDD_CCWARC :
                    case IDD_CWARC :
                         iArc=LOWORD (wParam);
                         CheckRadioButton(hDlg,IDD_EITHERARC,IDD_CWARC,LOWORD (wParam));
                         return TRUE;
                    }
               break ;
          }
     return FALSE ;
     }

HWND prehwnd, posthwnd;
char *szTemp1, *szTemp2;
BOOL CALLBACK PriorityDlgProc (HWND hDlg, UINT iMsg, WPARAM wParam, LPARAM lParam)
     {
     static int iPriority;
     static char szString[30], *endptr;
     static float fTemp;
     static WNDCLASSEX wndclass ;

     switch (iMsg)
          {
          case WM_INITDIALOG :
               iPriority=SendMessage(GetDlgItem(hWindow, IDD_PRIORITY), LB_GETCURSEL, 0, 0);
               sprintf(szString,"%d",iPriority+1);
               SetWindowText(GetDlgItem(hDlg, IDD_PRIODISP),szString);
               if(priority[iPriority].optimize==TRUE) SendMessage(GetDlgItem(hDlg, IDD_OPTIMIZE), BM_SETCHECK, 1, 0);
               sprintf(szString,"%f",priority[iPriority].release);
               SetWindowText(GetDlgItem(hDlg, IDD_RELEASE),szString);
               sprintf(szString,"%f",priority[iPriority].close);
               SetWindowText(GetDlgItem(hDlg, IDD_CLOSE),szString);
               wndclass.cbSize        = sizeof (wndclass) ;
               wndclass.style         = CS_HREDRAW | CS_VREDRAW ;
               wndclass.lpfnWndProc   = CodeWndProc ;
               wndclass.cbClsExtra    = 0 ;
               wndclass.cbWndExtra    = 0 ;
               wndclass.hInstance     = NULL ;
               wndclass.hIcon         = LoadIcon (hInst, szAppName) ;
               wndclass.hCursor       = LoadCursor (NULL, IDC_ARROW) ;
               wndclass.hbrBackground = (HBRUSH) GetStockObject (WHITE_BRUSH) ;
               wndclass.lpszMenuName  = NULL ;
               wndclass.lpszClassName = "code" ;
	            wndclass.hIconSm       = LoadIcon (hInst, szAppName) ;
               RegisterClassEx (&wndclass) ;
               szTemp1=(char *) calloc(strlen(priority[iPriority].precode)+1,sizeof(char));
               szTemp2=(char *) calloc(strlen(priority[iPriority].postcode)+1,sizeof(char)),
               sprintf(szTemp1,"%s",priority[iPriority].precode);
               sprintf(szTemp2,"%s",priority[iPriority].postcode);
               return FALSE ;

          case WM_COMMAND :
               switch (LOWORD (wParam))
                    {
                     case IDD_OK :
                         DestroyWindow(prehwnd);
                         DestroyWindow(posthwnd);
			                free(priority[iPriority].precode);
                         free(priority[iPriority].postcode);
                         priority[iPriority].precode=szTemp1;
                         priority[iPriority].postcode=szTemp2;
                         priority[iPriority].optimize=SendMessage(GetDlgItem(hDlg, IDD_OPTIMIZE), BM_GETCHECK, 0, 0);
                         GetWindowText(GetDlgItem(hDlg, IDD_RELEASE), szString, 30);
                         endptr=NULL;
                         fTemp=(float) strtod(szString, &endptr);
                         if(*endptr==NULL) priority[iPriority].release=fTemp;
                         GetWindowText(GetDlgItem(hDlg, IDD_CLOSE), szString, 30);
                         endptr=NULL;
                         fTemp=(float) fabs(strtod(szString, &endptr));
                         if(*endptr==NULL) priority[iPriority].close=fTemp;
                         EndDialog (hDlg, 0) ;
                         return TRUE ;
                    case IDD_CANCEL :
                         DestroyWindow(prehwnd);
                         DestroyWindow(posthwnd);
			                EndDialog (hDlg, 0) ;
                         return TRUE ;
                    case IDD_PRECODE :
                         if(prehwnd==NULL)
                           {prehwnd = CreateWindow ("code", "Pre-Priority Code",
                                                    WS_OVERLAPPEDWINDOW,
                                                    CW_USEDEFAULT, CW_USEDEFAULT,
                                                    CW_USEDEFAULT, CW_USEDEFAULT,
                                                    NULL, NULL, NULL, NULL) ;
                            SetWindowText(GetWindow(prehwnd, GW_CHILD), szTemp1);
                           }
                         ShowWindow (prehwnd, SW_SHOWNORMAL) ;
                         UpdateWindow (prehwnd) ;
                         return TRUE ;
                    case IDD_POSTCODE :
                         if(posthwnd==NULL)
                           {posthwnd = CreateWindow ("code", "Post-Priority Code",
                                                     WS_OVERLAPPEDWINDOW,
                                                     CW_USEDEFAULT, CW_USEDEFAULT,
                                                     CW_USEDEFAULT, CW_USEDEFAULT,
                                                     NULL, NULL, NULL, NULL) ;
                            SetWindowText(GetWindow(posthwnd, GW_CHILD), szTemp2);
                           }
                         ShowWindow (posthwnd, SW_SHOWNORMAL) ;
                         UpdateWindow (posthwnd) ;
                         return TRUE ;
                    }
               break ;
          }
     return FALSE ;
     }

LRESULT CALLBACK CodeWndProc (HWND hwnd, UINT iMsg, WPARAM wParam, LPARAM lParam)
     {
     static HWND hwndEdit ;
     static int iTemp;

     switch (iMsg)
          {
          case WM_CREATE :
               hwndEdit = CreateWindow ("edit", NULL,
                         WS_CHILD | WS_VISIBLE | WS_HSCROLL | WS_VSCROLL |
                              WS_BORDER | ES_LEFT | ES_MULTILINE |
                              ES_AUTOHSCROLL | ES_AUTOVSCROLL,
                         0, 0, 0, 0,
                         hwnd, (HMENU) 1,
                         ((LPCREATESTRUCT) lParam) -> hInstance, NULL) ;
               return 0 ;

          case WM_SETFOCUS :
               SetFocus (hwndEdit) ;
               return 0 ;

          case WM_SIZE : 
               MoveWindow (hwndEdit, 0, 0, LOWORD (lParam),
                                           HIWORD (lParam), TRUE) ;
               return 0 ;

          case WM_COMMAND :
               if (LOWORD (wParam) == 1)
                    if (HIWORD (wParam) == EN_ERRSPACE ||
                        HIWORD (wParam) == EN_MAXTEXT)
                              MessageBox (hwnd, "Edit control out of space.",
                                          szAppName, MB_OK | MB_ICONSTOP) ;
               return 0 ;
          case WM_DESTROY :
               if(prehwnd==hwnd)
                 {prehwnd=NULL;
                  free(szTemp1);
                  szTemp1=(char *) calloc((iTemp=(GetWindowTextLength(hwndEdit)+1)),sizeof(char));
                  GetWindowText(hwndEdit, szTemp1, iTemp);
                 }
               if(posthwnd==hwnd)
                 {posthwnd=NULL;
                  free(szTemp2);
                  szTemp2=(char *) calloc((iTemp=(GetWindowTextLength(hwndEdit)+1)),sizeof(char));
                  GetWindowText(hwndEdit, szTemp2, iTemp);
                 }
               return 0;
          }
     return DefWindowProc (hwnd, iMsg, wParam, lParam) ;
     }

BOOL CALLBACK ConvertDlgProc (HWND hDlg, UINT iMsg, WPARAM wParam, LPARAM lParam)
     {
     switch (iMsg)
          {
          case WM_INITDIALOG :
               SendMessage(GetDlgItem(hDlg, IDD_IJREL), BM_SETCHECK, convertop.ijrel, 0);
               SendMessage(GetDlgItem(hDlg, IDD_IJFIRST), BM_SETCHECK, convertop.ijfirst, 0);
               SendMessage(GetDlgItem(hDlg, IDD_LINENUM), BM_SETCHECK, convertop.line_num, 0);
               SendMessage(GetDlgItem(hDlg, IDD_ZONCHANGE), BM_SETCHECK, convertop.extraz, 0);
               return FALSE ;

          case WM_COMMAND :
               switch (LOWORD (wParam))
                    {
		              case IDD_OK :
                         convertop.ijrel=SendMessage(GetDlgItem(hDlg, IDD_IJREL), BM_GETCHECK, 0, 0);
                         convertop.ijfirst=SendMessage(GetDlgItem(hDlg, IDD_IJFIRST), BM_GETCHECK, 0, 0);
                         convertop.line_num=SendMessage(GetDlgItem(hDlg, IDD_LINENUM), BM_GETCHECK, 0, 0);
                         convertop.extraz=SendMessage(GetDlgItem(hDlg, IDD_ZONCHANGE), BM_GETCHECK, 0, 0);
                         EndDialog (hDlg, 0) ;
			                return TRUE ;
                    }
               break ;
          }
     return FALSE ;
     }

int DelPriority(int num, int count)
{
   int i;
   if(num>0 && num<=count) free(priority[num-1].precode);
   if(num>0 && num<=count) free(priority[num-1].postcode);
   for(i=num;i<count;i++) priority[i-1]=priority[i];
   if(count==1) free(priority);
   return count-1;
}

int NewPriority(int count)
{
   int i;
   temp_pri=(struct priority_obj *) calloc((count+1), sizeof(struct priority_obj));
   for(i=0;i<count;i++) temp_pri[i]=priority[i];
   if(count>0) free(priority);
   priority=temp_pri;
   priority[count].release=defaultreleaseplane;
   priority[count].close=defaultcloseenough;
   priority[count].optimize=defaultpriorityoptimization;
   priority[count].precode=(char *) calloc(1, sizeof(char));
   priority[count].postcode=(char *) calloc(1, sizeof(char));
   return count+1;
}

int ReadLayer (char *file)
{
   int i, count=0, temp;
   char string[100];
   fpos_t pos;
   FILE *ifp;
   ifp=fopen(file,"r");
   while(1)
     {if((temp=fscanf(ifp,"%s",string))!=EOF && temp!=0 && strcmp(string,"0")==0)
        {fgetpos(ifp, &pos);
         if((temp=fscanf(ifp,"%s",string))!=EOF && temp!=0 && strcmp(string,"LAYER")==0)
           {while(1)
              {if((temp=fscanf(ifp,"%s",string))==EOF || temp==0) break;
               if(strcmp(string,"0")==0) break;
               else if(strcmp(string,"2")==0)
                 {temp_lay=(struct layer_obj *) calloc(++count, sizeof(struct layer_obj));
                  for(i=0;i<count-1;i++) temp_lay[i]=layer[i];
                  if(count>1) free(layer);
                  layer=temp_lay;
                  temp=fscanf(ifp,"%s",layer[count-1].name);
                  break;
                 }
               else if((temp=fscanf(ifp,"%s",string))==EOF || temp==0) break;
              }
           }
         else fsetpos(ifp, &pos);
        }
      if(temp==0 || temp==EOF) break;
     }
   if(count==0)
     {layer=(struct layer_obj *) calloc(++count, sizeof(struct layer_obj));
      sprintf(layer[0].name,"0");
     }
   for(i=0;i<count;i++)
     {layer[i].status=TRUE;
      layer[i].priority=1;
      layer[i].depth=defaultmaxzpass;
      layer[i].arc=IDD_EITHERARC;
      layer[i].zchar='Z';
      layer[i].zoffset=defaultzoffset;
     }
   fclose(ifp);
   return count;
}

BOOL CALLBACK EndDlgProc (HWND hDlg, UINT iMsg, WPARAM wParam, LPARAM lParam)
     {
     switch (iMsg)
          {
          case WM_COMMAND :
               switch (LOWORD (wParam))
                    {
		              case IDD_OK :
                         EndDialog(hDlg, 0);
                         return TRUE ;
                    }
               break;
         }
     return FALSE ;
     }

void GetOption(char num, char dir[])
{
   int iTemp;
   static char szWinDir[_MAX_PATH], szAceIni[_MAX_PATH], szString[100];
   FILE *ifp;

//   GetWindowsDirectory(szWinDir, _MAX_PATH);
   current_directory(szWinDir);
   sprintf(szAceIni,"%s\\ace.ini",szAcePath);
   if((ifp=fopen(szAceIni,"r"))!=NULL)
     {while((iTemp=fscanf(ifp,"%s",szString))!=EOF && strcmp(szString,"[1]")!=0);
      if(iTemp!=EOF)
        {iTemp=fgetc(ifp);
         while(1)
           {while(iTemp!=EOF && iTemp!='\n' && iTemp!='[') iTemp=fgetc(ifp);
            if(iTemp==EOF || iTemp=='[') break;
            iTemp=fgetc(ifp);
            if(iTemp==num && (iTemp=fgetc(ifp))=='=')
              {fgets(dir,_MAX_PATH,ifp);
               iTemp=strlen(dir);
               if(iTemp>1) dir[iTemp-1]=0;
               if(iTemp>2 && dir[iTemp-2]=='\\') dir[iTemp-2]=0;
               break;
              }
           }
        }
      fclose(ifp);
     }
}

void SaveOption(char num, char dir[])
{
   int iTemp;
   static char szWinDir[_MAX_PATH], szAceIni[_MAX_PATH], szString[_MAX_PATH];
   FILE *ifp, *ofp;

//   GetWindowsDirectory(szWinDir, _MAX_PATH);
   current_directory(szWinDir);
   sprintf(szAceIni,"%s\\ace.ini",szAcePath);
   ofp=fopen("temp.tmp","w");
   if((ifp=fopen(szAceIni,"r"))!=NULL)
     {while(1)
        {if((iTemp=fgetc(ifp))=='[')
           {fputc(iTemp,ofp);
            if((iTemp=fgetc(ifp))=='1')
              {fputc(iTemp,ofp);
               if((iTemp=fgetc(ifp))==']')
                 {fputc(iTemp,ofp);
                  break;
                 }
              }
           }
         if(iTemp==EOF) break;
         fputc(iTemp, ofp);
        }
      if(iTemp!=EOF)
        {iTemp=fgetc(ifp);
         while(1)
           {while(iTemp!=EOF && iTemp!='\n') {fputc(iTemp,ofp); iTemp=fgetc(ifp);}
            if(iTemp==EOF) break;
            fputc(iTemp,ofp); iTemp=fgetc(ifp);
            if(iTemp==num)
              {fputc(iTemp,ofp); iTemp=fgetc(ifp);
               if(iTemp=='=')
                 {fputc(iTemp,ofp);
                  fgets(szString,_MAX_PATH,ifp);
                  fprintf(ofp,"%s\n",dir);
                  iTemp=fgetc(ifp);
                 }
              }
           }
        }
      fclose(ifp);
     }
   fclose(ofp);
   ifp=fopen("temp.tmp","r");
   ofp=fopen(szAceIni,"w");
   while((iTemp=fgetc(ifp))!=EOF) fputc(iTemp, ofp);
   fclose(ifp);
   fclose(ofp);
   remove("temp.tmp");
}

char *current_directory(char *path)
{
  strcpy(path, "X:\\");      /* fill string with form of response: X:\ */
  path[0] = 'A' + getdisk();    /* replace X with current drive letter */
  getcurdir(0, path+3);  /* fill rest of string with current directory */
  return(path);
}


