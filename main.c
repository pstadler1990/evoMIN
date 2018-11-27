#include <stdio.h>
#include "evomin.h"
#include "tx_impl.h"

#include "testdata.h"

struct evoMin_Interface comInterface;

int main()
{
	evoMin_Init(&comInterface);
	evoMin_SetTXHandler(&comInterface, &evoMin_comTXImplementation);

	/* Test receive buffer and frame callback */
	const uint32_t numberOfTests = 16;
	const uint32_t payloadLength = 26;

	/*for(uint32_t f = 0; f < numberOfTests; f++)
	{
		uint32_t bufSize = payloadLength + EVOMIN_FRAME_SIZE;

		for(uint32_t i=0; i < bufSize; i++)
		{
			evoMin_RXHandler(&comInterface, testData[f][i]);
		}
	}
	*/
	/* Test sending of a frame */
	const uint32_t sendBufferLen = 9;
	uint8_t sendBuffer[] = {
			0xAA,
			0xAA,
			0xAA,
			0xBB,
			0xAA,
			0xAA,
			0xAA,
			0xCC,
			0xDD
	};

	evoMin_sendFrame(&comInterface, EVOMIN_CMD_CHIP, sendBuffer, sendBufferLen);

	uint32_t testBuffer2Len = 18;
	uint8_t testBuffer2[] = {
			 0xAA,
			 0xAA,
			 0xAA,
			 0xF0,
			 9,
			 0xAA,
			 0xAA,
			0x55,
			 0xAA,
			 0xBB,
			 0xAA,
			 0xAA,
			0x55,
			 0xAA,
			 0xCC,
			 0xDD,
			 0x8D,
			0x55
	};
	uint32_t bufSize = testBuffer2Len + EVOMIN_FRAME_SIZE;

	for(uint32_t i=0; i < bufSize; i++)
	{
		evoMin_RXHandler(&comInterface, testBuffer2[i]);
	}


	return 0;
}