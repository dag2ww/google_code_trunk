/*****************************************************************************
北京普源精电科技有限公司   版权所有(2005-2010)              
******************************************************************************

  **    文件名:      rigolusb.h    
  **    版  本:      1.0.0.1
  **	作  者:      HUANGZHEN
  **	创建日期:    2005-8-9
  **	描  述:      usb函数接口的定义
  **    说  明:   
  **
  **
  **	修改人:    
  **	修改日期:  
  **	修改内容:
  
*****************************************************************************/
#ifndef _RIGOL_USB_H_INCLUDED_
#define _RIGOL_USB_H_INCLUDED_

#ifndef _RIGOLUSB_API
#define RIGOLUSB_API  extern "C" __declspec(dllimport)
#else
#define RIGOLUSB_API  extern "C" __declspec(dllexport)
#endif

/*------------------------------ 外部变量声明 --------------------------------*/

/*------------------------------- 局部常数和类型定义 --------------------------*/

/*------------------------------- 局部宏定义 ---------------------------------*/

/*------------------------------- 局部变量 -----------------------------------*/

/*------------------------------- 局部函数原形 -------------------------------*/

/*------------------------------- 函数实现 -----------------------------------*/

/*******************************************************************************

  **  函数名称:     ReadUSB  
  **  输入参数
  **  devIndex ---  设备编号
  **  wLength  ---  需要从USB端口读取数据的长度（字节为单位）
  **  pBuffer  ---  接收返回数据的存储空间的指针（应用程序端开辟该存储空间）                                  
  **
  **  输出参数:
  **  返回类型 ---  BOOL类型
  **
  **  描    述:     读USB端口，使用批量传输从设备读取数据
********************************************************************************/
RIGOLUSB_API BOOL __stdcall  ReadUSB(ULONG devIndex,
									 ULONG* wLength,
								     PUCHAR pBuffer);

/*******************************************************************************

  **  函数名称:     WriteUSB  
  **  输入参数 
  **  devIndex ---  设备编号
  **  MsgID    ---  需要的消息id号
  **  bTag     ---  传输标志
  **  wLength  ---  需要从USB端口读取数据的长度（字节为单位）
  **  pBuffer  ---  应用程序端开辟该存储空间用于存放要写入设备的数据                           
  **
  **  输出参数:
  **  返回类型 ---  BOOL类型
  **
  **  描    述:     写USB端口，使用批量传输向设备写入数据
********************************************************************************/
RIGOLUSB_API BOOL __stdcall WriteUSB(ULONG devIndex,
									 UCHAR MsgID,
									 UCHAR bTag,
									 ULONG wLength,
								     PUCHAR pBuffer);


/*******************************************************************************

  **  函数名称:      Class_Request_Write
  **  输入参数 
  **  devIndex  ---  设备编号
  **  bRequest  ---  向USB设备发送的类请求号（由设备定义）
  **  wValue    ---  和请求格式的wValue相对应
  **  wIndex    ---  和请求格式的wIndex相对应
  **  wLength   ---  和请求格式的wLength相对应                     
  **  bEndPoint ---  表示是否通过端点,如果为接口则为false
  **  输出参数:
  **  返回类型  ---   BOOL类型
  **
  **  描    述:       发送命令——控制传输,通过控制端点0发送,建立相应的类型请求
********************************************************************************/
RIGOLUSB_API BOOL __stdcall Class_Request_Write(ULONG devIndex,
												UCHAR bRequest,
												USHORT wValue,
												USHORT wIndex,
												USHORT wLength,
												BOOL   bEndPoint);

/*******************************************************************************

  **  函数名称:      Class_Request_Read
  **  输入参数 
  **  devIndex  ---  设备编号
  **  bRequest  ---  向USB设备发送的类请求号（由设备定义）
  **  wValue    ---  和请求格式的wValue相对应
  **  wIndex    ---  和请求格式的wIndex相对应
  **  wLength   ---  和请求格式的wLength相对应
  **  pBuffer   ---  数据缓冲区                        
  **  bEndPoint ---  表示是否通过端点,如果为接口则为false
  **  输出参数:
  **  返回类型 ---   BOOL类型
  **
  **  描    述:      读取数据——控制传输,通过控制端点0发送,读取相应的状态信息
********************************************************************************/
RIGOLUSB_API BOOL __stdcall Class_Request_Read(ULONG devIndex,
											   UCHAR bRequest,
											   USHORT wValue,
											   USHORT wIndex,
											   USHORT wLength,
											   PUCHAR pBuffer,
											   BOOL   bEndPoint);


