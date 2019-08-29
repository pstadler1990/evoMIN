#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include "evomin.h"
#include "evoErrorHandler.h"
#include "ErrorHandler_EvoMIN.h"
#include "evoMIN_tx_impl.h"

enum {
	EVOMIN_FRAME_SOF = 0xAA, EVOMIN_FRAME_EOF = 0x55, EVOMIN_FRAME_STFBYT = 0x55, EVOMIN_FRAME_ACK = 0xFF
};

enum {
	EVOMIN_STATE_UNDEF = 0,
	EVOMIN_STATE_INIT,
	EVOMIN_STATE_IDLE,
	EVOMIN_STATE_SOF,
	EVOMIN_STATE_SOF2,
	EVOMIN_STATE_CMD,
	EVOMIN_STATE_LEN,
	EVOMIN_STATE_PAYLD,
	EVOMIN_STATE_CRC,
	EVOMIN_STATE_CRC_FAIL,
	EVOMIN_STATE_EOF,
	EVOMIN_STATE_MSG_SENT_WAIT_FOR_ACK,
	EVOMIN_STATE_ERROR
};

static void initialize_frame(struct evoMin_Frame* frame);
#ifndef EVOMIN_TX_DISABLE
ResultState_t send_frame(struct evoMin_Interface* interface, struct evoMin_Frame* frame);
#endif
static uint8_t buffer_initialize(struct evoMin_Buffer* buffer);
static uint8_t buffer_push(struct evoMin_Buffer* buffer, uint8_t byte);
static struct evoMin_Frame* queue_get_active_frame(struct evoMin_Interface* interface);


void 
evoMin_Init(struct evoMin_Interface* interface) {
	/* Initialize a blank evoMin interface */
	interface->state = EVOMIN_STATE_INIT;

	memset(interface->receivedFrames, 0, EVOMIN_MAX_FRAMES * sizeof(struct evoMin_Frame));
	memset(interface->queue, 0, EVOMIN_MAX_FRAMES * sizeof(struct evoMin_Frame));
	interface->queuePtrW = 0;
	interface->queuePtrR = 0;
	interface->currentFrameOffset = 0;
	interface->lastByteWasSTFBYT = -1;
	interface->forcedFrame = (struct evoMin_Frame) {};
#ifndef EVOMIN_TX_DISABLE
	interface->evoMin_Handler_TX = 0;
#endif
	interface->currentFrame = &interface->receivedFrames[interface->currentFrameOffset];
	initialize_frame(interface->currentFrame);
	
	interface->state = EVOMIN_STATE_IDLE;
}

void
evoMin_DeInit(struct evoMin_Interface* interface) {
	/* DeInitialize given evoMin interface */
	interface->state = EVOMIN_STATE_UNDEF;
	interface->currentFrame = 0;
	interface->lastByteWasSTFBYT = -1;
	interface->currentFrameOffset = 0;
	interface->currentFrameBytesReceived = 0;
	interface->lastRcvdByte = 0;
	interface->queuePtrR = 0;
	interface->queuePtrW = 0;
	interface->forcedFrame = (struct evoMin_Frame) {};
#ifndef EVOMIN_TX_DISABLE
	interface->evoMin_Handler_TX = 0;
#endif

	/* Free buffers */
	for(uint32_t fr = 0; fr < EVOMIN_MAX_FRAMES; fr++) {
		interface->receivedFrames[fr].isInitialized = 0;
		free(interface->receivedFrames[fr].buffer.buffer);
	}
}

#ifndef EVOMIN_TX_DISABLE
void 
evoMin_SetTXHandler(struct evoMin_Interface* interface, uint8_t (*evoMin_Handler_TX)(uint8_t byte)) {
	interface->evoMin_Handler_TX = evoMin_Handler_TX;
}
#endif

