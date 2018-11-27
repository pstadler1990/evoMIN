#include <string.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdio.h>
#include "evomin.h"

enum {
	EVOMIN_FRAME_SOF = 0xAA,
	EVOMIN_FRAME_EOF = 0x55,
	EVOMIN_FRAME_STFBYT = 0x55
};

enum {
	EVOMIN_STATE_INIT,
	EVOMIN_STATE_IDLE,
	EVOMIN_STATE_SOF,
	EVOMIN_STATE_SOF2,
	EVOMIN_STATE_CMD,
	EVOMIN_STATE_LEN,
	EVOMIN_STATE_PAYLD,
	EVOMIN_STATE_CRC,
	EVOMIN_STATE_CRC_OK,
	EVOMIN_STATE_CRC_FAIL,
	EVOMIN_STATE_EOF,
	EVOMIN_STATE_ERROR
};


static int8_t buffer_initialize(struct evoMin_Buffer* buffer, uint32_t size);
static int8_t buffer_push(struct evoMin_Buffer* buffer, uint8_t byte);
static struct evoMin_ResultState buffer_pop(struct evoMin_Buffer* buffer);
static void initialize_frame(struct evoMin_Frame* frame);


/* Initialize a blank evoMin interface */
void
evoMin_Init(struct evoMin_Interface* interface)
{
	interface->state = EVOMIN_STATE_INIT;

	memset(interface->receivedFrames, 0, EVOMIN_MAX_FRAMES * sizeof(struct evoMin_Frame));
	interface->currentFrameOffset = 0;
	interface->currentFrame = &interface->receivedFrames[interface->currentFrameOffset];
	initialize_frame(interface->currentFrame);

	interface->state = EVOMIN_STATE_IDLE;
}

void 
evoMin_SetTXHandler(struct evoMin_Interface* interface, void (*evoMin_Handler_TX)(uint8_t byte))
{
	interface->evoMin_Handler_TX = evoMin_Handler_TX;
}

/* The handler to be called from the low-level hardware receive interrupt, i.e. SPI_RX_IrQ */
void
evoMin_RXHandler(struct evoMin_Interface* interface, uint8_t cByte)
{
	switch(interface->state)
	{
		case EVOMIN_STATE_IDLE:

			if(cByte == EVOMIN_FRAME_SOF) {
				interface->state = EVOMIN_STATE_SOF;
			}
			else {
				goto error;
			}
			break;

		case EVOMIN_STATE_SOF:

			if(cByte == EVOMIN_FRAME_SOF) {
				interface->state = EVOMIN_STATE_SOF2;
			}
			else {
				goto error;
			}
			break;

		case EVOMIN_STATE_SOF2:

			if(cByte == EVOMIN_FRAME_SOF) {
				interface->state = EVOMIN_STATE_CMD;
			}
			else {
				goto error;
			}
			break;

		case EVOMIN_STATE_CMD:
			/* Store a reference to the current frame*/
			if(interface->currentFrameOffset >= EVOMIN_MAX_FRAMES) {
				interface->currentFrameOffset = 0;
			}

			interface->currentFrame = &interface->receivedFrames[interface->currentFrameOffset];

			/* initialize the current frame */
			initialize_frame(interface->currentFrame);

			interface->currentFrameBytesReceived = 0;

			interface->currentFrame->command = cByte;
			interface->state = EVOMIN_STATE_LEN;
			break;

		case EVOMIN_STATE_LEN:

			/* Store payload length in the current frame */
			interface->currentFrame->pLength = cByte;

			if(interface->currentFrame->pLength > EVOMIN_MAX_PAYLOAD_SIZE)
			{
				interface->currentFrame->buffer.status |= EVOMIN_BUF_STATUS_MASK_PTL;
				goto error;
			}

			interface->state = EVOMIN_STATE_PAYLD;
			break;

		case EVOMIN_STATE_PAYLD:

			/* For the reception of a frame body if two 0xAA bytes in a row are received
			   then the next received byte is discarded */
			if(interface->lastByteWasSTFBYT == 1)
			{
				interface->lastByteWasSTFBYT = -1;
				interface->lastRcvdByte = EVOMIN_FRAME_STFBYT;
				return;
			}
			if(cByte == EVOMIN_FRAME_SOF && interface->lastRcvdByte == EVOMIN_FRAME_SOF) {
				interface->lastByteWasSTFBYT = 1;
			}

			/* Store the payload data in the payload buffer */
			if(interface->currentFrameBytesReceived++ < interface->currentFrame->pLength)
			{
				int8_t copyState = buffer_push(&interface->currentFrame->buffer, cByte);

				if(!copyState)
				{
					/* Could not copy the whole payload into the pBuffer, FAIL */
					goto error;
				}
			}

			/* We've copied all payload data into the pBuffer */
			if(interface->currentFrameBytesReceived == interface->currentFrame->pLength) {
				interface->state = EVOMIN_STATE_CRC;
			}
			break;

		case EVOMIN_STATE_CRC:

			interface->currentFrame->crc8 = cByte;

			/* Check, if the transmitted crc8 equals the calculated one */
			if(interface->currentFrame->crc8 == evoMin_CRC8(interface->currentFrame->buffer.buffer, interface->currentFrame->pLength))
			{
				interface->currentFrame->isValid = 1;
				interface->state = EVOMIN_STATE_EOF;
			}
			else
			{
				interface->currentFrame->isValid = -1;
				interface->state = EVOMIN_STATE_CRC_FAIL;
			}
			break;

		case EVOMIN_STATE_CRC_FAIL:
			goto error;

		case EVOMIN_STATE_EOF:
			if(cByte == EVOMIN_FRAME_EOF)
			{
				interface->state = (interface->currentFrame->isValid) ? EVOMIN_STATE_IDLE : EVOMIN_STATE_ERROR;
				interface->receivedFrames[interface->currentFrameOffset];
				interface->currentFrameOffset++;

				/* external callback */
				evoMin_Handler_FrameRecvd(interface->currentFrame);
			}
			break;

		case EVOMIN_STATE_ERROR:
			/* TBD */
			goto error;
			break;

		default:
			interface->state = EVOMIN_STATE_ERROR;
	}

	interface->lastRcvdByte = cByte;
	return;

	error:
	{
		interface->currentFrame->isValid = -1;
		interface->state = EVOMIN_STATE_IDLE;
	};
}

