#include <stdio.h>
#include "evomin.h"
#include "tx_impl.h"

#include "testdata.h"

struct evoMin_Interface comInterface;

int main()
{
	evoMin_Init(&comInterface);
	evoMin_SetTXHandler(&comInterface, &evoMin_comTXImplementation);

	/* Test the buffer
	 * 1. Fill different arrays and pass them to the RXHandler evoMin_Handler_ByteRecvd
	 * 2. Watch for data overflows (especially what happens to overflowing data */
	const uint32_t numberOfTests = 50;
	const uint32_t payloadLength = 25;

	for(uint32_t f = 0; f < numberOfTests; f++)
	{
		uint32_t bufSize = payloadLength + EVOMIN_FRAME_SIZE;

		evoMin_Handler_ByteRecvd(&comInterface, testData[f], bufSize);

		struct evoMin_Frame* frame = evoMin_getNextFrame(&comInterface);
		
		/* Overflow should occur on this point */
		//evoMin_Handler_ByteRecvd(&comInterface, testData1, EVOMIN_MAX_PAYLOAD_SIZE);

		printf("Buffer data for frame %d\n", f);
		for(uint8_t i = 0; i<frame->pLength; i++)
		{
			uint8_t v = evoMin_FrameGetDataByte(&comInterface, frame, i);
			printf("%d\n", v);
		}
	}
	return 0;
}