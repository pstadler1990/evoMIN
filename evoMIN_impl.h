#ifndef EVOMIN_TX_IMPL_H
#define EVOMIN_TX_IMPL_H

uint8_t evoMin_CRC8(uint8_t* bytes, uint32_t bLen);
uint8_t evoMin_Handler_TX(uint8_t byte);
uint32_t evoMin_GetTimeNow(void);
uint8_t evoMin_Handler_FrameRecvd(struct evoMin_Frame *frame, uint8_t* answerBuffer, uint32_t answerBufferSize);
#ifdef IS_SYNCHRONOUS_MODE
void evoMin_RXTXHandler(struct evoMin_Interface* interface, uint8_t byteOut);
#endif
#endif //EVOMIN_TX_IMPL_H
