/******************************************************************
* rfm12.c
*
* HopeRf RFM12 FSK Wireless Module Library
*
* Copyright (c) 2007 Stephen Eaton
* 	seaton@everythingrobotics.com
*
* ===========================================================
* License:
*
*   Creative Commons - Attribution-Noncommercial-Share Alike 
*
* 	AU - http://creativecommons.org/licenses/by-nc-sa/2.5/au/
*	US - http://creativecommons.org/licenses/by-nc-sa/3.0/us/
*
*	All Other Countries please refer to the: 
*	  Attribution-Noncommercial-Share Alike License 
*	  for your country found on http://creativecommons.org/
*
* ===========================================================
*
******************************************************************/

#include <system.h>
#include "rfm12.h"
#include "..\common.h"

// Declaration of local variables.

/*
 The buffer is used for transmitting and receiving
 */
uchar buffer[32];
uchar txrx_pointer;
uchar txrx_counter;

// Global variable to this function, contains all the settings
rfm12_t rfm12_conf;



// function Declarations
uint WriteCMD( uint cmd);
void Write0(void);
void Write1(void);
void Init(void);
void rfm12_Basic_Config();				// basic config for the RFM12

//	state machine
void goto_poweringtransmitter_state(void);
void goto_transmitting_state(void);
void goto_receiving_state(void);
void goto_quiettime_state(void);


//
// Initialiazes the RFM Module to default settings
void Init(void)
{
	
	// Define Module control Pins
	
	// outputs
	RFM12_CS_OUT();								// Module Chip Select 
	rfm12_cs=1;									// Active Low so set High
	
	SCK_OUT();									// SPI SCK as Output
	SCK=0;
	
	SDO_OUT();									// SPI SDO as Output
	SDO=0;
		
	// inputs
	RFM12_nIRQ_IN();							// nIRQ
	SDI_IN();									// SPI SDI
	
	// reset buffer pointers
	txrx_pointer = 0;
	
	// initialiase state machine
	goto_poweringtransmitter_state();
}

/*
	Initialiase RFM12 to 915Mhz Band with basic config
 */
void rfm12_Init_915(void)
{
	Init();										// setup Module I/O
	rfm12_Basic_Config();						// Basic config
	WriteCMD(RFM12_CMD_CONFIG | (RFM12_BAND_915 << RFM12_8000_B_OFF)); // 915Mhz Band
	
	goto_quiettime_state();
}


/*
	Initialiase RFM12 to 433Mhz Band with basic config
 */
void rfm12_Init_433(void)
{
	Init();										// setup Module I/O
	rfm12_Basic_Config();						// Basic config
	WriteCMD(RFM12_CMD_CONFIG | (RFM12_BAND_433 << RFM12_8000_B_OFF));	// 433Mhz Band
	
	goto_quiettime_state();
}


/*
 configures the RFM12 with a basic working config 
*/
void rfm12_Basic_Config(void)
{
	WriteCMD(0xC0E0);			// CLK: 10MHz
	WriteCMD(0x80D7);			// Enable FIFO
	WriteCMD(0xC2AB);			// Data Filter: internal
	WriteCMD(0xCA81);			// Set FIFO mode
	WriteCMD(0xE000);			// disable wakeuptimer
	WriteCMD(0xC800);			// disable low duty cycle
	WriteCMD(0xC4F7);			// AFC settings: autotuning: -10kHz...+7,5kHz
	rfm12_conf.status = WriteCMD(0x0000);
}


/*
	Sets the RF Baud Rate
	must be the same at both ends i.e. Tx and Rx
	
	parameters:
		baud: baudrate in bps  e.g. 19200 or 2400
*/
void rfm12_SetBaud(uint baud)
{
	if(baud<664)
		baud = 664;						// min allowed baudrate
	if(baud<5400)						// 344827.58621 / (1 + cs*7) / (baud-1)
		WriteCMD(0xC680|(43104/baud)-1);  //CS=1 (344828/8)/baud-1
	else
		WriteCMD(0xC680|(344828/baud)-1); //CS=0 344828/Baud-1
		
}

