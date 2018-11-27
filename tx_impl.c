#include "evomin.h"
#include <stdio.h>

void 
evoMin_comTXImplementation(uint8_t byte)
{
	printf("\nSend byte: %d (%X)", byte, byte);
}


uint8_t
evoMin_CRC8(uint8_t* bytes, uint32_t bLen)
{


}


void
evoMin_Handler_FrameRecvd(struct evoMin_Frame* frame)
{
	printf("\nFrame received!\n");
	printf("Command: %d\n", frame->command);
	printf("Len: %d\n", frame->pLength);

	printf("\nFrame data: \n");
	for(uint32_t i = 0; i<frame->pLength; i++)
	{
		printf("%d (%X)\n", evoMin_FrameGetDataByte(frame, i),evoMin_FrameGetDataByte(frame, i));
	}
}