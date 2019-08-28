#include <stdio.h>
#include <stdint.h>
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

	uint8_t idnBuf[] = {
			0xCA,
			0xDE
	};

	evoMin_InitializeFrame(&sendFrame);
	evoMin_CreateFrame(&sendFrame, EVOMIN_CMD_SEND_IDN, idnBuf, 2);
	evoMin_QueueFrame(&comInterface, &sendFrame);
	evoMin_QueueFrame(&comInterface, &sendFrame);
	evoMin_QueueFrame(&comInterface, &sendFrame);


	for(uint32_t i = 0; i < 32; i++) {
		evoMin_SendResendLastFrame(&comInterface);
	}

	evoMin_QueueFrame(&comInterface, &sendFrame);

	for(uint32_t i = 0; i < 32; i++) {
		evoMin_SendResendLastFrame(&comInterface);
	}

	evoMin_QueueFrame(&comInterface, &sendFrame);
	evoMin_QueueFrame(&comInterface, &sendFrame);
	evoMin_QueueFrame(&comInterface, &sendFrame);

	for(uint32_t i = 0; i < 32; i++) {
		evoMin_SendResendLastFrame(&comInterface);
	}


	return 1;
}