ResultState_t 
evoMin_RXHandler(struct evoMin_Interface* interface, uint8_t cByte) {
	/* The handler to be called from the low-level hardware receive interrupt, i.e. SPI_RX_IrQ */
  	ResultState_t resultState = CreateResultState(type_NoError, src_evoMIN, prio_None);
	
	int8_t copyState;
	switch (interface->state) {
		case EVOMIN_STATE_MSG_SENT_WAIT_FOR_ACK:
			/* Reception of a ACK byte while in this state
			   indicates the reception of an evoMIN message on the target side */
			if(cByte == EVOMIN_FRAME_ACK) {
				/* By setting isSent we indicate that this frame has been received - and thus sent - with success.
				   It will be dequeued and cleared within the next loop */
				if(!interface->forcedFrame.isInitialized) {
					queue_get_active_frame(interface)->isSent = 1;
				} else {
					/* There's a forced frame that has been sent */
					interface->forcedFrame.isSent = 1;
				}
				interface->state = EVOMIN_STATE_IDLE;
			} else {
				CreateResultState(type_Unknown, src_evoMIN + src_evoMIN_ACK, prio_Low);
				goto error;
			}
			break;

		case EVOMIN_STATE_IDLE:
			if (cByte == EVOMIN_FRAME_SOF) {
				interface->state = EVOMIN_STATE_SOF;
			} else {
				CreateResultState(type_Unknown, src_evoMIN + src_evoMIN_SOF, prio_Low);
				goto error;
			}
			break;

		case EVOMIN_STATE_SOF:
			if (cByte == EVOMIN_FRAME_SOF) {
				interface->state = EVOMIN_STATE_SOF2;
			} else {
				resultState = CreateResultState(type_Unknown, src_evoMIN + src_evoMIN_SOF, prio_Low);
				goto error;
			}
			break;

		case EVOMIN_STATE_SOF2:
			if (cByte == EVOMIN_FRAME_SOF) {
				interface->state = EVOMIN_STATE_CMD;
			} else {
				resultState = CreateResultState(type_Unknown, src_evoMIN + src_evoMIN_SOF, prio_Low);
				goto error;
			}
			break;

		case EVOMIN_STATE_CMD:
			/* Store a reference to the current frame*/
			interface->currentFrame = &interface->receivedFrames[interface->currentFrameOffset];

			/* initialize the current frame */
			initialize_frame(interface->currentFrame);

			interface->currentFrameBytesReceived = 0;
			interface->currentFrame->command = cByte;
			buffer_push(&interface->currentFrame->buffer, cByte);
			interface->state = EVOMIN_STATE_LEN;
			break;

		case EVOMIN_STATE_LEN:
			/* Store payload length in the current frame */
			interface->currentFrame->pLength = cByte;

			if (interface->currentFrame->pLength > EVOMIN_MAX_PAYLOAD_SIZE) {
				interface->currentFrame->buffer.status |= EVOMIN_BUF_STATUS_MASK_PTL;
				resultState = CreateResultState(type_OutOfBounds, src_evoMIN + src_evoMIN_LEN, prio_Low);
				goto error;
			}

			buffer_push(&interface->currentFrame->buffer, cByte);

			if(interface->currentFrame->pLength !=0) {
				interface->state = EVOMIN_STATE_PAYLD;
			} else {
				interface->state = EVOMIN_STATE_CRC;
			}
			break;

		case EVOMIN_STATE_PAYLD:
			/* For the reception of a frame body if two 0xAA bytes in a row are received
			 then the next received byte is discarded */
			if (interface->lastByteWasSTFBYT == 1) {
				interface->lastByteWasSTFBYT = -1;
				interface->lastRcvdByte = EVOMIN_FRAME_STFBYT;
				return resultState;
			}
			if (cByte == EVOMIN_FRAME_SOF && interface->lastRcvdByte == EVOMIN_FRAME_SOF) {
				interface->lastByteWasSTFBYT = 1;
			}

			/* Store the payload data in the payload buffer */
			if (interface->currentFrameBytesReceived < interface->currentFrame->pLength) {
				copyState = buffer_push(&interface->currentFrame->buffer, cByte);
				interface->currentFrameBytesReceived++;

				if (!copyState) {
					/* Could not copy the whole payload into the pBuffer, FAIL */
					resultState = CreateResultState(type_OutOfBounds, src_evoMIN + src_evoMIN_PAYLD, prio_Low);
					goto error;
				}
			}

			/* We've copied all payload data into the pBuffer */
			if (interface->currentFrameBytesReceived == interface->currentFrame->pLength) {
				interface->state = EVOMIN_STATE_CRC;
			}
			break;

		case EVOMIN_STATE_CRC:
			interface->currentFrame->crc8 = cByte;

			/* Check, if the transmitted crc8 equals the calculated one */
			if (interface->currentFrame->crc8 == evoMin_CRC8(interface->currentFrame->buffer.buffer, interface->currentFrame->pLength+2)) {
				interface->currentFrame->isValid = 1;
				interface->state = EVOMIN_STATE_EOF;

				/* Send ACK byte to acknowledge reception of message (pre-fill TX buffer to be ready at the EOF byte) */
				interface->evoMin_Handler_TX(EVOMIN_FRAME_ACK);

			} else {
				interface->currentFrame->isValid = 0;
				interface->state = EVOMIN_STATE_CRC_FAIL;
			}
			break;

		case EVOMIN_STATE_CRC_FAIL:
			resultState = CreateResultState(type_Unknown, src_evoMIN + src_evoMIN_CRC, prio_Low);
			goto error;

		case EVOMIN_STATE_EOF:
			if (cByte == EVOMIN_FRAME_EOF) {
				interface->state = (interface->currentFrame->isValid) ? EVOMIN_STATE_IDLE : EVOMIN_STATE_ERROR;
				interface->currentFrameOffset = (interface->currentFrameOffset+1) % EVOMIN_MAX_FRAMES;

				/* external callback */
				evoMin_Handler_FrameRecvd(interface->currentFrame);
			} else {
				resultState = CreateResultState(type_OutOfBounds, src_evoMIN + src_evoMIN_SOF, prio_Low);
				goto error;
			}
			break;

		case EVOMIN_STATE_ERROR:
			resultState = CreateResultState(type_Unknown, src_evoMIN, prio_Low);
			goto error;

		default:
			interface->state = EVOMIN_STATE_ERROR;
	}

	interface->lastRcvdByte = cByte;
	return resultState;

	error: 
		interface->currentFrame->isValid = 0;
		interface->state = EVOMIN_STATE_IDLE;
		return resultState;
}

