VERSION 5.00
Begin VB.Form Form1 
   Caption         =   "Demo"
   ClientHeight    =   2805
   ClientLeft      =   60
   ClientTop       =   450
   ClientWidth     =   5700
   LinkTopic       =   "Form1"
   ScaleHeight     =   2805
   ScaleWidth      =   5700
   StartUpPosition =   3  'Windows Default
   Begin VB.CommandButton Command3 
      Caption         =   "MATH"
      Height          =   615
      Left            =   4080
      TabIndex        =   2
      Top             =   1680
      Width           =   1215
   End
   Begin VB.CommandButton Command2 
      Caption         =   "CH2"
      Height          =   615
      Left            =   2280
      TabIndex        =   1
      Top             =   1680
      Width           =   1215
   End
   Begin VB.CommandButton Command1 
      Caption         =   "CH1"
      Height          =   615
      Left            =   480
      TabIndex        =   0
      Top             =   1680
      Width           =   1215
   End
   Begin VB.Label Label1 
      Caption         =   "��"
      BeginProperty Font 
         Name            =   "MS Sans Serif"
         Size            =   18
         Charset         =   0
         Weight          =   400
         Underline       =   0   'False
         Italic          =   0   'False
         Strikethrough   =   0   'False
      EndProperty
      ForeColor       =   &H00800080&
      Height          =   375
      Index           =   2
      Left            =   4440
      TabIndex        =   5
      Top             =   720
      Width           =   375
   End
   Begin VB.Label Label1 
      Caption         =   "��"
      BeginProperty Font 
         Name            =   "MS Sans Serif"
         Size            =   18
         Charset         =   0
         Weight          =   400
         Underline       =   0   'False
         Italic          =   0   'False
         Strikethrough   =   0   'False
      EndProperty
      ForeColor       =   &H00FF0000&
      Height          =   375
      Index           =   1
      Left            =   2640
      TabIndex        =   4
      Top             =   720
      Width           =   375
   End
   Begin VB.Label Label1 
      Caption         =   "��"
      BeginProperty Font 
         Name            =   "MS Sans Serif"
         Size            =   18
         Charset         =   0
         Weight          =   400
         Underline       =   0   'False
         Italic          =   0   'False
         Strikethrough   =   0   'False
      EndProperty
      ForeColor       =   &H0000FFFF&
      Height          =   375
      Index           =   0
      Left            =   840
      TabIndex        =   3
      Top             =   720
      Width           =   375
   End
End
Attribute VB_Name = "Form1"
Attribute VB_GlobalNameSpace = False
Attribute VB_Creatable = False
Attribute VB_PredeclaredId = True
Attribute VB_Exposed = False
Private Declare Function ReadUSB Lib "RigolTMCUsb_UI.dll" (ByVal Index As Long, ByVal lpszLength As Long, ByVal lpszBuffer As Long) As Boolean
Private Declare Function WriteUSB Lib "RigolTMCUsb_UI.dll" (ByVal Index As Long, ByVal msgid As Long, ByVal tag As Long, ByVal length As Long, ByVal lpszBuffer As Long) As Boolean
Private Declare Function GetTMCDeviceNum Lib "RigolTMCUsb_UI.dll" () As Long

Private Sub Command1_Click()

   Dim i As Long
   Dim sendbuf As String
   Dim readbuf As String
   
   rSize = 100
   sendbuf = ":CHAN1:DISP?"
   
   '��ʼ��USB�豸
   i = GetTMCDeviceNum
   
   '����ѯ���������ȡ����ֵ
   Call SendToUSB(sendbuf)
   readbuf = Read_USB()
   
   '���ݷ���ֵ��ȷ����Ӧͨ���Ƶ�״̬
   If readbuf = "ON" Then
         
   ' ������������
   sendbuf = ":CHAN1:DISP 0"
   Call SendToUSB(sendbuf)
   
   Label1(0).ForeColor = &H808080  '��ɫ
   
   Else
   
   sendbuf = ":CHAN1:DISP 1"
   Call SendToUSB(sendbuf)
   Label1(0).ForeColor = &HFFFF&  '��ɫ
   
   End If
  

End Sub

