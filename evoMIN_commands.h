#ifndef EVOMIN_COMMANDS_H
#define EVOMIN_COMMANDS_H

enum evoMin_Command {
	EVOMIN_CMD_RESERVED = 0x00,
	EVOMIN_CMD_SET_CHANNEL = 0xA0,
	EVOMIN_CMD_REQUEST_IDN = 0xB1,
	EVOMIN_CMD_SEND_IDN = 0xCD
};

#endif