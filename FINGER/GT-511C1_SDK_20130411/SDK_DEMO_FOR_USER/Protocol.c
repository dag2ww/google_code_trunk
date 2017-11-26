#include "Protocol.h"
#include "Mass_StorageApi.h"

devinfo gDeviceInfo;
SB_OEM_PKT* Command_Packet;
unsigned char DataBuff[26];
unsigned char CommandBuff[512];


int CalcChkSumOfCmdAckPkt( SB_OEM_PKT* pPkt )
{
	unsigned short wChkSum = 0;
	unsigned char * pBuf = (unsigned char*)pPkt;
	int i;
	
	for(i=0;i<(sizeof(SB_OEM_PKT)-SB_OEM_CHK_SUM_SIZE);i++)
		wChkSum += pBuf[i];

	return wChkSum;
}
int CheckChkSumOfRpsAckPkt( SB_OEM_PKT* pPkt )
{
	unsigned short wChkSum = 0;
	unsigned char * pBuf = (unsigned char*)pPkt;
	int i;

	if(pPkt->Head1 != RESPONSE_START_CODE1 || pPkt->Head2 != RESPONSE_START_CODE2)
		return PKT_HDR_ERR;

	if(pPkt->wDevId0 != DEVICE_ID || pPkt->wDevId1 != ZERO)
		return PKT_DEV_ID_ERR;

	for(i=0;i<(sizeof(SB_OEM_PKT)-SB_OEM_CHK_SUM_SIZE);i++)
		wChkSum += pBuf[i];

	if(pPkt->wChkSum0 != (wChkSum & 0xFF) || pPkt->wChkSum1 != (wChkSum & 0xFF00)>>8)
		return PKT_CHK_SUM_ERR;

	return OK;
}
int CalcChkSumOfDataPkt( unsigned char* pDataPkt, int nSize )
{
	int i;
	unsigned short wChkSum = 0;
	unsigned char* pBuf = (unsigned char*)pDataPkt;

	if(*pBuf != DATA_START_CODE1 || *(pBuf + 1) != DATA_START_CODE2)
		return PKT_HDR_ERR;

	if(*(pBuf + 2) != DEVICE_ID || *(pBuf + 3) != ZERO)
		return PKT_DEV_ID_ERR;
	
	for(i=0;i< nSize - SB_OEM_CHK_SUM_SIZE;i++)
		wChkSum += pBuf[i];

	if(*(pBuf + nSize - SB_OEM_CHK_SUM_SIZE) != (wChkSum & 0xFF) || *(pBuf + nSize - SB_OEM_CHK_SUM_SIZE + 1) != (wChkSum & 0xFF00)>>8)
		return PKT_CHK_SUM_ERR;
	return OK;
}

int Open(int Flag,unsigned char* DataBuffer)
{
	unsigned char Buffer[12];
//	unsigned char DataBuff[26];
	int rtn;
	int i,j;
	unsigned long tmp = 0;
	Command_Packet = &CommandBuff[0];
	/*********Open Command***********/
	Command_Packet->Head1 = COMMAND_START_CODE1;
	Command_Packet->Head2 = COMMAND_START_CODE2;
	Command_Packet->wDevId0 = DEVICE_ID;
	Command_Packet->wDevId1 = DEVICE_ID>>8;
	Command_Packet->nParam0 = Flag & 0xFF;
	Command_Packet->nParam1 = (Flag & 0xFF00) >> 8 ;
	Command_Packet->nParam2 = (Flag & 0xFF0000) >> 16;
	Command_Packet->nParam3 = (Flag & 0xFF000000) >> 24;
	Command_Packet->wCmd0 = OPEN;
	Command_Packet->wCmd1 = ZERO;
	tmp = CalcChkSumOfCmdAckPkt(Command_Packet);
	Command_Packet->wChkSum0 = tmp % 256;
	Command_Packet->wChkSum1 = tmp >> 8;
	/*********Open Command***********/

	rtn = SendCommand((unsigned char *)Command_Packet,MASS_BUFFER);
	if(rtn == FAIL)
		return FAIL;
	rtn = ReceiveCommand((unsigned char *)Command_Packet,MASS_BUFFER);
	if(rtn == FAIL)
		return FAIL;

	if(Command_Packet->wCmd0 != ACK)
		return Command_Packet->nParam0 + (Command_Packet->nParam1 * 256);

	rtn = CheckChkSumOfRpsAckPkt(Command_Packet);
	if(rtn != OK)
		return rtn;

	if(Flag)
	{
		rtn = ReceiveCommand((unsigned char *)Command_Packet,MASS_BUFFER);
		if(rtn == FAIL)
			return FAIL;

		rtn = CalcChkSumOfDataPkt(Command_Packet,SB_OEM_PKT_SIZE);
		if(rtn != OK)
			return rtn;
		else
		{
			memcpy(DataBuffer,Command_Packet,SB_OEM_PKT_SIZE);
		}
	}
	return OK;
}

