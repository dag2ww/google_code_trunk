VERSION 5.00
Begin VB.Form Form1 
   BackColor       =   &H8000000A&
   Caption         =   "Demo"
   ClientHeight    =   4230
   ClientLeft      =   60
   ClientTop       =   450
   ClientWidth     =   11010
   FillColor       =   &H000000FF&
   LinkTopic       =   "Form1"
   ScaleHeight     =   4230
   ScaleWidth      =   11010
   StartUpPosition =   3  'Windows Default
   Begin VB.PictureBox Picture1 
      BackColor       =   &H80000006&
      Height          =   2560
      Left            =   360
      ScaleHeight     =   2505
      ScaleWidth      =   10185
      TabIndex        =   2
      Top             =   480
      Width           =   10240
   End
   Begin VB.CommandButton Command2 
      Caption         =   "CH2"
      Height          =   615
      Left            =   6000
      TabIndex        =   1
      Top             =   3360
      Width           =   1695
   End
   Begin VB.CommandButton Command1 
      Caption         =   "CH1"
      Height          =   615
      Left            =   2760
      TabIndex        =   0
      Top             =   3360
      Width           =   1695
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
   Dim DataLen As Long
   Dim stepW As Integer
   Dim stepH As Integer
      
   sendbuf = ":WAV:DATA? CHAN1"
   
   '初始化USB设备
   i = GetTMCDeviceNum
   
   '发送询问命令，并读取返回值
   Call SendToUSB(sendbuf)
   DataLen = Read_USB()
    
   stepW = Picture1.Width
   stepH = Picture1.Height / 256
   
   '清除波形
   Picture1.Cls
      
   For i = 0 To DataLen - 1 'Picture1.Width
   
   '将波形数据点连接，并将波形设置为黄色
   Picture1.Line ((i * stepW / DataLen), (rcv_buffer(i)) * stepH)-(((i + 1) * stepW / DataLen), (rcv_buffer((i + 1))) * stepH), &HFFFF&
   Next i
       

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
Public Function Read_USB() As Long

    Dim retcode As Boolean
    Dim i, size As Long
    Dim rSize As Long

    rSize = 100

    '向示波器发送取值命令，发送此命令后可以从示波器获得数值
    retcode = WriteUSB(0, 2, 2, 10240, 0)
    '从示波器中读取返回值
    retcode = ReadUSB(0, VarPtr(rSize), VarPtr(rcv_buffer(0)))
    
    Read_USB = rSize
    
End Function


Private Sub Command2_Click()

   Dim i As Long
   Dim sendbuf As String
   Dim DataLen As Long
   Dim stepW As Integer
   Dim stepH As Integer
      
   sendbuf = ":WAV:DATA? CHAN2"
   
   '初始化USB设备
   i = GetTMCDeviceNum
   
   '发送询问命令，并读取返回值
   Call SendToUSB(sendbuf)
   DataLen = Read_USB()
    
   stepW = Picture1.Width
   stepH = Picture1.Height / 256
   
   Picture1.Cls
      
   For i = 0 To DataLen - 1
   
   Picture1.Line ((i * stepW / DataLen), (rcv_buffer(i)) * stepH)-(((i + 1) * stepW / DataLen), (rcv_buffer((i + 1))) * stepH), &HFFFF00
   Next i

End Sub