/*
	Sets the Tx Power and Modulation
*/
void rfm12_SetPower(uchar power, uchar modulation)
{
	WriteCMD(RFM12_CMD_TX_CTL|((power & RFM12_9800_P_MASK)|(modulation & RFM12_9800_M_MASK) << RFM12_9800_M_OFF));
}

/*
	Sets the RF frequency of the RFM12
	
	Parameters
		freq: Centre Frequency bit value
		
		the bit value can be calculaed by:
		
		433MHz(FREQ-430)/0.0025  where FREQ is centre frequency in MHz
		915MHz(FREQ-900)/0.0075  where FREQ is centre frequency in MHz
		
		or use the helper macros to calculate the register bit value 
		
			RFM12_FREQ_433(FREQ)
			RFM12_FREQ_915(FREQ)
		
		e.g. rfm12_SetFrequency(RFM12_FREQ_915(921.25));
		this sets the frequency to 921.25MHz
*/
void rfm12_SetFreq(uint freq)
{
	if (freq<96)				// lower limit 430.2400MHz or 900.7200MHZ
		freq=96;
	else if (freq>3903)			// upper limit 439.7575MHz or 929.2725MHz
		freq=3903;
	WriteCMD(RFM12_CMD_FREQ|freq);
}



// Bitbang binary 1 via SPI
void Write1(void)
{
	SDO=1;
	SCK=0;
	nop();
	nop();
	nop();
	nop();
	nop();
	nop();
	nop();
	nop();
	nop();
	nop();
	nop();
	nop();
	nop();
	nop();
	nop();
	nop();
	
	SCK=1;
	
	nop();
}

// bitbang binary 0 via SPI
void Write0(void)
{
	SDO=0;
	SCK=0;
	nop();
	nop();
	nop();
	nop();
	nop();
	nop();
	nop();
	nop();
	nop();
	nop();
	nop();
	nop();
	nop();
	nop();
	nop();
	nop();
	
	SCK=1;
	
	nop();
}

/*

  Write a Command to the RFM Module using SPI
  
  Requires: 16bit valid command
  Returns:  16bit result from transaction
  
  This is a bi-directional transfer.  
  A command is clocked out to the RFM a bit at a time.  
  At the same time a result is clocked back in a bit at a time.
   
*/
uint WriteCMD(uint CMD)
{
	
	uint RESULT = 0;							// Holds the received SDI
	uchar n=16;									// number of bits in SPI Command we need to send
	SCK=0;										// SCK LOW
	rfm12_cs = 0;								// CS LOW
	while(n--)									// Send All 16 Bits MSB First
	{
		if (CMD&0x8000)
			Write1();							// Write 1 via SDI
		else
			Write0();							// Write 0 via SPI
		
		CMD<<=1;								// Shift left for next bit to send
		RESULT<<=1;								// Shift left for next bit to receive
		if(SDI)									// Check if we received a high on SDI
			RESULT |= 0x0001;					// RESULT LSB = 1		
	}
	SCK=0;										// SCK LOW
	rfm12_cs = 1;								// CS HIGH - Finished Sending Command
}

/*
	Turns on the Transmitter
*/
void rfm12_EnableTx(void)
{
	goto_transmitting_state();
}

void rfm12_DisableTx(void)
{
	goto_quiettime_state();
}

void rfm12_EnableRx(void)
{	
	goto_receiving_state();
}

void rfm12_disableRx(void)
{
	goto_quiettime_state();
}


/*
 Handles the interrupt generated by the nIRQ of the RFM12 
 
 This routine should be used as a callback from the main application 
 interrupt routine.
 
 If we get here then its for one of the following reasons:
 
	 (in bit order as clocked out as status bit)
	  /RGIT - TX Register Ready to receive the next byte(Cleared by Transmit Write)
	  \FFIT - RX FIFO has reached preprogrammed limit	(Cleared by any FIFO Read)
	  POR  - Power-on reset occurred					(cleared on status read)
	  /RGUR - TX Register underrun, register overwrite	(cleared on status read)
	  \FFOV - RX FIFO Overflow							(cleared on status read)
	  WKUP - Wakeup Timer overflow   					(cleared on status read)
	  EXT  - LOW on External Pin 16
	  LBD  - Low battery detected, powersupply is below the preprogrammed threshold
 */
