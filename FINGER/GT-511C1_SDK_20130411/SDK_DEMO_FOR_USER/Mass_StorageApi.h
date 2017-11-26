
#ifndef _MASS_STORAGE_
#define _MASS_STORAGE_

#define OK 0
#define FAIL -1
#define CONNECT_FAIL 0xFF
#define Mass_Storage 0x47





#ifdef __cplusplus
extern "C"
{
#endif

	short MyConnect(void);
	short MyDisconnect(void);
	short SendCommand(unsigned char ucData[],unsigned int uiLen);
	short ReceiveCommand(unsigned char *ucBuffer, unsigned int uiLen);
	short ShowDriveInfo(char* drive);

#ifdef __cplusplus
}
#endif
#endif
