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
      Caption         =   "■"
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
      Caption         =   "■"
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
      Caption         =   "■"
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
   
   '初始化USB设备
   i = GetTMCDeviceNum
   
   '发送询问命令，并读取返回值
   Call SendToUSB(sendbuf)
   readbuf = Read_USB()
   
   '根据返回值来确定对应通道灯的状态
   If readbuf = "ON" Then
         
   ' 发送设置命令
   sendbuf = ":CHAN1:DISP 0"
   Call SendToUSB(sendbuf)
   
   Label1(0).ForeColor = &H808080  '灰色
   
   Else
   
   sendbuf = ":CHAN1:DISP 1"
   Call SendToUSB(sendbuf)
   Label1(0).ForeColor = &HFFFF&  '黄色
   
   End If
  

End Sub

'发送设置命令
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
    '向示波器发送命令
    '接口的定义在Rigolusb.h中注明
    retcode = WriteUSB(0, 1, 1, Len(cmdstr), VarPtr(send_buf(0)))
End Sub

'获取返回值
Public Function Read_USB() As String

    Dim retcode As Boolean
    Dim rcv_buffer(256) As Byte
    Dim tmpstr As String
    Dim i, size As Long
    Dim rSize As Long

    rSize = 100
    tmpstr = ""

    '向示波器发送取值命令，发送此命令后可以从示波器获得数值
    retcode = WriteUSB(0, 2, 2, 256, 0)
    '从示波器中读取返回值
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
         
   ' 发送设置命令
   sendbuf = ":CHAN2:DISP 0"
   Call SendToUSB(sendbuf)
   
   Label1(1).ForeColor = &H808080  '灰色
   
   Else
   
   sendbuf = ":CHAN2:DISP 1"
   Call SendToUSB(sendbuf)
   Label1(1).ForeColor = &HFF0000     '蓝色
   
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
         
   ' 发送设置命令
   sendbuf = ":MATH:DISP 0"
   Call SendToUSB(sendbuf)
   
   Label1(2).ForeColor = &H808080  '灰色
   
   Else
   
   sendbuf = ":MATH:DISP 1"
   Call SendToUSB(sendbuf)
   Label1(2).ForeColor = &H800080        '紫色
   
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
   
   Label1(0).ForeColor = &HFFFF&  '黄色
   
   Else

   Label1(0).ForeColor = &H808080  '灰色
   
   End If
   
   
   sendbuf = ":CHAN2:DISP?"
   
   i = GetTMCDeviceNum
   
   Call SendToUSB(sendbuf)
   readbuf = Read_USB()
   
   If readbuf = "ON" Then
         
   Label1(1).ForeColor = &HFF0000     '蓝色
   
   Else
   
   Label1(1).ForeColor = &H808080  '灰色
   
   End If
   
   sendbuf = ":MATH:DISP?"
   
   i = GetTMCDeviceNum
   
   Call SendToUSB(sendbuf)
   readbuf = Read_USB()
   
   If readbuf = "ON" Then
          
   Label1(2).ForeColor = &H800080     '紫色
   
   Else
    
   Label1(2).ForeColor = &H808080  '灰色
  
   End If
   
End Sub