void rfm12_Handle_Interrupt(void)
{
	/*rfm12_conf.status = WriteCMD(RFM12_CMD_STATUS_READ);	// Updates the config data with lastest Status

	if(rfm12_conf.status & isRGITFFIT) 				// Check RGIT / FFIT 
	{
		switch (rfm12_conf.txrx_mode)				// Check Tx or Rx
		{
			case RFM12_MODE_MASTER_RX:
			case RFM12_MODE_SLAVE_RX:
					//TODO:							// handle Rx Interrupt
					break;
						
			case RFM12_MODE_MASTER_TX:
			case RFM12_MODE_SLAVE_TX:
					//TODO:							// handle Tx interrupt
					break;
		}
		
	}
	
	if(rfm12_conf.status & isPOR)	// Check Power on reset
	{
			//TODO: handle POR
	}
	
	if(rfm12_conf.status & isRGURFFOV)  // Check RGUR /Check FFOV status bit
	{
		switch (rfm12_conf.txrx_mode)				// Check Tx or Rx
		{
			case RFM12_MODE_MASTER_RX:
			case RFM12_MODE_SLAVE_RX:
					//TODO:								// handle Rx Interrupt
					break;
						
			case RFM12_MODE_MASTER_TX:
			case RFM12_MODE_SLAVE_TX:
					//TODO:								// handle Tx interrupt
					break;
		}
	}
	
	if(rfm12_conf.status & isWKUP)	// Check Wakeup status bit
	{
			//TODO: handle Wakeup
	}
	
	if(rfm12_conf.status & isEXT)	// Check EXT Interrupt status bit
	{
			//TODO: handle External
	}
	
	if(rfm12_conf.status & isLBD)	// Check Low battery status bit
	{
			//TODO: handle Low Battery
	}*/
}



/* 
  Waits for RFM 12 to become ready by checking first bit of status register
  Before we can Tx or Rx data the RFM12 needs to be ready, 
  i.e. not in the middle of transmitting a previous byte
  
  This function is blocking and will only return when it is ready to Tx or Rx
*/
void rfm12_WaitReady(void)
{	
	bit READY = 0;
	while(!READY)						
	{
		SCK = 0;
		rfm12_cs = 0;					// !cs LOW - enable
	
		SDO = 0;						// clock out a 0 = status command
		SCK = 1;						// SCK HIGH 
		if(SDI)							// check RGIT bit of status (first bit)
		{ 								// If HIGH then Tx/RX is ready to accept the next byte
			READY = 1;
		}
		else
		{
			READY = 0;
		}
		
		SCK = 0;						// SCK LOW
		SDO = 1;						// SDO HIGH
		rfm12_cs = 1;					// !cs HIGH - disable
	}
}

/* 
  Waits for RFM12 to become ready by checking first bit of status register
  Before we can Tx or Rx data the RFM12 needs to be ready, 
  i.e. not in the middle of transmitting or receiving 
  
  This will return after a set amount of time
  
  Returns:  0 = Not ready/Timeout
			1 = Ready
*/
uchar rfm12_isReady(void)
{	
	bit READY = 0;
	uchar timeout = 1000;
	
	while(!READY && timeout)
	{
		SCK = 0;
		rfm12_cs = 0;					// !cs LOW - enable
	
		SDO = 0;						// clock out a 0 - status command request
		SCK = 1;						// SCK HIGH 
		if(SDI)							// check RGIT bit of status (first bit)
		{ 								// If HIGH then Tx/RX is ready to accept the next byte
			READY = 1;
		}
		else
		{
			READY = 0;
		}
		timeout--;						// check time
		
		SCK = 0;						// SCK LOW
		SDO = 1;						// SDO HIGH
		rfm12_cs = 1;					// !cs HIGH - disable
	}
	
	if(timeout==0)
		return 0;						// Not Ready - timeout
	else
		return 1;						// Ready
}


/*
	loads a byte into the tx buffer
 */
