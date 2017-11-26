#ifndef _PROTOCOL_
#define _PROTOCOL_

#define OK 0
#define FAIL (char)-1
#define MAX_ENTRY_LEN 512
#define VSIZE 224
#define HSIZE 216
#define CAP_OK 				1
#define GET_USB_MESSAGE 	2
#define PROCESSING 			3
#define QUILITY_BAD 		4
#define LOGIN_PASS 			5
#define LOGIN_FAIL			6
#define RECOVER				7
#define ID_EMPTY			8
#define ENROLLING			0x30
#define GET_IMAGE_DATA		0x33
#define IMAGE_PROTOCOLl_START 0x33
#define NO_PUT_FINGER		9
#define REMOVE_FINGER		10
#define	REMOVE_OK			11
#define UNDETECT_AGAIN		12
#define ENROLL_FAIL			13
#define ENROLL_OK			14
#define ERASEID_OK			15
#define ERASE_PROTECT		0xFB
#define FLASH_FAIL			0xFC
#define CAP_FAIL			0xFF
#define EVEN_BREAK			0xFF



#define INFO_EXISTED  -1000
#define PROCESSING_FAIL		-1001
#define COMMAND_FAIL		-1002


#define COMMAND_START_CODE1 0x55
#define COMMAND_START_CODE2 0xAA
#define DEVICE_ID			0x0001

#define RESPONSE_START_CODE1 0x55
#define RESPONSE_START_CODE2 0xAA

#define DATA_START_CODE1	0x5A
#define DATA_START_CODE2	0xA5

#define OPEN					0x01
#define CLOSE					0x02
#define USBINTERNALCHECK		0x03
#define CHANGBAUDRATE			0x04
#define IAPMODE					0x05
#define CMOSLED					0x12
#define GETENROLLCOUNT			0x20
#define CHECKENROLLED			0x21
#define	ENROLLSTART				0x22
#define ENROLL1					0x23
#define ENROLL2					0x24
#define ENROLL3					0x25
#define ISPRESSFINGER			0x26
#define DELETEID				0x40
#define DELETEALL				0x41
#define VERIFY					0x50
#define IDENTIFY				0x51
#define VERIFY_TEMPLATE			0x52
#define IDENTIFY_TEMPLATE		0x53
#define CAPTURE_FINGER			0x60
#define GETIMAGE				0x62
#define GETRAWIMAGE				0x63
#define GETTEMPLATE				0x70
#define SETTEMPLATE				0x71
#define GETDATABASESTART		0x72
#define GETDATABASEEND			0x73
#define UPGRADEFIRMWARE			0x80
#define ACK						0x30
#define NACK					0x31

#define NACK_NONE				0x1000
#define NACK_TIMEOUT			0x1001
#define NACK_INVALID_BAUDRATE	0x1002
#define NACK_INVALID_POS		0x1003
#define NACK_IS_NOT_USED		0x1004
#define NACK_IS_ALREADY_USED	0x1005
#define NACK_COMM_ERR			0x1006
#define NACK_VERIFY_FAILED		0x1007
#define NACK_IDENTIFY_FAILED	0x1008
#define NACK_DB_IS_FULL			0x1009
#define NACK_DB_IS_EMPTY		0x100A
#define NACK_TURN_ERR			0x100B
#define NACK_BAD_FINGER			0x100C
#define NACK_ENROLL_FAILED		0x100D
#define NACK_IS_NOT_SUPPORTED	0x100E
#define NACK_DEV_ERR			0x100F
#define NACK_CAPTURE_CANCELED	0x1010
#define NACK_INVALID_PARAM		0x1011
#define NACK_FINGER_IS_NOT_PRESSED	0x1012

#define COMMAND_PACKET			0x00
#define RESPONSE_PACKET			0x01
#define	DATA_PACKET				0x02

#define SB_OEM_PKT_SIZE			12
#define SB_OEM_HEADER_SIZE		2
#define SB_OEM_DEV_ID_SIZE		2
#define SB_OEM_CHK_SUM_SIZE		2

#define NONZERO					0x01
#define ZERO					0x00
#define DEVUCE_SERIAL_NUMBER_ERROR	0xFF

#define PKT_ERR_START	-500
#define PKT_COMM_ERR	PKT_ERR_START+1
#define PKT_HDR_ERR		PKT_ERR_START+2
#define PKT_DEV_ID_ERR	PKT_ERR_START+3
#define PKT_CHK_SUM_ERR	PKT_ERR_START+4
#define PKT_PARAM_ERR	PKT_ERR_START+5

#define FP_MAX_USERS		20


#define MASS_BUFFER	512

typedef struct _devinfo
{
	long FirmwareVersion;
	unsigned char DeviceSerialNumber[16];
} devinfo;

typedef struct {		
	unsigned char 	Head1;		
	unsigned char 	Head2;		
	unsigned char	wDevId0;
	unsigned char	wDevId1;
	unsigned char	nParam0;
	unsigned char	nParam1;
	unsigned char	nParam2;
	unsigned char	nParam3;
	unsigned char	wCmd0;// or nAck
	unsigned char	wCmd1;
	unsigned char 	wChkSum0;
	unsigned char 	wChkSum1;
} SB_OEM_PKT;

#ifdef __cplusplus
extern "C"
{
#endif

	int Open(int Flag,unsigned char* DataBuffer);
	int ChangeBaudRate(unsigned long BAUDRATE);
	int Termination();
	int Enroll_Start(int ID);
	int CaptureFinger(char cFlag);
	int Enroll_nth(char Count);
	int RemoveFinger();
	int GetEnrollCount();
	int Verify(int ID);
	int Identify();
	int DeleteID(unsigned char ID);
	int DeleteAll();
	int CheckEnroll(int ID);
	int LED(unsigned char State);
	int GetRawImage();
	int GetTemple(int ID,unsigned char* DataBuffer);
	int SetTemple(int ID,unsigned char* DataBuffer);
	int VerifyTemplate(int ID,unsigned char* DataBuffer);
	int IdentifyTemplate(int ID,unsigned char* DataBuffer);
	int GetDatabaseStart();
	int GetDatabaseEnd();
	int UpgradeFirmware();


#ifdef __cplusplus
}
#endif
#endif