uint8_t 
evoMin_FrameGetDataByte(struct evoMin_Frame* frame, uint8_t n) {
	uint8_t byte = 0;
	/* Add an offset of 2 to n, as we also store the command and payload length byte in the payload data */
	if (frame->buffer.headOffset + (n + 2) < (frame->buffer.size + 2)) {
		/* No overflow, return byte from buffer offset */
		byte = frame->buffer.buffer[n + 2];
	}

	return byte;
}

/* Creates a frame to be sent over the low-level transport layer from a command, a byte buffer and its length
 returns the number of sent bytes */
uint8_t 
evoMin_CreateFrame(struct evoMin_Frame* frame, uint8_t command, uint8_t* bytes, uint8_t bLength) {
	initialize_frame(frame);
	
	uint8_t bytesToSend = bLength;
	
	/* An invalid frame means it hasn't been send yet */
	frame->isValid = 0;
	frame->isSent = 0;
	frame->command = command;
	frame->timestamp = evoMin_GetTimeNow();
	
	/* If the payload length is larger than the maximum payload size,
	 only send EVOMIN_MAX_PAYLOAD_SIZE bytes */
	if (bLength > EVOMIN_MAX_PAYLOAD_SIZE) {
		bytesToSend = EVOMIN_MAX_PAYLOAD_SIZE;
	}
	
	/* Copy buffer */
	memcpy(frame->buffer.buffer, bytes, bytesToSend);
	frame->pLength = bytesToSend;
	
	/* Return the number of bytes sent after the package has been created */
	return bytesToSend;
}

