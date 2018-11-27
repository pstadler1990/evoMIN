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
	const uint32_t numberOfTests = 16;
	const uint32_t payloadLength = 4;

	for(uint32_t f = 0; f < numberOfTests; f++)
	{
		uint32_t bufSize = payloadLength + EVOMIN_FRAME_SIZE;

		for(uint32_t i=0; i < bufSize; i++)
		{
			evoMin_RXHandler(&comInterface, testData[f][i]);
		}
	}

	return 0;
}