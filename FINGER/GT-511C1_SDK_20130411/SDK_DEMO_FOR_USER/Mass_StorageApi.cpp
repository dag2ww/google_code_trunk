#include "Shlwapi.h"
#include <windows.h>
#include <winioctl.h>
#include <stdio.h>
#include "Mass_StorageApi.h"



HANDLE hCommand,hImage;
char* cUSBDevice;
char Path_Buff[64];
char Path_Buff1[64];
char s[64];
char chr[8];
char bRet;


short MyConnect(void)
{
	unsigned char buffer[64];
	char i;
	unsigned long num;
	int rtn;
	for(i = 0; i < 27; i++)
	{
		chr[0] = 0x41 + i;
		chr[1] = 0x3a;
		chr[2] = 0x00;
		rtn = ShowDriveInfo( chr );
		if(rtn == Mass_Storage)
		{
			memcpy(Path_Buff,chr,2);
			strcpy(Path_Buff + 2,"\\\\README.BIN");
			hCommand = CreateFile(Path_Buff,  // drive to open
												GENERIC_READ|GENERIC_WRITE,
												0,
												0,
												OPEN_EXISTING,
												FILE_ATTRIBUTE_NORMAL | FILE_FLAG_NO_BUFFERING,// FILE_FLAG_NO_BUFFERING close readfile cache
												0);

			if (hCommand == INVALID_HANDLE_VALUE) // cannot open the drive
			{
				return FAIL;
			}
			//FlushFileBuffers(hCommand);
			SetFilePointer(hCommand,0,NULL,FILE_BEGIN);

			memcpy(Path_Buff1,chr,2);
			strcpy(Path_Buff1 + 2,"\\\\IMAGE.BIN");
			hImage = CreateFile(Path_Buff1,  // drive to open
											GENERIC_READ|GENERIC_WRITE,
											0,
											0,
											OPEN_EXISTING,
											FILE_ATTRIBUTE_NORMAL | FILE_FLAG_NO_BUFFERING,// FILE_FLAG_NO_BUFFERING close readfile cache
											0);

			if (hImage == INVALID_HANDLE_VALUE) // cannot open the drive
			{
				return FAIL;
			}
			//FlushFileBuffers(hImage);
			return OK;
		}
	}
	return FAIL;
}

short MyDisconnect(void)
{
	CloseHandle(hCommand);
	CloseHandle(hImage);
	return OK;
}

short SendCommand(unsigned char ucData[],unsigned int uiLen)
{
	DWORD dwError;	
	DWORD dwWriteSize = 0;
	long lDistance = 0;

	SetFilePointer(hCommand,lDistance,NULL,FILE_BEGIN);
	bRet = WriteFile(hCommand,ucData,uiLen,&dwWriteSize,NULL);
	dwError = GetLastError();
	
	if(bRet &&dwError == 0)
	{
		return OK;
	}
	else
	{
		return (short)dwError;
	}
}

short ReceiveCommand(unsigned char *ucBuffer, unsigned int uiLen)
{
	DWORD dwError;	
	long lDistance = 0;
	DWORD dwReadSize = 0;
	DWORD TimerCount;
	int i;
	int tmp = 0;
	FILETIME lpCreationTime;

	FILETIME lpLastAccessTime;

	FILETIME lpLastWriteTime;

	DWORD lpEvtMask;
	OVERLAPPED lpOverlapped;


	TimerCount = GetTickCount();
	if(uiLen <= 512)
	{
		//FlushFileBuffers(hCommand);
		//SetFilePointer(hCommand,lDistance,NULL,FILE_BEGIN);

		while(uiLen != dwReadSize && GetTickCount() - TimerCount < 5000)
		{
			//FlushFileBuffers(hCommand);
			SetFilePointer(hCommand,lDistance,NULL,FILE_BEGIN);
			bRet = ReadFile(hCommand,ucBuffer,uiLen,&dwReadSize,NULL);
			for(i = 10;i < 64; i++)
				tmp += *(ucBuffer + i);
			if(tmp == 0x00)
			{
				Sleep(100);
				dwReadSize = 0;
				tmp =0;
			}
		}
		if(GetTickCount() - TimerCount >= 5000)
			return FAIL;
		dwError = GetLastError();
		if(bRet &&dwError == 0)
		{
			//FlushFileBuffers(hCommand);
			//dwError = GetLastError();
			return OK;
		}
		else
		{
			//FlushFileBuffers(hCommand);
			//dwError = GetLastError();
			return (short)dwError;
		}
	}
	else
	{
		//TimerCount = GetTickCount();
		
		//while(uiLen != dwReadSize)
		{
			//FlushFileBuffers(hImage);
			SetFilePointer(hImage,lDistance,NULL,FILE_BEGIN);
			bRet = ReadFile(hImage,ucBuffer,uiLen,&dwReadSize,NULL);
		}
		dwError = GetLastError();
		if(bRet &&dwError == 0)
		{
			//FlushFileBuffers(hImage);
			return OK;
		}
		else
		{
			//FlushFileBuffers(hImage);
			return (short)dwError;
		}
	}
	return FAIL;
}
short ShowDriveInfo(char* drive)
{
	UINT result;
	int rtn;
	 DWORD   dwVolumeSerialNumber     = 0;   
	   DWORD   dwMaximumComponentLength = 0;   
	   DWORD   dwFileSystemFlags        = 0;  
	   char* strRootPath              = drive;
	   char  strVolumeName[64];   
	   char  strFileSysName[64];  
	result = GetDriveType(drive);
	
	if (result == DRIVE_REMOVABLE)
//	if (result == DRIVE_CDROM)
	{
	   if (GetVolumeInformation (   
							 strRootPath,    
							 strVolumeName,    
							 sizeof (strVolumeName),    
							 &dwVolumeSerialNumber,   
							 &dwMaximumComponentLength,   
							 &dwFileSystemFlags,   
							 strFileSysName,   
							 sizeof (strFileSysName)   
								)   
		  )   
	   {   
			rtn = strcmp((char*)strVolumeName,"Gingy Disk");
			if(rtn == 0)
			{
				return Mass_Storage;
			}		 
	   }
	}
	return OK;
}