ResultState_t
evoMin_QueueFrame(struct evoMin_Interface* interface, struct evoMin_Frame frame) {
	ResultState_t resultState = CreateResultState(type_NoError, src_evoMIN + src_evoMIN_QUEUEFRAME, prio_None);

	if(interface->queuePtrW + 1 > EVOMIN_MAX_FRAMES) {
		/* Cannot queue the frame as we're already full */
		return CreateResultState(type_OutOfBounds, src_evoMIN + src_evoMIN_QUEUEFRAME, prio_Low);
	}

	frame.timestamp = evoMin_GetTimeNow();

	interface->queue[interface->queuePtrW] = frame;
	interface->queuePtrW++;

	return resultState;
}

void
evoMin_SendResendLastFrame(struct evoMin_Interface* interface) {
	/* This function needs to be called regularly inside the application's main loop,
       as it tries to send (or resend) enqueued frames over the low-level sending routine */
	if(interface->queuePtrW == 0) {
		/* No elements in queue, cancel */
		return;
	}

	struct evoMin_Frame* frame = NULL;

	if(!interface->forcedFrame.isInitialized) {
		frame = queue_get_active_frame(interface);
	} else {
		frame = &interface->forcedFrame;
	}
	if(!(frame && frame->isInitialized)) {
		return;
	}

	if(frame->retriesLeft != EVOMIN_SEND_RETRIES_ON_FAIL
		&& (evoMin_GetTimeNow() - frame->timestamp) < EVOMIN_SEND_RETRY_MIN_TIME){
		/* Resend frame if a minimum time of EVOMIN_SEND_RETRY_MIN_TIME has passed since the last try.
		   However, this only happens after the first try, as the first try immediately sends the frame! */
		return;
	}

	/* Update timestamp and (re)send frame */
	frame->timestamp = evoMin_GetTimeNow();
	send_frame(interface, frame);

	if(!frame->isSent && frame->retriesLeft - 1 > 0) {
		frame->retriesLeft--;
	} else {
		/* No more retries or frame already sent, dequeue frame (discard) */
		if(!frame->isSent) {
			/* ERROR, frame couldn't be sent */
			CreateResultState(type_OutOfBounds, src_evoMIN + src_evoMIN_SENDFRAME_RETRY, prio_Low);
		}
		if(!interface->forcedFrame.isInitialized) {
			memset(&interface->queue[interface->queuePtrR], 0, sizeof(struct evoMin_Frame));
		} else {
			interface->forcedFrame = (struct evoMin_Frame) {};
		}

		if(interface->queuePtrR + 1 == interface->queuePtrW) {
			interface->queuePtrR = 0;
			interface->queuePtrW = 0;
		} else {
			interface->queuePtrR++;
		}
	}
}

uint8_t
evoMin_SendFrameImmediately(struct evoMin_Interface* interface, struct evoMin_Frame frame) {
	/* Force sending a frame immediately,
	   therefor we need to place it in a special buffer and circumvent the regular queue */
	if(interface->forcedFrame.isInitialized) {
		/* There's another forced frame already, that hasn't been sent yet, cancel */
		return 0;
	}
	interface->forcedFrame = frame;
	send_frame(interface, &interface->forcedFrame);

	while(!interface->forcedFrame.isSent) {
		evoMin_SendResendLastFrame(interface);
		/* Interrupt should be called meanwhile to set isSent,
		   otherwise the frame will get discarded after n retries */
	}
	return 1;
}


/* Private functions */
static void
initialize_frame(struct evoMin_Frame* frame) {
	frame->isValid = 0;
	frame->command = EVOMIN_CMD_RESERVED;
	frame->pLength = 0;
	frame->isSent = 0;
	frame->retriesLeft = EVOMIN_SEND_RETRIES_ON_FAIL;
	frame->isInitialized = buffer_initialize(&frame->buffer);
	frame->crc8 = 0;
}

