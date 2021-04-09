#include <OBD2.h>
#include <DueTimer.h>
#include "CAN_raw.h"

//create the CANport acqisition schedulers
cAcquireCAN CANport0(CAN_PORT_0);

/***** DEFINITIONS FOR RAW CAN FRAME *****/      
cCANFrame  RAW_CAN_Frame1;
cCANFrame  RAW_CAN_Frame2;

// this is our timer interrupt handler, called at 2mS interval
// This does not mean that the CAN qeuries are sent out at 2mS interval
// The actual qeuries only go out every 100mS, and I suspect that
// for a list of 7 parameters each parameter is only qeuried every 700mS
// Execution time appears to be around 12uS
// Checked the SPI interrupt and it appears to be interrupting the timer interrupt, which is good.
uint32_t txCnt = 0x0;
void CAN_RxTx()
{
  digitalWrite(39, HIGH);
  //run CAN acquisition scheduler on OBD CAN port
  CANport0.run(TIMER_2mS);
  RAW_CAN_Frame1.setLowerU32(txCnt++); // frame[4:7]
  digitalWrite(39, LOW);
}     	

void initRaw_CAN()
{
  // Enable power supply associated with CAN0, and enable CAN0 transceiver
  pinMode(OBD_5V_EN, OUTPUT);
  digitalWrite(OBD_5V_EN, HIGH);
  pinMode(CANTX0_EN_N, OUTPUT);
  digitalWrite(CANTX0_EN_N, LOW);

  //start CAN port0,  enable interrupts and RX masks, set the baud rate here
  CANport0.initialize(_500K);
  NVIC_SetPriority(CAN0_IRQn, 15);
  NVIC_SetPriority(TC3_IRQn, 15);

  //initialize the items needed to TX/RX raw CAN mesasges
  RAW_CAN_Frame1.ID    = 0x101;
  RAW_CAN_Frame1.rate  = _1Hz_Rate;
  RAW_CAN_Frame1.setUpperU32(0xdeafface); // frame[0:3]
  RAW_CAN_Frame1.setLowerU32(0x00000000); // frame[4:7]
  RAW_CAN_Frame2.ID    = 0x100; //expected ID from Pod
  RAW_CAN_Frame2.rate  = _1Hz_Rate;

  //add our raw messages to the scheduler
  CANport0.addMessage(&RAW_CAN_Frame1, TRANSMIT);
  CANport0.addMessage(&RAW_CAN_Frame2, RECEIVE);

  //set up the transmission/reception of messages to occur at 500Hz (2mS) timer interrupt
  Timer3.attachInterrupt(CAN_RxTx).setFrequency(500).start();
  
}

#define RAW_CAN_DATA_LEN 4
uint32_t rawCANData [RAW_CAN_DATA_LEN];

void readRawCANData()
{
  rawCANData[0] = CANport0.getRxCtr();
  rawCANData[1] = CANport0.getTxCtr();
  rawCANData[2] = RAW_CAN_Frame2.getLowerU32();
  rawCANData[3] = RAW_CAN_Frame2.getUpperU32();
}

// pack 32-bit OBD2 data into a byte buffer
// Return error state
bool packRawCANData(uint16_t offset, uint16_t len, byte bufferMem[]) {
uint16_t i;
uint16_t bInd;
uint32_t param;
  // Check if the read is within bounds of parameter buffer, and that we are reading from a 32-bit boundary
  if ((offset+len) > (RAW_CAN_DATA_LEN * 4) || (len % 4 != 0) || (offset % 4 != 0))
    return true;

  // Pack the 32-bit values into the byte array in big endian format  
  for (i=0; i < len/4; i++) {
    bInd = i * 4;
    param = rawCANData[i+offset/4];
    bufferMem[bInd++] = (param >> 24) & 0xff;
    bufferMem[bInd++] = (param >> 16) & 0xff;
    bufferMem[bInd++] = (param >> 8) & 0xff;
    bufferMem[bInd++] = param & 0xff;
  }
  return false;
}
