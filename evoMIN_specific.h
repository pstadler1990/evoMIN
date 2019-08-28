#ifndef EVOMIN_SPECIFIC_H
#define EVOMIN_SPECIFIC_H

#define EVOMIN_MAX_PAYLOAD_SIZE			(uint32_t)50
#define EVOMIN_P_BUF_SIZE				(uint32_t)EVOMIN_MAX_PAYLOAD_SIZE
#define EVOMIN_BUFFER_SIZE				(uint32_t)EVOMIN_P_BUF_SIZE + 2
#define	EVOMIN_SEND_RETRIES_ON_FAIL		(uint32_t)3
#define EVOMIN_SEND_RETRY_MIN_TIME		(uint32_t)3	// TODO: Replace with suitable value (depending on communication interface speed etc.)

enum evoMin_Command {
	EVOMIN_CMD_RESERVED = 0x00,
	EVOMIN_CMD_SET_CHANNEL = 0xA0,
	EVOMIN_CMD_REQUEST_IDN = 0xB1,
	EVOMIN_CMD_SEND_IDN = 0xCD
};

#endif