#include <stdio.h>
#include "evomin.h"
#include "tx_impl.h"

struct evoMin_Interface comInterface;

int main()
{
	evoMin_Init(&comInterface);
	evoMin_SetTXHandler(&comInterface, &evoMin_comTXImplementation);

	/* Test the buffer
	 * 1. Fill different arrays and pass them to the RXHandler evoMin_Handler_ByteRecvd
	 * 2. Watch for data overflows (especially what happens to overflowing data */
	uint8_t testData2[] = {
			0xAA,
			0xAA,
			0xAA,
			EVOMIN_CMD_CHIP,
			12,
			0xA0,
			0xB0,
			0xC0,
			0xD0,
			0xE0,
			0xF0,
			0xA1,
			0xB1,
			0xC1,
			0xD1,
			0xE1,
			0xF1,
			0xCC,
			0x55
	};

	const uint32_t bufSize = sizeof(testData2) / sizeof(uint8_t);

	for(uint32_t f = 0; f < 26; f++)
	{
		evoMin_Handler_ByteRecvd(&comInterface, testData2, bufSize);

		struct evoMin_Frame* frame = evoMin_getNextFrame(&comInterface);

		/* Overflow should occur on this point */
		//evoMin_Handler_ByteRecvd(&comInterface, testData1, EVOMIN_MAX_PAYLOAD_SIZE);

		printf("Buffer data for frame %d\n", f);
		for(uint8_t i = 0; i<frame->pLength; i++)
		{
			printf("%X\n", evoMin_FrameGetDataByte(&comInterface, frame, i));
		}
	}

	evoMin_sendFrame()



	return 0;
}