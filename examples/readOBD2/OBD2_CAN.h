// ------------------------------ OBD2_CAN.h -------------------------------
#ifndef OBD2_CAN_INCLUDE
#define OBD2_CAN_INCLUDE

void initOBD2_CAN();
void readOBD2Data();
bool packOBD2Data(uint16_t offset, uint16_t len, byte bufferMem[]);

#endif