void rfm12_Load_Byte(uchar data)
{
	buffer[txrx_pointer] = data;
	txrx_pointer++;
}

/*
	Reads and returns data from the FIFO of rfm12
	
	Will return 0 if nothing received within timeout period
 */
uchar rfm12_ReadFifo(void)
{
	rfm12_ResetFifo(); 									// reset rm12 fifo ready to receive next byte
	if(rfm12_isReady())									// wait with timeout until byte is received
		return WriteCMD(RFM12_CMD_FIFO_READ) & 0xFF;	// return the contents of fifo
	else
		return 0;
}

/*
	Sets the bandwidth, gain and drssi for the Receiver
*/
void rfm12_SetBandwidth(uchar bandwidth, uchar gain, uchar drssi)
{
	WriteCMD(0x9400|((bandwidth & RFM12_9000_I_MASK) << RFM12_9000_I_OFF)
				| ((gain & RFM12_9000_G_MASK) << RFM12_9000_G_OFF)
				| (drssi & RFM12_9000_R_MASK)); // P16 enabled
}
	

/*
  Sends the tx buffer

  Blocking - will not return until all bytes sent
  
  Assumes that RF is enabled
  
*/
void rfm12_Tx_Buffer(void)
{
	txrx_counter = txrx_pointer;
	txrx_pointer = 0;
	
	if(txrx_counter==0)				// nothing to send so exit
		return;		
	
	while(txrx_counter != 0);			// tx until the buffer is empty
	{
		rfm12_Tx_Byte(buffer[txrx_pointer++]);
		txrx_counter--;
	}
	
	txrx_pointer=0;						// reset the buffer
}

/*
	Receives RF data into buffer
	parameter:
		count = number of bytes to receive
 */
void rfm12_Rx_Data(uchar count)
{

	txrx_counter = count;
	txrx_pointer = 0;
	
	if(txrx_counter==0)					// nothing to receive so exit
		return;		
	
	while(txrx_counter != 0);			// rx until the we reach the count
	{
		buffer[txrx_pointer++]=rfm12_ReadFifo();  // get data from RFM12 fifo
		txrx_counter--;
	}
	
	txrx_pointer=0;							// reset the buffer
}
 	
/*
	Resets the FIFO on the RFM12 
	ready to receive the next byte
*/
void rfm12_ResetFifo(void)
{
	WriteCMD(0xCA81); 				// disable FIFO
	WriteCMD(0xCA83); 				// Enable FIFO and ready to receive next Byte
}

// resets the txrx buffer
void rfm12_Init_Buffer(void)
{
	txrx_pointer = 0;
	return;
}

/*
  Transmits a single byte via RF Transmitter
 
	Blocking - will not return until byte transmitted

	assumes: el bit (bit 7) is cleared in config register
	assumes: er bit (bit 8) is cleared pwr mgmt register
	assumes: et bit (bit 5) is set in pwr mgmt register
 */
void rfm12_Tx_Byte(uchar data)
{
	rfm12_WaitReady();			 	 	  // Wait until RFM is ready to Tx
	WriteCMD(RFM12_CMD_TX_WRITE | data);  // write data to transmit register	
}

/*******************************************************************************
 * State machine
 ******************************************************************************/
 
/*
	transmitting state
*/
void goto_transmitting_state(void)
{
	rfm12_conf.state = RFM12_STATE_TRANSMITTING;
	WriteCMD(0x8238);			// sets EL Bit - Tx On
}

/*
	Receiving state
 */
void goto_receiving_state(void)
{
	rfm12_conf.state = RFM12_STATE_RECEIVING;
	WriteCMD(0x82C8);			// turn RX on
}

/*
	quiet time state
 */
void goto_quiettime_state(void)
{
 	rfm12_conf.state = RFM12_STATE_QUIETTIME;
 	WriteCMD(0x8258);			// turn off Rx and RX (EL and ET bits)
}

/*
	powering transmitter state
 */
void goto_poweringtransmitter_state(void)
{
 	rfm12_conf.state = RFM12_STATE_POWERINGTRANSMITTER;
 	//TODO:
}
	