#ifndef EVOMIN_TX_DISABLE
ResultState_t
send_frame(struct evoMin_Interface* interface, struct evoMin_Frame* frame) {
	/* Sends a previously created frame via the low-level implementation of the evoMin_Handler_TX callback */
	ResultState_t resultState = CreateResultState(type_NoError, src_evoMIN + src_evoMIN_SENDFRAME, prio_None);

	uint32_t bytesSent = EVOMIN_FRAME_SIZE;	 /* Start with frame size of bytes and increase for each additional byte */
	uint8_t nextByte = 0;
	uint8_t lastByte = 0;
	uint32_t pCnt = 0;
	uint32_t crcCnt = 0;
	uint8_t bytesToSend = frame->pLength;
	uint32_t crcBufSize = bytesToSend + 1 + 1;
	int8_t foundPlHeader = -1;
	uint8_t* crcBuffer = 0;

	/* Send SOF bytes */
	interface->evoMin_Handler_TX(EVOMIN_FRAME_SOF);
	interface->evoMin_Handler_TX(EVOMIN_FRAME_SOF);
	interface->evoMin_Handler_TX(EVOMIN_FRAME_SOF);

	/* A transport frame always has a payload length of EVOMIN_MAX_PAYLOAD_SIZE while initialization */

	/* Pre-initialize the crc buffer */
	/* 0	command
	 1	len
	 2..n payload data */
	if(!frame->isValid) {
		crcBuffer = calloc(crcBufSize, sizeof(uint8_t));
		if(!crcBuffer) {
			return CreateResultState(type_OutOfBounds, src_evoMIN + src_evoMIN_SENDFRAME_BUF_ALLOC, prio_High);
		}
		crcBuffer[0] = frame->command;
		crcBuffer[1] = bytesToSend;
		crcCnt = 1;
	}

	/* Send command byte */
	interface->evoMin_Handler_TX(frame->command);

	/* Send payload length */
	interface->evoMin_Handler_TX(bytesToSend);

	for (uint32_t cCnt = 0; cCnt < bytesToSend && pCnt < EVOMIN_MAX_PAYLOAD_SIZE ; cCnt++) {
		if (foundPlHeader == 1) {
			interface->evoMin_Handler_TX(EVOMIN_FRAME_STFBYT);
			bytesSent++;
			lastByte = EVOMIN_FRAME_STFBYT;

			foundPlHeader = -1;
		}
		/* For the transmission of a frame body if two 0xAA bytes in a row are transmitted a stuff byte with value 0x55
		 is inserted into the transmitted byte stream */
		if (frame->buffer.buffer[cCnt] == EVOMIN_FRAME_SOF && lastByte == EVOMIN_FRAME_SOF) {
			foundPlHeader = 1;
		}

		nextByte = frame->buffer.buffer[cCnt];

		if(!frame->isValid && crcBuffer) {
			crcBuffer[++crcCnt] = frame->buffer.buffer[cCnt];
		}

		lastByte = frame->buffer.buffer[cCnt];

		/* Send next payload byte */
		interface->evoMin_Handler_TX(nextByte);
		bytesSent++;
	}

	if(!frame->isValid && crcBuffer) {
		/* Calculate and send the crc8 for the complete frame (excluding the crc8 itself) */
		frame->crc8 = evoMin_CRC8(crcBuffer, crcBufSize);
		frame->isValid = 1;

		free(crcBuffer);
	}

	interface->evoMin_Handler_TX(frame->crc8);

	/* SEND EOF byte */
	interface->evoMin_Handler_TX(EVOMIN_FRAME_EOF);

	/* Set internal state to wait for the ACK, the reception of ACK happens in the RX handler */
	interface->state = EVOMIN_STATE_MSG_SENT_WAIT_FOR_ACK;

	return resultState;
}
#endif

static uint8_t
buffer_initialize(struct evoMin_Buffer* buffer) {
	buffer->headOffset = 0;
	buffer->tailOffset = 0;
	buffer->size = EVOMIN_P_BUF_SIZE + 2;
	buffer->status = EVOMIN_BUF_STATUS_MASK_INIT;
	return 1;
}

static uint8_t
buffer_push(struct evoMin_Buffer* buffer, uint8_t byte) {
	/* Push a byte onto the buffer */
	if (buffer->tailOffset + 1 >= buffer->size) {
		buffer->status |= EVOMIN_BUF_STATUS_MASK_NES;
		return 0;
	}

	buffer->buffer[buffer->tailOffset++] = byte;
	return 1;
}

static struct evoMin_Frame*
queue_get_active_frame(struct evoMin_Interface* interface) {
	return &interface->queue[interface->queuePtrR];
}