int ChangeBaudRate(unsigned long BAUDRATE)
{
	
	unsigned char Buffer[12];
//	unsigned char DataBuff[26];
	int rtn;
	unsigned char i;
	unsigned long tmp = 0;
	

	/*********Change Baudrate Command***********/
	Command_Packet->Head1 = COMMAND_START_CODE1;
	Command_Packet->Head2 = COMMAND_START_CODE2;
	Command_Packet->wDevId0 = DEVICE_ID;
	Command_Packet->wDevId1 = DEVICE_ID>>8;
	Command_Packet->nParam0 = BAUDRATE & 0xFF;
	Command_Packet->nParam1 = (BAUDRATE & 0xFF00) >> 8 ;
	Command_Packet->nParam2 = (BAUDRATE & 0xFF0000) >> 16;
	Command_Packet->nParam3 = (BAUDRATE & 0xFF000000) >> 24;
	Command_Packet->wCmd0 = CHANGBAUDRATE;
	Command_Packet->wCmd1 = ZERO;
	tmp = CalcChkSumOfCmdAckPkt(Command_Packet);
	Command_Packet->wChkSum0 = tmp % 256;
	Command_Packet->wChkSum1 = tmp >> 8;
	/*********Change Baudrate Command***********/

	rtn = SendCommand((unsigned char *)Command_Packet,MASS_BUFFER);
	if(rtn == FAIL)
		return FAIL;

	rtn = ReceiveCommand((unsigned char *)Command_Packet,MASS_BUFFER);
	if(rtn == FAIL)
		return FAIL;

	if(Command_Packet->wCmd0 != ACK)
		return Command_Packet->nParam0 + (Command_Packet->nParam1 * 256);

	rtn = CheckChkSumOfRpsAckPkt(Command_Packet);
	if(rtn != OK)
		return rtn;

	return OK;
	
}
int Termination()
{
//	SB_OEM_PKT* Command_Packet;
//	unsigned char Buffer[12];
	int rtn;
	unsigned char i;
	unsigned long tmp = 0;
//	Command_Packet = Buffer;
	

	/*********Termination Command***********/
	Command_Packet->Head1 = COMMAND_START_CODE1;
	Command_Packet->Head2 = COMMAND_START_CODE2;
	Command_Packet->wDevId0 = DEVICE_ID;
	Command_Packet->wDevId1 = DEVICE_ID>>8;
	Command_Packet->nParam0 = ZERO;
	Command_Packet->nParam1 = ZERO;
	Command_Packet->nParam2 = ZERO;
	Command_Packet->nParam3 = ZERO;
	Command_Packet->wCmd0 = CLOSE;
	Command_Packet->wCmd1 = ZERO;
	tmp = CalcChkSumOfCmdAckPkt(Command_Packet);
	Command_Packet->wChkSum0 = tmp % 256;
	Command_Packet->wChkSum1 = tmp >> 8;
	/*********Open Command***********/

	rtn = SendCommand((unsigned char *)Command_Packet,MASS_BUFFER);
	if(rtn == FAIL)
		return FAIL;
	rtn = ReceiveCommand((unsigned char *)Command_Packet,MASS_BUFFER);
	if(rtn == FAIL)
		return FAIL;

	if(Command_Packet->wCmd0 != ACK)
		return Command_Packet->nParam0 + (Command_Packet->nParam1 * 256);

	rtn = CheckChkSumOfRpsAckPkt(Command_Packet);
	if(rtn != OK)
		return rtn;

	/*********Change Baudrate Command***********/
	Command_Packet->Head1 = COMMAND_START_CODE1;
	Command_Packet->Head2 = COMMAND_START_CODE2;
	Command_Packet->wDevId0 = DEVICE_ID;
	Command_Packet->wDevId1 = DEVICE_ID>>8;
	Command_Packet->nParam0 = 9600 & 0xFF;
	Command_Packet->nParam1 = (9600 & 0xFF00) >> 8 ;
	Command_Packet->nParam2 = (9600 & 0xFF0000) >> 16;
	Command_Packet->nParam3 = (9600 & 0xFF000000) >> 24;
	Command_Packet->wCmd0 = CHANGBAUDRATE;
	Command_Packet->wCmd1 = ZERO;
	tmp = CalcChkSumOfCmdAckPkt(Command_Packet);
	Command_Packet->wChkSum0 = tmp % 256;
	Command_Packet->wChkSum1 = tmp >> 8;
	/*********Change Baudrate Command***********/

	rtn = SendCommand((unsigned char *)Command_Packet,MASS_BUFFER);
	if(rtn == FAIL)
		return FAIL;

	rtn = ReceiveCommand((unsigned char *)Command_Packet,MASS_BUFFER);
	if(rtn == FAIL)
		return FAIL;

	if(Command_Packet->wCmd0 != ACK)
		return Command_Packet->nParam0 + (Command_Packet->nParam1 * 256);

	rtn = CheckChkSumOfRpsAckPkt(Command_Packet);
	if(rtn != OK)
		return rtn;
	return OK;
}
int Enroll_Start(int ID)
{
//	SB_OEM_PKT* Command_Packet;
//	unsigned char Buffer[12];
//	unsigned char DataBuff[26];
	int rtn;
	unsigned char i;
	unsigned long tmp = 0;
//	Command_Packet = Buffer;
	/*********Change Baudrate Command***********/
	Command_Packet->Head1 = COMMAND_START_CODE1;
	Command_Packet->Head2 = COMMAND_START_CODE2;
	Command_Packet->wDevId0 = DEVICE_ID;
	Command_Packet->wDevId1 = DEVICE_ID>>8;
	Command_Packet->nParam0 = ID & 0xFF;
	Command_Packet->nParam1 = (ID & 0xFF00) >> 8 ;
	Command_Packet->nParam2 = (ID & 0xFF0000) >> 16;
	Command_Packet->nParam3 = (ID & 0xFF000000) >> 24;
	Command_Packet->wCmd0 = ENROLLSTART;
	Command_Packet->wCmd1 = ZERO;
	tmp = CalcChkSumOfCmdAckPkt(Command_Packet);
	Command_Packet->wChkSum0 = tmp % 256;
	Command_Packet->wChkSum1 = tmp >> 8;
	/*********Change Baudrate Command***********/

	rtn = SendCommand((unsigned char *)Command_Packet,MASS_BUFFER);
	if(rtn == FAIL)
		return FAIL;

	rtn = ReceiveCommand((unsigned char *)Command_Packet,MASS_BUFFER);
	if(rtn == FAIL)
		return FAIL;

	if(Command_Packet->wCmd0 != ACK)
		return Command_Packet->nParam0 + (Command_Packet->nParam1 * 256);

	rtn = CheckChkSumOfRpsAckPkt(Command_Packet);
	if(rtn != OK)
		return rtn;

	return OK;
}
int CaptureFinger(char cFlag)
{
//	SB_OEM_PKT* Command_Packet;
//	unsigned char Buffer[12];
//	unsigned char DataBuff[26];
	int rtn;
	unsigned char i;
	unsigned long tmp = 0;
//	Command_Packet = Buffer;
	/*********Change Baudrate Command***********/
	Command_Packet->Head1 = COMMAND_START_CODE1;
	Command_Packet->Head2 = COMMAND_START_CODE2;
	Command_Packet->wDevId0 = DEVICE_ID;
	Command_Packet->wDevId1 = DEVICE_ID>>8;
	Command_Packet->nParam0 = cFlag & 0xFF;
	Command_Packet->nParam1 = (cFlag & 0xFF00) >> 8 ;
	Command_Packet->nParam2 = (cFlag & 0xFF0000) >> 16;
	Command_Packet->nParam3 = (	cFlag & 0xFF000000) >> 24;
	Command_Packet->wCmd0 = CAPTURE_FINGER;
	Command_Packet->wCmd1 = ZERO;
	tmp = CalcChkSumOfCmdAckPkt(Command_Packet);
	Command_Packet->wChkSum0 = tmp % 256;
	Command_Packet->wChkSum1 = tmp >> 8;
	/*********Change Baudrate Command***********/

	rtn = SendCommand((unsigned char *)Command_Packet,MASS_BUFFER);
	if(rtn == FAIL)
		return FAIL;

	rtn = ReceiveCommand((unsigned char *)Command_Packet,MASS_BUFFER);
	if(rtn == FAIL)
		return FAIL;

	if(Command_Packet->wCmd0 != ACK)
		return Command_Packet->nParam0 + (Command_Packet->nParam1 * 256);

	rtn = CheckChkSumOfRpsAckPkt(Command_Packet);
	if(rtn != OK)
		return rtn;

	return OK;
}
int Enroll_nth(char Count)
{
//	SB_OEM_PKT* Command_Packet;
//	unsigned char Buffer[12];
//	unsigned char DataBuff[26];
	int rtn;
	unsigned char i;
	unsigned long tmp = 0;
//	Command_Packet = Buffer;
	/*********Change Baudrate Command***********/
	Command_Packet->Head1 = COMMAND_START_CODE1;
	Command_Packet->Head2 = COMMAND_START_CODE2;
	Command_Packet->wDevId0 = DEVICE_ID;
	Command_Packet->wDevId1 = DEVICE_ID>>8;
	Command_Packet->nParam0 = ZERO;
	Command_Packet->nParam1 = ZERO;
	Command_Packet->nParam2 = ZERO;
	Command_Packet->nParam3 = ZERO;
	Command_Packet->wCmd0 = ENROLLSTART + Count;
	Command_Packet->wCmd1 = ZERO;
	tmp = CalcChkSumOfCmdAckPkt(Command_Packet);
	Command_Packet->wChkSum0 = tmp % 256;
	Command_Packet->wChkSum1 = tmp >> 8;
	/*********Change Baudrate Command***********/

	rtn = SendCommand((unsigned char *)Command_Packet,MASS_BUFFER);
	if(rtn == FAIL)
		return FAIL;

	rtn = ReceiveCommand((unsigned char *)Command_Packet,MASS_BUFFER);
	if(rtn == FAIL)
		return FAIL;

	if(Command_Packet->wCmd0 != ACK)
		return Command_Packet->nParam0 + (Command_Packet->nParam1 * 256);

	rtn = CheckChkSumOfRpsAckPkt(Command_Packet);
	if(rtn != OK)
		return rtn;

	return Command_Packet->nParam0 + (Command_Packet->nParam1 * 256);
}
int RemoveFinger()
{
//	SB_OEM_PKT* Command_Packet;
//	unsigned char Buffer[12];
//	unsigned char DataBuff[26];
	int rtn;
	unsigned char i;
	unsigned long tmp = 0;
//	Command_Packet = Buffer;
	/*********Change Baudrate Command***********/
	Command_Packet->Head1 = COMMAND_START_CODE1;
	Command_Packet->Head2 = COMMAND_START_CODE2;
	Command_Packet->wDevId0 = DEVICE_ID;
	Command_Packet->wDevId1 = DEVICE_ID>>8;
	Command_Packet->nParam0 = ZERO;
	Command_Packet->nParam1 = ZERO;
	Command_Packet->nParam2 = ZERO;
	Command_Packet->nParam3 = ZERO;
	Command_Packet->wCmd0 = ISPRESSFINGER;
	Command_Packet->wCmd1 = ZERO;
	tmp = CalcChkSumOfCmdAckPkt(Command_Packet);
	Command_Packet->wChkSum0 = tmp % 256;
	Command_Packet->wChkSum1 = tmp >> 8;
	/*********Change Baudrate Command***********/

	rtn = SendCommand((unsigned char *)Command_Packet,MASS_BUFFER);
	if(rtn == FAIL)
		return FAIL;

	rtn = ReceiveCommand((unsigned char *)Command_Packet,MASS_BUFFER);
	if(rtn == FAIL)
		return FAIL;

	if(Command_Packet->wCmd0 != ACK)
		return Command_Packet->nParam0 + (Command_Packet->nParam1 * 256);

	rtn = CheckChkSumOfRpsAckPkt(Command_Packet);
	if(rtn != OK)
		return rtn;

	return Command_Packet->nParam0 + (Command_Packet->nParam1 * 256);
}
int GetEnrollCount()
{
//	SB_OEM_PKT* Command_Packet;
//	unsigned char Buffer[12];
//	unsigned char DataBuff[26];
	int rtn;
	unsigned char i;
	unsigned long tmp = 0;
//	Command_Packet = Buffer;
	/*********Change Baudrate Command***********/
	Command_Packet->Head1 = COMMAND_START_CODE1;
	Command_Packet->Head2 = COMMAND_START_CODE2;
	Command_Packet->wDevId0 = DEVICE_ID;
	Command_Packet->wDevId1 = DEVICE_ID>>8;
	Command_Packet->nParam0 = ZERO;
	Command_Packet->nParam1 = ZERO;
	Command_Packet->nParam2 = ZERO;
	Command_Packet->nParam3 = ZERO;
	Command_Packet->wCmd0 = GETENROLLCOUNT;
	Command_Packet->wCmd1 = ZERO;
	tmp = CalcChkSumOfCmdAckPkt(Command_Packet);
	Command_Packet->wChkSum0 = tmp % 256;
	Command_Packet->wChkSum1 = tmp >> 8;
	/*********Change Baudrate Command***********/

	rtn = SendCommand((unsigned char *)Command_Packet,MASS_BUFFER);
	if(rtn == FAIL)
		return FAIL;

	rtn = ReceiveCommand((unsigned char *)Command_Packet,MASS_BUFFER);
	if(rtn == FAIL)
		return FAIL;

	if(Command_Packet->wCmd0 != ACK)
		return Command_Packet->nParam0 + (Command_Packet->nParam1 * 256);

	rtn = CheckChkSumOfRpsAckPkt(Command_Packet);
	if(rtn != OK)
		return rtn;

	return Command_Packet->nParam0 + (Command_Packet->nParam1 * 256);
}
int Identify()
{
//	SB_OEM_PKT* Command_Packet;
//	unsigned char Buffer[12];
//	unsigned char DataBuff[26];
	int rtn;
	unsigned char i;
	unsigned long tmp = 0;
//	Command_Packet = Buffer;
	/*********Change Baudrate Command***********/
	Command_Packet->Head1 = COMMAND_START_CODE1;
	Command_Packet->Head2 = COMMAND_START_CODE2;
	Command_Packet->wDevId0 = DEVICE_ID;
	Command_Packet->wDevId1 = DEVICE_ID>>8;
	Command_Packet->nParam0 = ZERO;
	Command_Packet->nParam1 = ZERO;
	Command_Packet->nParam2 = ZERO;
	Command_Packet->nParam3 = ZERO;
	Command_Packet->wCmd0 = IDENTIFY;
	Command_Packet->wCmd1 = ZERO;
	tmp = CalcChkSumOfCmdAckPkt(Command_Packet);
	Command_Packet->wChkSum0 = tmp % 256;
	Command_Packet->wChkSum1 = tmp >> 8;
	/*********Change Baudrate Command***********/

	rtn = SendCommand((unsigned char *)Command_Packet,MASS_BUFFER);
	if(rtn == FAIL)
		return FAIL;

	rtn = ReceiveCommand((unsigned char *)Command_Packet,MASS_BUFFER);
	if(rtn == FAIL)
		return FAIL;

	if(Command_Packet->wCmd0 != ACK)
		return Command_Packet->nParam0 + (Command_Packet->nParam1 * 256);

	rtn = CheckChkSumOfRpsAckPkt(Command_Packet);
	if(rtn != OK)
		return rtn;

	return (Command_Packet->nParam1<<8) + Command_Packet->nParam0;
}

