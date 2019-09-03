#include <stdio.h>
#include <stdint.h>
#include <unistd.h>
#include "evomin.h"
#include "evoMIN_impl.h"

struct evoMin_Interface comInterface;

static int8_t MIN_Test_ForceSendFrame(void);

int main() {
	evoMin_Init(&comInterface);
	evoMin_SetTXHandler(&comInterface, &evoMin_Handler_TX);

	MIN_Test_ForceSendFrame();

	return 0;
}


int8_t
MIN_Test_ForceSendFrame(void)  {
	struct evoMin_Frame sendFrame;

	/* Initialize payload */
	uint8_t buf[] = {
			0xDE,
			0xAD,
			0xBE,
			0xEF
	};

	/* Create and queue frame to be sent over the low-level interface */
	evoMin_CreateFrame(&sendFrame, EVOMIN_CMD_SEND_IDN, buf, 4);
	evoMin_QueueFrame(&comInterface, sendFrame);

	/* This function forces a frame to be sent immediately */
	evoMin_SendFrameImmediately(&comInterface, sendFrame);

	/* Otherwise, we could call the function below in our main loop */
	// evoMin_SendResendLastFrame(&comInterface);

	return 1;
}