uint8_t
evoMin_FrameGetDataByte(struct evoMin_Frame* frame, uint8_t n)
{
	uint8_t byte = 0;

	/* If an overflow occurs on the n-th byte offset,
	   store n as the new offset from 0 and return the difference */
	if(frame->buffer.headOffset + n < frame->buffer.size)
	{
		/* No overflow, return byte from buffer offset */
		byte = frame->buffer.buffer[n];
	}

	return byte;
}

/* Sends a frame (a frame to be sent over the low-level transport layer) from a command, a byte buffer and its length
   returns the number of remaining bytes, if the byte buffer couldn't be packed into a single frame */
uint8_t
evoMin_sendFrame(struct evoMin_Interface* interface, uint8_t command, uint8_t* bytes, uint32_t bLength)
{
	uint8_t nextByte = 0;
	uint8_t lastByte = 0;
	uint32_t pCnt = 0;
	uint32_t crcCnt = 0;
	uint32_t bytesToSend = bLength;
	const uint32_t crcBufSize = bytesToSend + 1 + 1;
	int8_t foundPlHeader = -1;

	/* Send SOF bytes */
	interface->evoMin_Handler_TX(EVOMIN_FRAME_SOF);
	interface->evoMin_Handler_TX(EVOMIN_FRAME_SOF);
	interface->evoMin_Handler_TX(EVOMIN_FRAME_SOF);

	/* A transport frame always has a payload length of EVOMIN_MAX_PAYLOAD_SIZE while initialization */

	/* Pre-initialize the crc buffer */
	/* 0	command
	   1	len
	   2..n payload data */
	uint8_t crcBuffer[crcBufSize];
	crcBuffer[0] = command;
	crcBuffer[1] = bytesToSend;
	crcCnt = 1;

	/* Send command byte */
	interface->evoMin_Handler_TX(command);

	/* If the payload length is larger than the maximum payload size,
	   only send EVOMIN_MAX_PAYLOAD_SIZE bytes */
	if(bLength > EVOMIN_MAX_PAYLOAD_SIZE) {
		bytesToSend = EVOMIN_MAX_PAYLOAD_SIZE;
	}
	/* Send payload length */
	interface->evoMin_Handler_TX(bytesToSend);

	for(uint32_t cCnt = 0; cCnt < bytesToSend && pCnt < EVOMIN_MAX_PAYLOAD_SIZE; cCnt++)
	{
		if(foundPlHeader == 1)
		{
			interface->evoMin_Handler_TX(EVOMIN_FRAME_STFBYT);
			lastByte = EVOMIN_FRAME_STFBYT;

			foundPlHeader = -1;
		}
		/* For the transmission of a frame body if two 0xAA bytes in a row are transmitted a stuff byte with value 0x55 
	       is inserted into the transmitted byte stream */
		if(bytes[cCnt] == EVOMIN_FRAME_SOF && lastByte == EVOMIN_FRAME_SOF) {
			foundPlHeader = 1;
		}

		nextByte = bytes[cCnt];

		crcBuffer[++crcCnt] = bytes[cCnt];
		lastByte = bytes[cCnt];

		/* Send next payload byte */
		interface->evoMin_Handler_TX(nextByte);
	}

	/* Calculate and send the crc8 for the complete frame (excluding the crc8 itself) */
	uint8_t crc8 = evoMin_CRC8(crcBuffer, crcBufSize);
	interface->evoMin_Handler_TX(crc8);

	/* SEND EOF byte */
	interface->evoMin_Handler_TX(EVOMIN_FRAME_EOF);

	/* Return the remaining number of bytes, if any 
	   This allows the creation of another frame with the remaining bytes */
	return bLength - pCnt;
}

static int8_t
buffer_initialize(struct evoMin_Buffer* buffer, uint32_t size)
{
	buffer->buffer = calloc(size, sizeof(uint8_t));
	if(!buffer->buffer) {
		return -1;
	}
	memset(buffer->buffer, 0, size * sizeof(uint8_t));
	buffer->headOffset = 0;
	buffer->tailOffset = 0;
	buffer->size = size;
	buffer->status = EVOMIN_BUF_STATUS_MASK_INIT;
	return 1;
}


/* Push a byte onto the buffer */
static int8_t
buffer_push(struct evoMin_Buffer* buffer, uint8_t byte)
{
	if(buffer->tailOffset + 1 >= buffer->size)
	{
		buffer->status |= EVOMIN_BUF_STATUS_MASK_NES;
		return -1;
	}

	buffer->buffer[buffer->tailOffset++] = byte;
	return 1;
}

static void
initialize_frame(struct evoMin_Frame* frame)
{
	frame->isValid = -1;
	frame->command = EVOMIN_CMD_RESERVED;
	frame->pLength = 0;

	/* Initialize the payload buffer */
	buffer_initialize(&frame->buffer, EVOMIN_P_BUF_SIZE);

	frame->crc8 = 0;
}