int Verify(int ID)
{
//	SB_OEM_PKT* Command_Packet;
//	unsigned char Buffer[12];
//	unsigned char DataBuff[26];
	int rtn;
	unsigned char i;
	unsigned long tmp = 0;
//	Command_Packet = Buffer;
	/*********Change Baudrate Command***********/
	Command_Packet->Head1 = COMMAND_START_CODE1;
	Command_Packet->Head2 = COMMAND_START_CODE2;
	Command_Packet->wDevId0 = DEVICE_ID;
	Command_Packet->wDevId1 = DEVICE_ID>>8;
	Command_Packet->nParam0 = ID;
	Command_Packet->nParam1 = ZERO;
	Command_Packet->nParam2 = ZERO;
	Command_Packet->nParam3 = ZERO;
	Command_Packet->wCmd0 = VERIFY;
	Command_Packet->wCmd1 = ZERO;
	tmp = CalcChkSumOfCmdAckPkt(Command_Packet);
	Command_Packet->wChkSum0 = tmp % 256;
	Command_Packet->wChkSum1 = tmp >> 8;
	/*********Change Baudrate Command***********/

	rtn = SendCommand((unsigned char *)Command_Packet,MASS_BUFFER);
	if(rtn == FAIL)
		return FAIL;

	rtn = ReceiveCommand((unsigned char *)Command_Packet,MASS_BUFFER);
	if(rtn == FAIL)
		return FAIL;

	if(Command_Packet->wCmd0 != ACK)
		return Command_Packet->nParam0 + (Command_Packet->nParam1 * 256);

	rtn = CheckChkSumOfRpsAckPkt(Command_Packet);
	if(rtn != OK)
		return rtn;

	return (Command_Packet->nParam1<<8) + Command_Packet->nParam0;
}


