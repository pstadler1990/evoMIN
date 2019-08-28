#include "evomin.h"
#include "evoMIN_tx_impl.h"
#include <stdio.h>
#include <string.h>


uint8_t
evoMin_CRC8(uint8_t* bytes, uint32_t bLen) {
	uint8_t crc = 0x00;
	uint8_t extract;
	uint8_t sum;
	for(uint32_t i=0;i<bLen;i++)
	{
		extract = *bytes;
		for (char tempI = 8; tempI; tempI--)
		{
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
	printf("Send byte: %X\n", byte);
	return 0;
}

void
evoMin_Handler_FrameRecvd(struct evoMin_Frame* frame) {
	/* Received a valid evoMIN frame over SPI */
	char rBuf[EVOMIN_BUFFER_SIZE];
	for(uint32_t bCnt = 0; bCnt < frame->pLength && bCnt < EVOMIN_BUFFER_SIZE; bCnt++) {
		rBuf[bCnt] = evoMin_FrameGetDataByte(frame, bCnt);
	}

	switch(frame->command) {
		case EVOMIN_CMD_SET_CHANNEL:
			break;
		case EVOMIN_CMD_REQUEST_IDN:
			break;
		default:
			break;
	}
}