#ifndef EVOMIN_IMPL_H
#define EVOMIN_IMPL_H

uint8_t evoMin_CRC8(uint8_t* bytes, uint32_t bLen);
uint32_t evoMin_GetTimeNow(void);
uint8_t evoMin_Handler_Send(uint8_t byte);
void evoMin_Handler_FrameRecvd(struct evoMin_Frame* frame);

#endif //EVOMIN_IMPL_H