int DeleteID(unsigned char ID)
{
//	SB_OEM_PKT* Command_Packet;
//	unsigned char Buffer[12];
//	unsigned char DataBuff[26];
	int rtn;
	unsigned char i;
	int tmp = 0;
//	Command_Packet = (SB_OEM_PKT *)Buffer;
	/*********Change Baudrate Command***********/
	Command_Packet->Head1 = COMMAND_START_CODE1;
	Command_Packet->Head2 = COMMAND_START_CODE2;
	Command_Packet->wDevId0 = DEVICE_ID;
	Command_Packet->wDevId1 = DEVICE_ID>>8;
	Command_Packet->nParam0 = ID & 0xFF;
	Command_Packet->nParam1 = (ID & 0xFF00) >> 8 ;
	Command_Packet->nParam2 = (ID & 0xFF0000) >> 16;
	Command_Packet->nParam3 = (ID & 0xFF000000) >> 24;
	Command_Packet->wCmd0 = DELETEID;
	Command_Packet->wCmd1 = ZERO;
	tmp = CalcChkSumOfCmdAckPkt(Command_Packet);
	Command_Packet->wChkSum0 = tmp % 256;
	Command_Packet->wChkSum1 = tmp >> 8;
	/*********Change Baudrate Command***********/

	rtn = SendCommand((unsigned char *)Command_Packet,MASS_BUFFER);
	if(rtn == FAIL)
		return FAIL;

	rtn = ReceiveCommand((unsigned char *)Command_Packet,MASS_BUFFER);
	if(rtn == FAIL)
		return FAIL;

	if(Command_Packet->wCmd0 != ACK)
		return Command_Packet->nParam0 + (Command_Packet->nParam1 * 256);

	rtn = CheckChkSumOfRpsAckPkt(Command_Packet);
	if(rtn != OK)
		return rtn;
	
	return OK;
}
int DeleteAll()
{
//	SB_OEM_PKT* Command_Packet;
//	unsigned char Buffer[12];
//	unsigned char DataBuff[26];
	int rtn;
	unsigned char i;
	unsigned long tmp = 0;
//	Command_Packet = Buffer;
	/*********Change Baudrate Command***********/
	Command_Packet->Head1 = COMMAND_START_CODE1;
	Command_Packet->Head2 = COMMAND_START_CODE2;
	Command_Packet->wDevId0 = DEVICE_ID;
	Command_Packet->wDevId1 = DEVICE_ID>>8;
	Command_Packet->nParam0 = ZERO;
	Command_Packet->nParam1 = ZERO;
	Command_Packet->nParam2 = ZERO;
	Command_Packet->nParam3 = ZERO;
	Command_Packet->wCmd0 = DELETEALL;
	Command_Packet->wCmd1 = ZERO;
	tmp = CalcChkSumOfCmdAckPkt(Command_Packet);
	Command_Packet->wChkSum0 = tmp % 256;
	Command_Packet->wChkSum1 = tmp >> 8;
	/*********Change Baudrate Command***********/

	rtn = SendCommand((unsigned char *)Command_Packet,MASS_BUFFER);
	if(rtn == FAIL)
		return FAIL;

	rtn = ReceiveCommand((unsigned char *)Command_Packet,MASS_BUFFER);
	if(rtn == FAIL)
		return FAIL;

	if(Command_Packet->wCmd0 != ACK)
		return Command_Packet->nParam0 + (Command_Packet->nParam1 * 256);

	rtn = CheckChkSumOfRpsAckPkt(Command_Packet);
	if(rtn != OK)
		return rtn;

	return OK;
}
int CheckEnroll(int ID)
{
//	SB_OEM_PKT* Command_Packet;
//	unsigned char Buffer[12];
//	unsigned char DataBuff[26];
	int rtn;
	unsigned char i;
	unsigned long tmp = 0;
//	Command_Packet = Buffer;
	/*********Change Baudrate Command***********/
	Command_Packet->Head1 = COMMAND_START_CODE1;
	Command_Packet->Head2 = COMMAND_START_CODE2;
	Command_Packet->wDevId0 = DEVICE_ID;
	Command_Packet->wDevId1 = DEVICE_ID>>8;
	Command_Packet->nParam0 = ID & 0xFF;
	Command_Packet->nParam1 = (ID & 0xFF00) >> 8 ;
	Command_Packet->nParam2 = (ID & 0xFF0000) >> 16;
	Command_Packet->nParam3 = (ID & 0xFF000000) >> 24;
	Command_Packet->wCmd0 = CHECKENROLLED;
	Command_Packet->wCmd1 = ZERO;
	tmp = CalcChkSumOfCmdAckPkt(Command_Packet);
	Command_Packet->wChkSum0 = tmp % 256;
	Command_Packet->wChkSum1 = tmp >> 8;
	/*********Change Baudrate Command***********/

	rtn = SendCommand((unsigned char *)Command_Packet,MASS_BUFFER);
	if(rtn == FAIL)
		return FAIL;

	rtn = ReceiveCommand((unsigned char *)Command_Packet,MASS_BUFFER);
	if(rtn == FAIL)
		return FAIL;

	if(Command_Packet->wCmd0 != ACK)
		return Command_Packet->nParam0 + (Command_Packet->nParam1 * 256);

	rtn = CheckChkSumOfRpsAckPkt(Command_Packet);
	if(rtn != OK)
		return rtn;

	return OK;
}
int LED(unsigned char State)
{
//	SB_OEM_PKT* Command_Packet;
//	unsigned char Buffer[12];
//	unsigned char DataBuff[26];
	int rtn;
	unsigned char i;
	unsigned long tmp = 0;
//	Command_Packet = Buffer;
	/*********Change Baudrate Command***********/
	Command_Packet->Head1 = COMMAND_START_CODE1;
	Command_Packet->Head2 = COMMAND_START_CODE2;
	Command_Packet->wDevId0 = DEVICE_ID;
	Command_Packet->wDevId1 = DEVICE_ID>>8;
	Command_Packet->nParam0 = State & 0xFF;
	Command_Packet->nParam1 = (State & 0xFF00) >> 8 ;
	Command_Packet->nParam2 = (State & 0xFF0000) >> 16;
	Command_Packet->nParam3 = (State & 0xFF000000) >> 24;
	Command_Packet->wCmd0 = CMOSLED;
	Command_Packet->wCmd1 = ZERO;
	tmp = CalcChkSumOfCmdAckPkt(Command_Packet);
	Command_Packet->wChkSum0 = tmp % 256;
	Command_Packet->wChkSum1 = tmp >> 8;
	/*********Change Baudrate Command***********/

	rtn = SendCommand((unsigned char *)Command_Packet,MASS_BUFFER);
	if(rtn == FAIL)
		return FAIL;

	rtn = ReceiveCommand((unsigned char *)Command_Packet,MASS_BUFFER);
	if(rtn == FAIL)
		return FAIL;

	if(Command_Packet->wCmd0 != ACK)
		return Command_Packet->nParam0 + (Command_Packet->nParam1 * 256);

	rtn = CheckChkSumOfRpsAckPkt(Command_Packet);
	if(rtn != OK)
		return rtn;

	return OK;
}

