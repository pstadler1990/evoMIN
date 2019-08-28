#include <stdio.h>
#include <stdint.h>
#include <unistd.h>
#include "evomin.h"
#include "evoMIN_tx_impl.h"
#include "testdata.h"

struct evoMin_Interface comInterface;

/* Test */
uint8_t globalRxBuffer[32];
uint32_t globalRxBufferIndex = 0;
static int8_t MIN_Test_BasicSendFrameAndReceive(void);


/*
    ###################################
 	Alles wichtige => siehe tx_impl.c !
 	###################################
*/
int main()
{
	evoMin_Init(&comInterface);
	evoMin_SetTXHandler(&comInterface, &evoMin_Handler_TX);

	MIN_Test_BasicSendFrameAndReceive();

	return 0;
}


int8_t
MIN_Test_BasicSendFrameAndReceive(void)  {
	struct evoMin_Frame sendFrame;
	struct evoMin_Frame sendFrame2;

	uint8_t idnBuf[] = {
			0xCA,
			0xDE
	};

	uint8_t buf2[] = {
			0xDE,
			0xAD,
			0xBE,
			0xEF
	};

	evoMin_CreateFrame(&sendFrame, EVOMIN_CMD_SEND_IDN, idnBuf, 2);
	evoMin_QueueFrame(&comInterface, sendFrame);

	evoMin_CreateFrame(&sendFrame2, EVOMIN_CMD_SET_CHANNEL, buf2, 4);
	evoMin_QueueFrame(&comInterface, sendFrame2);

	while(1) {
		evoMin_SendResendLastFrame(&comInterface);
		sleep(1);
	}

	return 1;
}

