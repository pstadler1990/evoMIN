#include "evomin.h"
#include "evoMIN_impl.h"
#include <stdio.h>
#include <string.h>

uint8_t
evoMin_CRC8(uint8_t* bytes, uint32_t bLen)
{
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

uint32_t
evoMin_GetTimeNow(void) {
	/* Return an integer representation of a suitable time component,
	   the time is required for timestamp calculations for resending enqueued frames */
	// TODO: Replace with real system time (on embedded systems, this could be the Upcounter)
	time_t now = time(0);
	return (uint32_t) now;
}

uint8_t 
evoMin_Handler_Send(uint8_t byte) {
	// TODO: Fill TX buffer to be send via SPI
	(void) byte;
	return 0;
}


void
evoMin_Handler_FrameRecvd(struct evoMin_Frame* frame) {
	// TODO: Implement real FrameRecvd handler
	printf("\nFrame received!\n");
	printf("Command: %d\n", frame->command);
	printf("Len: %d\n", frame->pLength);
}