int GetRawImage()
{
	unsigned char Buffer[12];
//	unsigned char DataBuff[26];
	int rtn;
	unsigned char i;
	unsigned long tmp = 0;
	Command_Packet = &CommandBuff[0];
	/*********Open Command***********/
	Command_Packet->Head1 = COMMAND_START_CODE1;
	Command_Packet->Head2 = COMMAND_START_CODE2;
	Command_Packet->wDevId0 = DEVICE_ID;
	Command_Packet->wDevId1 = DEVICE_ID>>8;
	Command_Packet->nParam0 = ZERO;
	Command_Packet->nParam1 = ZERO;
	Command_Packet->nParam2 = ZERO;
	Command_Packet->nParam3 = ZERO;
	Command_Packet->wCmd0 = GETRAWIMAGE;
	Command_Packet->wCmd1 = ZERO;
	tmp = CalcChkSumOfCmdAckPkt(Command_Packet);
	Command_Packet->wChkSum0 = tmp % 256;
	Command_Packet->wChkSum1 = tmp >> 8;
	/*********Open Command***********/

	rtn = SendCommand((unsigned char *)Command_Packet,MASS_BUFFER);
	if(rtn == FAIL)
		return FAIL;
	rtn = ReceiveCommand((unsigned char *)Command_Packet,MASS_BUFFER);
	if(rtn == FAIL)
		return FAIL;

	if(Command_Packet->wCmd0 != ACK)
		return Command_Packet->nParam0 + (Command_Packet->nParam1 * 256);

	rtn = CheckChkSumOfRpsAckPkt(Command_Packet);
	if(rtn != OK)
		return rtn;
	return OK;
}

