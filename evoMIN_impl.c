#include "evomin.h"
#include "evoMIN_impl.h"
#include <stdio.h>
#include <sys/types.h>
#include <time.h>

uint8_t
evoMin_CRC8(uint8_t* bytes, uint32_t bLen) {
	uint8_t crc = 0x00;
	uint8_t extract;
	uint8_t sum;
	for(uint32_t i=0;i<bLen;i++) {
		extract = *bytes;
		for (char tempI = 8; tempI; tempI--) {
			sum = (crc ^ extract) & 0x01;
			crc >>= 1;
			if (sum)
				crc ^= 0x8C;
			extract >>= 1;
		}
		bytes++;
	}
	return crc;
}

uint8_t
evoMin_Handler_TX(uint8_t byte) {
	printf("Send TX byte: \t%X\n", byte);
	return 0;
}

uint32_t
evoMin_GetTimeNow(void) {
	time_t now = time(0);
	return (uint32_t) now;
}

uint8_t
evoMin_Handler_FrameRecvd(struct evoMin_Frame *frame, uint8_t* answerBuffer, uint32_t answerBufferSize) {
	/* Received a valid evoMIN frame over SPI */
	printf("Frame received!\n");

	char rBuf[EVOMIN_BUFFER_SIZE];
	for(uint8_t bCnt = 0; bCnt < frame->pLength && bCnt < EVOMIN_BUFFER_SIZE; bCnt++) {
		rBuf[bCnt] = evoMin_FrameGetDataByte(frame, bCnt);
	}
	/* Received bytes are in rBuf */

	// Prepare answer (if any)
	if(answerBufferSize >= 4) {
		answerBuffer[0] = 0xDE;
		answerBuffer[1] = 0xAD;
		answerBuffer[2] = 0xBE;
		answerBuffer[3] = 0xEF;

		return 4;
	} else {
		return 0;
	}
}