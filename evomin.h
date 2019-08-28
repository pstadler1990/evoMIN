/*
 EVOMIN transport protocol
 
 Procedure:
 1. Enable transport layer (low-level), i.e. I2C, SPI
 2. Use evoMin_Init() to initialize a passed instance of evoMin_Interface
 3. evoMin_RXHandler() needs to be called from within the low-level transport routine (i.e. IRQHandler for SPI/I2C/UART)
 with the received byte (if you want to receive data, otherwise just skip this method)
 4. Whenever a new, valid frame is received, the evoMin_Handler_FrameRecvd() callback gets called from evoMIN
 5. To send data, one must implement a TX callback method (low-level implementation for the send transport layer, i.e. I2C, SPI)
 and assign the implemented callback to the interface by using evoMin_SetTXHandler()
 If the device is receive-only, you can disable sending by compiling with the EVOMIN_TX_DISABLE statement.
 
 To send a frame:
  - First create a transport frame for your command and payload using evoMin_createFrame()
    This method needs to be called via a user allocated evoMin_Frame
  - MAKE SURE TO INITIALIZE THE FRAME FIRST, using evoMin_InitializeFrame()!
  - To eventually send the previously created frame, use the evoMin_sendFrame() method and pass it the created frame.
    This function can try to resend a frame, if the received checksum from the target device doesn't match with the calculated crc in the sendFrame()-method. 
	You can define the retry count in evomin.h EVOMIN_SEND_RETRIES_ON_FAIL (default: 3)
  
  Example for sending a frame:
  ----------------------------
  struct evoMin_Frame sendFrame;
  // ...
  evoMin_InitializeFrame(&sendFrame);
  evoMin_createFrame(FPGA_Interface.comInterface, &sendFrame, command, payload, payloadLength);
  ResultState_t sendResult = evoMin_sendFrame(FPGA_Interface.comInterface, &sendFrame);	// sendResult is type_OutOfBounds with src_evoMIN_SENDFRAME if the 
																						// frame couldn't be send within EVOMIN_SEND_RETRIES_ON_FAIL retries
  // ...
  
 Buffer status management:
 The buffer of each individual frame can be checked against overflows etc. Therefor, use the EVOMIN_BUF_STATUS_MASK_xxx masks

 CRC8:
 The checksum CRC8 is only calculated over the payload bytes EXCLUDING the stuff bytes and also EXCLUDING the frame header bytes,
 but INCLUDING the command and the payload length byte!
 0	 	command byte
 1	 	payload length byte
 2..n 	payload bytes
*/
#ifndef __EVOMIN_H_
#define __EVOMIN_H_

#include "evoMIN_commands.h"
#include "evoErrorHandler.h"

#define EVOMIN_FRAME_SIZE 				(uint32_t)7	/* 3 sof bytes, 1 cmd byte, 1 length byte, 1 crc byte, 1 eof byte */
#define EVOMIN_MAX_PAYLOAD_SIZE			(uint32_t)50
#define EVOMIN_P_BUF_SIZE				(uint32_t)EVOMIN_MAX_PAYLOAD_SIZE
#define EVOMIN_BUFFER_SIZE				(uint32_t)EVOMIN_P_BUF_SIZE + 2
#define	EVOMIN_SEND_RETRIES_ON_FAIL		(uint32_t)3
#define EVOMIN_SEND_RETRY_MIN_TIME		(uint32_t)3	// TODO: Replace with real value

/* number of frames to hold, after EVOMIN_MAX_FRAMES frames the frames are overwritten */
#define EVOMIN_MAX_FRAMES				(uint32_t)4

/* Status masks for the rx buffer
 RXne TXne 0 0 0 0 0 INIT */
#define	EVOMIN_BUF_STATUS_MASK_INIT		0x01
#define	EVOMIN_BUF_STATUS_MASK_NES		0x40	/* Not enough space error (if the required amount of data fits in the buffer, but is
											   	   larger than the currently available space */
#define EVOMIN_BUF_STATUS_MASK_PTL		0x80	/* Payload too large (if the required amount of data is larger than the buffer) */

struct evoMin_Buffer {
	uint8_t buffer[EVOMIN_BUFFER_SIZE];
	uint32_t size;
	uint32_t headOffset;
	uint32_t tailOffset;
	uint8_t status;
};

/* Frames are the internal representation of a received packet, 
 including the command, the payload offset and length and the crc8 */
struct evoMin_Frame {
  	uint8_t isInitialized;
	uint8_t isValid;
	uint8_t isSent;
	uint8_t command;

	struct evoMin_Buffer buffer;
	uint8_t pLength;
	uint8_t crc8;

	uint32_t timestamp;
	
	uint32_t retriesLeft;
};

struct evoMin_Interface {

	struct evoMin_Frame queue[EVOMIN_MAX_FRAMES];
	uint32_t queuePtrW;
	uint32_t queuePtrR;
	struct evoMin_Frame* currentFrame;
	struct evoMin_Frame receivedFrames[EVOMIN_MAX_FRAMES];

	uint32_t currentFrameOffset;

	uint8_t currentFrameBytesReceived;
	uint8_t lastRcvdByte;
	int8_t 	lastByteWasSTFBYT;

	struct evoMin_Frame forcedFrame;

	uint8_t state;

	/* Interface from the hardware low-level, must be implemented if you require sending data */
#ifndef EVOMIN_TX_DISABLE
	uint8_t (*evoMin_Handler_TX)(uint8_t byte);
#endif
};

void evoMin_Init(struct evoMin_Interface* interface);
void evoMin_DeInit(struct evoMin_Interface* interface);
void evoMin_SetTXHandler(struct evoMin_Interface* interface, uint8_t (*evoMin_Handler_TX)(uint8_t byte));
void evoMin_InitializeFrame(struct evoMin_Frame* frame);
uint8_t evoMin_CreateFrame(struct evoMin_Frame* frame, uint8_t command, uint8_t* bytes, uint8_t bLength);
ResultState_t evoMin_QueueFrame(struct evoMin_Interface* interface, struct evoMin_Frame frame);
uint8_t evoMin_SendFrameImmediately(struct evoMin_Interface* interface, struct evoMin_Frame frame);
void evoMin_SendResendLastFrame(struct evoMin_Interface* interface);
uint8_t evoMin_FrameGetDataByte(struct evoMin_Frame* frame, uint8_t n);

/* -- Custom handlers, must be implemented on the application side -- */

/* evoMin_RXHandler must be called through the low-level byte receive method, i.e. the SPI RX IRQHandler
 whenever a byte is received on the line */
ResultState_t evoMin_RXHandler(struct evoMin_Interface* interface, uint8_t cByte);

/* Application dependent CRC8 calculation, must be implemented! */
uint8_t evoMin_CRC8(uint8_t* bytes, uint32_t bLen);

/* evoMin_Handler_FrameRecvd callback gets called by evoMIN whenever a new, valid frame has been received
 It contains a pointer to the received frame, including the command, it's payload length and the payload itself
 in a buffer */
void evoMin_Handler_FrameRecvd(struct evoMin_Frame* frame);

#endif