int GetTemple(int ID,unsigned char* DataBuffer)
{
	unsigned char Buffer[12];
//	unsigned char DataBuff[26];
	int rtn;
	int i,j,k;
	unsigned long tmp = 0;
	Command_Packet = &CommandBuff[0];
	/*********Open Command***********/
	Command_Packet->Head1 = COMMAND_START_CODE1;
	Command_Packet->Head2 = COMMAND_START_CODE2;
	Command_Packet->wDevId0 = DEVICE_ID;
	Command_Packet->wDevId1 = DEVICE_ID>>8;
	Command_Packet->nParam0 = ID & 0xFF;
	Command_Packet->nParam1 = (ID & 0xFF00) >> 8 ;
	Command_Packet->nParam2 = (ID & 0xFF0000) >> 16;
	Command_Packet->nParam3 = (ID & 0xFF000000) >> 24;
	Command_Packet->wCmd0 = GETTEMPLATE;
	Command_Packet->wCmd1 = ZERO;
	tmp = CalcChkSumOfCmdAckPkt(Command_Packet);
	Command_Packet->wChkSum0 = tmp % 256;
	Command_Packet->wChkSum1 = tmp >> 8;
	/*********Open Command***********/

	rtn = SendCommand((unsigned char *)Command_Packet,MASS_BUFFER);
	if(rtn == FAIL)
		return FAIL;
	rtn = ReceiveCommand((unsigned char *)Command_Packet,MASS_BUFFER);
	if(rtn == FAIL)
		return FAIL;

	if(Command_Packet->wCmd0 != ACK)
		return Command_Packet->nParam0 + (Command_Packet->nParam1 * 256);

	rtn = CheckChkSumOfRpsAckPkt(Command_Packet);
	if(rtn != OK)
		return rtn;
	for(k = 0; k < 512; k += 64)
	{
		rtn = ReceiveCommand((unsigned char *)Command_Packet,MASS_BUFFER);
		if(rtn == FAIL)
			return FAIL;
		memcpy((DataBuffer + k),Command_Packet,64);
	}
	rtn = CalcChkSumOfDataPkt(DataBuffer,512);
	if(rtn != OK)
		return rtn;
	return OK;
}

