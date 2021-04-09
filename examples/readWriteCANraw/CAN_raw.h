// ------------------------------ CAN_raw.h -------------------------------
#ifndef RAW_CAN_INCLUDE
#define RAW_CAN_INCLUDE

void initRaw_CAN();
void readRawCANData();
bool packRawCANData(uint16_t offset, uint16_t len, byte bufferMem[]);

#endif