/*******************************************************************************

  **  函数名称:      Vendor_Request_Write  
  **  输入参数
  **  devIndex  ---  设备编号
  **  bRequest  ---  向USB设备发送的vendor请求号（由设备定义）
  **  wValue    ---  和请求格式的wValue相对应
  **  wIndex    ---  和请求格式的wIndex相对应
  **  wLength   ---  和请求格式的wLength相对应                     
  **
  **  输出参数:
  **  返回类型  ---  BOOL类型
  **
  **  描    述:      发送命令——控制传输,通过控制端点0发送,建立相应的厂商请求
********************************************************************************/
RIGOLUSB_API BOOL __stdcall Vendor_Request_Write(ULONG devIndex,
												 UCHAR bRequest,
												 USHORT wValue,
												 USHORT wIndex,
												 USHORT wLength);


/*******************************************************************************

  **  函数名称:      Vendor_Request_Read
  **  输入参数 
  **  devIndex  ---  设备编号
  **  bRequest  ---  向USB设备发送的vendor请求号（由设备定义）
  **  wValue    ---  和请求格式的wValue相对应
  **  wIndex    ---  和请求格式的wIndex相对应
  **  wLength   ---  和请求格式的wLength相对应
  **  pBuffer   ---  数据缓冲区                        
  **
  **  输出参数:
  **  返回类型  ---  BOOL类型
  **
  **  描    述:      读取数据——控制传输,通过控制端点0发送,读取状态数据信息
********************************************************************************/
RIGOLUSB_API BOOL __stdcall Vendor_Request_Read(ULONG devIndex,
												UCHAR bRequest,
												USHORT wValue,
												USHORT wIndex,
												USHORT wLength,
												PUCHAR pBuffer);

/*******************************************************************************

  **  函数名称:      USB_Clear_Feature  
  **  输入参数
  **  devIndex  ---  设备编号 
  **  bRequest  ---  向USB设备发送的vendor请求号（由设备定义）
  **  wValue    ---  和请求格式的wValue相对应
  **  wIndex    ---  和请求格式的wIndex相对应
  **  wLength   ---  和请求格式的wLength相对应                     
  **
  **  输出参数:
  **  返回类型  ---  BOOL类型
  **
  **  描    述:      发送命令——控制传输,通过控制端点0发送,建立标准请求
********************************************************************************/
RIGOLUSB_API BOOL __stdcall USB_Clear_Feature(ULONG devIndex,
											  UCHAR bRequest,
											  USHORT wValue,
											  USHORT wIndex,
											  USHORT wLength);

//获取当前系统中的设备数目
RIGOLUSB_API INT  __stdcall  GetTMCDeviceNum();

/*********************************************************************************

  **  函数名称:      ReadAllTMCDeviceInfo
  **  输入参数 
  **  pInfoBuf  ---  指向用户开辟的存储空间的指针。存储空间为二维数组，其中行数
                     为最大设备数MAX_DEVICE(不大于127),列数为最大设备信息的长度MAXINFOLENGTH = 100.
  **
  **  输出参数:
  **  返回类型  ---  VOID
  **
  **  描    述:      发送“*IDN?”读取所有连接的TMC设备的设备信息，与设备句柄对应。
**********************************************************************************/
RIGOLUSB_API VOID __stdcall ReadAllTMCDeviceInfo(PUCHAR pInfoBuf);

/*********************************************************************************

  **  函数名称:      ChooseTMCDevice
  **  输入参数 
  **  CurDevInfo  ---  指向当前选中的设备信息的char类型指针
  **  pInfoBuf    ---  指向用户开辟的存储空间的指针。存储空间为二维数组，其中行数
                       为最大设备数MAX_DEVICE(不大于127),列数为最大设备信息的长度MAXINFOLENGTH = 100.
  **
  **  输出参数:
  **  返回类型    ---  ULONG类型
  **
  **  描    述:   返回当前选中的设备的设备编号   
**********************************************************************************/
RIGOLUSB_API ULONG __stdcall ChooseTMCDevice(PCHAR CurDevInfo,PUCHAR pInfobuf);

#endif