int SetTemple(int ID,unsigned char* DataBuffer)
{
	unsigned char Buffer[12];
//	unsigned char DataBuff[26];
	int rtn;
	int i,j,k;
	unsigned long tmp = 0;
	Command_Packet = &CommandBuff[0];
	/*********Open Command***********/
	Command_Packet->Head1 = COMMAND_START_CODE1;
	Command_Packet->Head2 = COMMAND_START_CODE2;
	Command_Packet->wDevId0 = DEVICE_ID;
	Command_Packet->wDevId1 = DEVICE_ID>>8;
	Command_Packet->nParam0 = ID & 0xFF;
	Command_Packet->nParam1 = (ID & 0xFF00) >> 8 ;
	Command_Packet->nParam2 = (ID & 0xFF0000) >> 16;
	Command_Packet->nParam3 = (ID & 0xFF000000) >> 24;
	Command_Packet->wCmd0 = SETTEMPLATE;
	Command_Packet->wCmd1 = ZERO;
	tmp = CalcChkSumOfCmdAckPkt(Command_Packet);
	Command_Packet->wChkSum0 = tmp % 256;
	Command_Packet->wChkSum1 = tmp >> 8;
	/*********Open Command***********/

	rtn = SendCommand((unsigned char *)Command_Packet,MASS_BUFFER);
	if(rtn == FAIL)
		return FAIL;
	rtn = ReceiveCommand((unsigned char *)Command_Packet,MASS_BUFFER);
	if(rtn == FAIL)
		return FAIL;

	if(Command_Packet->wCmd0 != ACK)
		return Command_Packet->nParam0 + (Command_Packet->nParam1 * 256);

	rtn = CheckChkSumOfRpsAckPkt(Command_Packet);
	if(rtn != OK)
		return rtn;
	for(k = 0; k < 512; k += 64)
	{
		rtn = SendCommand((unsigned char *)DataBuffer + k,MASS_BUFFER);
		if(rtn == FAIL)
			return FAIL;
	}
	rtn = ReceiveCommand((unsigned char *)Command_Packet,MASS_BUFFER);
	if(rtn == FAIL)
		return FAIL;

	if(Command_Packet->wCmd0 != ACK)
		return Command_Packet->nParam0 + (Command_Packet->nParam1 * 256);

	rtn = CheckChkSumOfRpsAckPkt(Command_Packet);
	if(rtn != OK)
		return rtn;
	return OK;
}

int VerifyTemplate(int ID,unsigned char* DataBuffer)
{
	unsigned char Buffer[12];
//	unsigned char DataBuff[26];
	int rtn;
	int i,j,k;
	unsigned long tmp = 0;
	Command_Packet = &CommandBuff[0];
	/*********Open Command***********/
	Command_Packet->Head1 = COMMAND_START_CODE1;
	Command_Packet->Head2 = COMMAND_START_CODE2;
	Command_Packet->wDevId0 = DEVICE_ID;
	Command_Packet->wDevId1 = DEVICE_ID>>8;
	Command_Packet->nParam0 = ID & 0xFF;
	Command_Packet->nParam1 = (ID & 0xFF00) >> 8 ;
	Command_Packet->nParam2 = (ID & 0xFF0000) >> 16;
	Command_Packet->nParam3 = (ID & 0xFF000000) >> 24;
	Command_Packet->wCmd0 = VERIFY_TEMPLATE;
	Command_Packet->wCmd1 = ZERO;
	tmp = CalcChkSumOfCmdAckPkt(Command_Packet);
	Command_Packet->wChkSum0 = tmp % 256;
	Command_Packet->wChkSum1 = tmp >> 8;
	/*********Open Command***********/

	rtn = SendCommand((unsigned char *)Command_Packet,MASS_BUFFER);
	if(rtn == FAIL)
		return FAIL;
	rtn = ReceiveCommand((unsigned char *)Command_Packet,MASS_BUFFER);
	if(rtn == FAIL)
		return FAIL;

	if(Command_Packet->wCmd0 != ACK)
		return Command_Packet->nParam0 + (Command_Packet->nParam1 * 256);

	rtn = CheckChkSumOfRpsAckPkt(Command_Packet);
	if(rtn != OK)
		return rtn;
	for(k = 0; k < 512; k += 64)
	{
		rtn = SendCommand((unsigned char *)DataBuffer + k,MASS_BUFFER);
		if(rtn == FAIL)
			return FAIL;
	}
	rtn = ReceiveCommand((unsigned char *)Command_Packet,MASS_BUFFER);
	if(rtn == FAIL)
		return FAIL;

	if(Command_Packet->wCmd0 != ACK)
		return Command_Packet->nParam0 + (Command_Packet->nParam1 * 256);

	rtn = CheckChkSumOfRpsAckPkt(Command_Packet);
	if(rtn != OK)
		return rtn;
	return OK;
}

int IdentifyTemplate(int ID,unsigned char* DataBuffer)
{
	unsigned char Buffer[12];
//	unsigned char DataBuff[26];
	int rtn;
	int i,j,k;
	unsigned long tmp = 0;
	Command_Packet = &CommandBuff[0];
	/*********Open Command***********/
	Command_Packet->Head1 = COMMAND_START_CODE1;
	Command_Packet->Head2 = COMMAND_START_CODE2;
	Command_Packet->wDevId0 = DEVICE_ID;
	Command_Packet->wDevId1 = DEVICE_ID>>8;
	Command_Packet->nParam0 = ID & 0xFF;
	Command_Packet->nParam1 = (ID & 0xFF00) >> 8 ;
	Command_Packet->nParam2 = (ID & 0xFF0000) >> 16;
	Command_Packet->nParam3 = (ID & 0xFF000000) >> 24;
	Command_Packet->wCmd0 = IDENTIFY_TEMPLATE;
	Command_Packet->wCmd1 = ZERO;
	tmp = CalcChkSumOfCmdAckPkt(Command_Packet);
	Command_Packet->wChkSum0 = tmp % 256;
	Command_Packet->wChkSum1 = tmp >> 8;
	/*********Open Command***********/

	rtn = SendCommand((unsigned char *)Command_Packet,MASS_BUFFER);
	if(rtn == FAIL)
		return FAIL;
	rtn = ReceiveCommand((unsigned char *)Command_Packet,MASS_BUFFER);
	if(rtn == FAIL)
		return FAIL;

	if(Command_Packet->wCmd0 != ACK)
		return Command_Packet->nParam0 + (Command_Packet->nParam1 * 256);

	rtn = CheckChkSumOfRpsAckPkt(Command_Packet);
	if(rtn != OK)
		return rtn;
	for(k = 0; k < 512; k += 64)
	{
		rtn = SendCommand((unsigned char *)DataBuffer + k,MASS_BUFFER);
		if(rtn == FAIL)
			return FAIL;
	}
	rtn = ReceiveCommand((unsigned char *)Command_Packet,MASS_BUFFER);
	if(rtn == FAIL)
		return FAIL;

	if(Command_Packet->wCmd0 != ACK)
		return Command_Packet->nParam0 + (Command_Packet->nParam1 * 256);

	rtn = CheckChkSumOfRpsAckPkt(Command_Packet);
	if(rtn != OK)
		return rtn;
	return OK;
}

