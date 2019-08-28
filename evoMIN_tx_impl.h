#ifndef EVOMIN_TX_IMPL_H
#define EVOMIN_TX_IMPL_H

uint8_t evoMin_CRC8(uint8_t* bytes, uint32_t bLen);
uint8_t evoMin_Handler_TX(uint8_t byte);
uint32_t evoMin_GetTimeNow(void);
void evoMin_Handler_FrameRecvd(struct evoMin_Frame* frame);

#endif //EVOMIN_TX_IMPL_H