'������������
Sub SendToUSB(cmd As String)
    Dim retcode As Boolean
    Dim send_buf(256) As Byte
    Dim temp As Long
    Dim cmdstr As String
    
    cmdstr = cmd

    temp = Len(cmdstr)
    For i = 0 To temp - 1
        tempStr = Mid(cmdstr, i + 1, 1)
        send_buf(i) = Asc(tempStr)
    Next i
    '��ʾ������������
    '�ӿڵĶ�����Rigolusb.h��ע��
    retcode = WriteUSB(0, 1, 1, Len(cmdstr), VarPtr(send_buf(0)))
End Sub

'��ȡ����ֵ
Public Function Read_USB() As String

    Dim retcode As Boolean
    Dim rcv_buffer(256) As Byte
    Dim tmpstr As String
    Dim i, size As Long
    Dim rSize As Long

    rSize = 100
    tmpstr = ""

    '��ʾ��������ȡֵ������ʹ��������Դ�ʾ���������ֵ
    retcode = WriteUSB(0, 2, 2, 256, 0)
    '��ʾ�����ж�ȡ����ֵ
    retcode = ReadUSB(0, VarPtr(rSize), VarPtr(rcv_buffer(0)))
        
    For i = 0 To (rSize - 1)
        tmpstr = tmpstr + Chr(rcv_buffer(i))
    Next i
                           
    Read_USB = tmpstr
    
End Function

Private Sub Command2_Click()

   Dim i As Long
   Dim sendbuf As String
   Dim readbuf As String
   
   rSize = 100
   sendbuf = ":CHAN2:DISP?"
   
   i = GetTMCDeviceNum
   
   Call SendToUSB(sendbuf)
   readbuf = Read_USB()
   
   If readbuf = "ON" Then
         
   ' ������������
   sendbuf = ":CHAN2:DISP 0"
   Call SendToUSB(sendbuf)
   
   Label1(1).ForeColor = &H808080  '��ɫ
   
   Else
   
   sendbuf = ":CHAN2:DISP 1"
   Call SendToUSB(sendbuf)
   Label1(1).ForeColor = &HFF0000     '��ɫ
   
   End If
End Sub

Private Sub Command3_Click()
   Dim i As Long
   Dim sendbuf As String
   Dim readbuf As String
   
   rSize = 100
   sendbuf = ":MATH:DISP?"
   
   i = GetTMCDeviceNum
   
   Call SendToUSB(sendbuf)
   readbuf = Read_USB()
   
   If readbuf = "ON" Then
         
   ' ������������
   sendbuf = ":MATH:DISP 0"
   Call SendToUSB(sendbuf)
   
   Label1(2).ForeColor = &H808080  '��ɫ
   
   Else
   
   sendbuf = ":MATH:DISP 1"
   Call SendToUSB(sendbuf)
   Label1(2).ForeColor = &H800080        '��ɫ
   
   End If
End Sub

Private Sub Form_Load()
   Dim i As Long
   Dim sendbuf As String
   Dim readbuf As String
        
   rSize = 100
   sendbuf = ":CHAN1:DISP?"
   
   i = GetTMCDeviceNum
   
   Call SendToUSB(sendbuf)
   readbuf = Read_USB()
   
   If readbuf = "ON" Then
   
   Label1(0).ForeColor = &HFFFF&  '��ɫ
   
   Else

   Label1(0).ForeColor = &H808080  '��ɫ
   
   End If
   
   
   sendbuf = ":CHAN2:DISP?"
   
   i = GetTMCDeviceNum
   
   Call SendToUSB(sendbuf)
   readbuf = Read_USB()
   
   If readbuf = "ON" Then
         
   Label1(1).ForeColor = &HFF0000     '��ɫ
   
   Else
   
   Label1(1).ForeColor = &H808080  '��ɫ
   
   End If
   
   sendbuf = ":MATH:DISP?"
   
   i = GetTMCDeviceNum
   
   Call SendToUSB(sendbuf)
   readbuf = Read_USB()
   
   If readbuf = "ON" Then
          
   Label1(2).ForeColor = &H800080     '��ɫ
   
   Else
    
   Label1(2).ForeColor = &H808080  '��ɫ
  
   End If
   
End Sub