int GetDatabaseStart()
{
//	SB_OEM_PKT* Command_Packet;
//	unsigned char Buffer[12];
//	unsigned char DataBuff[26];
	int rtn;
	unsigned char i;
	unsigned long tmp = 0;
//	Command_Packet = Buffer;
	/*********Change Baudrate Command***********/
	Command_Packet->Head1 = COMMAND_START_CODE1;
	Command_Packet->Head2 = COMMAND_START_CODE2;
	Command_Packet->wDevId0 = DEVICE_ID;
	Command_Packet->wDevId1 = DEVICE_ID>>8;
	Command_Packet->nParam0 = ZERO;
	Command_Packet->nParam1 = ZERO;
	Command_Packet->nParam2 = ZERO;
	Command_Packet->nParam3 = ZERO;
	Command_Packet->wCmd0 = GETDATABASESTART;
	Command_Packet->wCmd1 = ZERO;
	tmp = CalcChkSumOfCmdAckPkt(Command_Packet);
	Command_Packet->wChkSum0 = tmp % 256;
	Command_Packet->wChkSum1 = tmp >> 8;
	/*********Change Baudrate Command***********/

	rtn = SendCommand((unsigned char *)Command_Packet,MASS_BUFFER);
	if(rtn == FAIL)
		return FAIL;

	rtn = ReceiveCommand((unsigned char *)Command_Packet,MASS_BUFFER);
	if(rtn == FAIL)
		return FAIL;

	if(Command_Packet->wCmd0 != ACK)
		return Command_Packet->nParam0 + (Command_Packet->nParam1 * 256);

	rtn = CheckChkSumOfRpsAckPkt(Command_Packet);
	if(rtn != OK)
		return rtn;

	return Command_Packet->nParam0 + (Command_Packet->nParam1 * 256);
}

int GetDatabaseEnd()
{
//	SB_OEM_PKT* Command_Packet;
//	unsigned char Buffer[12];
//	unsigned char DataBuff[26];
	int rtn;
	unsigned char i;
	unsigned long tmp = 0;
//	Command_Packet = Buffer;
	/*********Change Baudrate Command***********/
	Command_Packet->Head1 = COMMAND_START_CODE1;
	Command_Packet->Head2 = COMMAND_START_CODE2;
	Command_Packet->wDevId0 = DEVICE_ID;
	Command_Packet->wDevId1 = DEVICE_ID>>8;
	Command_Packet->nParam0 = ZERO;
	Command_Packet->nParam1 = ZERO;
	Command_Packet->nParam2 = ZERO;
	Command_Packet->nParam3 = ZERO;
	Command_Packet->wCmd0 = GETDATABASEEND;
	Command_Packet->wCmd1 = ZERO;
	tmp = CalcChkSumOfCmdAckPkt(Command_Packet);
	Command_Packet->wChkSum0 = tmp % 256;
	Command_Packet->wChkSum1 = tmp >> 8;
	/*********Change Baudrate Command***********/

	rtn = SendCommand((unsigned char *)Command_Packet,MASS_BUFFER);
	if(rtn == FAIL)
		return FAIL;

	rtn = ReceiveCommand((unsigned char *)Command_Packet,MASS_BUFFER);
	if(rtn == FAIL)
		return FAIL;

	if(Command_Packet->wCmd0 != ACK)
		return Command_Packet->nParam0 + (Command_Packet->nParam1 * 256);

	rtn = CheckChkSumOfRpsAckPkt(Command_Packet);
	if(rtn != OK)
		return rtn;

	return Command_Packet->nParam0 + (Command_Packet->nParam1 * 256);
}

int UpgradeFirmware()
{
//	SB_OEM_PKT* Command_Packet;
//	unsigned char Buffer[12];
//	unsigned char DataBuff[26];
	int rtn;
	unsigned char i;
	unsigned long tmp = 0;
//	Command_Packet = Buffer;
	/*********Change Baudrate Command***********/
	Command_Packet->Head1 = COMMAND_START_CODE1;
	Command_Packet->Head2 = COMMAND_START_CODE2;
	Command_Packet->wDevId0 = DEVICE_ID;
	Command_Packet->wDevId1 = DEVICE_ID>>8;
	Command_Packet->nParam0 = ZERO;
	Command_Packet->nParam1 = ZERO;
	Command_Packet->nParam2 = ZERO;
	Command_Packet->nParam3 = ZERO;
	Command_Packet->wCmd0 = UPGRADEFIRMWARE;
	Command_Packet->wCmd1 = ZERO;
	tmp = CalcChkSumOfCmdAckPkt(Command_Packet);
	Command_Packet->wChkSum0 = tmp % 256;
	Command_Packet->wChkSum1 = tmp >> 8;
	/*********Change Baudrate Command***********/

	rtn = SendCommand((unsigned char *)Command_Packet,MASS_BUFFER);
	if(rtn == FAIL)
		return FAIL;

	rtn = ReceiveCommand((unsigned char *)Command_Packet,MASS_BUFFER);
	if(rtn == FAIL)
		return FAIL;

	if(Command_Packet->wCmd0 != ACK)
		return Command_Packet->nParam0 + (Command_Packet->nParam1 * 256);

	rtn = CheckChkSumOfRpsAckPkt(Command_Packet);
	if(rtn != OK)
		return rtn;

	return Command_Packet->nParam0 + (Command_Packet->nParam1 * 256);
}
