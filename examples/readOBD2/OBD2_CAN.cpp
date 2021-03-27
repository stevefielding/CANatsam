#include <OBD2.h>
#include <DueTimer.h>
#include "OBD2_CAN.h"

/********************************************************************
This example is built upon the CANAcquisition class and the OBDParmameter class using 11bit (non-extended) OBD2 ID's

This example shows how to set up periodic data acquisition of OBD2 paramters based upon
standard PID's. If you'd like to add another paramter,simply copy one of the definitions and modify it accordingly. 
You may also need to add a new PID to the "OBD_PID" enum in the header file. 


********************************************************************/

//create the CANport acqisition schedulers
cAcquireCAN CANport1(CAN_PORT_1);

/***** DEFINITIONS FOR OBD MESSAGES ON CAN PORT 0, see https://en.wikipedia.org/wiki/OBD-II_PIDs to add your own ***************/
//char _name[10], char _units[10], OBD_PID pid,  uint8_t OBD_PID_SIZE size, bool _signed, OBD_MODE_REQ mode, float32 slope, float32 offset, cAcquireCAN *, extended ID;

  cOBDParameter OBD_Speed(      "Speed "        , " KPH"		,  SPEED       , _8BITS,   false,   CURRENT,  1,      0,  &CANport1, false);
  cOBDParameter OBD_EngineSpeed("Engine Speed " , " RPM"		,  ENGINE_RPM  , _16BITS,  false,   CURRENT,  0.25,   0,  &CANport1, false);
  cOBDParameter OBD_Throttle(   "Throttle "     , " %"  		,  THROTTLE_POS, _8BITS,   false,   CURRENT,  0.3922, 0,  &CANport1, false);
  cOBDParameter OBD_Coolant(    "Coolant "      , " C"  		,  COOLANT_TEMP, _8BITS,   false ,  CURRENT,  1,    -40,  &CANport1, false);
  cOBDParameter OBD_EngineLoad( "Load "         , " %"  		,  ENGINE_LOAD , _8BITS,   false,   CURRENT,  0.3922, 0,  &CANport1, false);
  cOBDParameter OBD_MAF(        "MAF "          , " grams/s",  ENGINE_MAF  , _16BITS,  false,   CURRENT,  0.01,   0,  &CANport1, false);
  cOBDParameter OBD_IAT(        "IAT "          , " C"  		,  ENGINE_IAT  , _8BITS,   false ,  CURRENT,  1,    -40,  &CANport1, false);

uint32_t carSpeed;
uint32_t engineSpeed;
uint32_t throttlePos;
uint32_t coolantTemp;
uint32_t engineLoad;
uint32_t maFlow;
uint32_t inletAirTemp;

uint32_t *OBD2_parameters[] = {&carSpeed, &engineSpeed, &throttlePos, &coolantTemp, &engineLoad, &maFlow, &inletAirTemp};
uint16_t OBD2_parameters_len = *(&OBD2_parameters + 1) - OBD2_parameters;

// this is our timer interrupt handler, called at 2mS interval
// This does not mean that the CAN qeuries are sent out at 2mS interval
// The actual qeuries only go out every 100mS, and I suspect that
// for a list of 7 parameters each parameter is only qeuried every 700mS
// Execution time appears to be around 12uS
// Checked the SPI interrupt and it appears to be interrupting the timer interrupt, which is good.
void CAN_RxTx()
{
  digitalWrite(39, HIGH);
  //run CAN acquisition scheduler on OBD CAN port
  CANport1.run(TIMER_2mS);
  digitalWrite(39, LOW);
}     	

void initOBD2_CAN()
{
  // Enable power supply associated with CAN1, and enable CAN1 transceiver
  pinMode(OBD_5V_EN, OUTPUT);
  digitalWrite(OBD_5V_EN, HIGH);
  pinMode(CANTX1_EN_N, OUTPUT);
  digitalWrite(CANTX1_EN_N, LOW);

  //start CAN ports,  enable interrupts and RX masks, set the baud rate here
#define AUTOBAUD _500K  
  CANport1.initialize(AUTOBAUD);
  NVIC_SetPriority(CAN1_IRQn, 15);
  NVIC_SetPriority(TC3_IRQn, 15);

  //set up the transmission/reception of messages to occur at 500Hz (2mS) timer interrupt
  Timer3.attachInterrupt(CAN_RxTx).setFrequency(500).start();
  
}

void readOBD2Data()
{
  //print out our latest OBDII data
  Serial.print(OBD_Speed.getName()); 
  carSpeed = OBD_Speed.getIntData();
  Serial.println(carSpeed);
	
  Serial.print(OBD_EngineSpeed.getName()); 
  engineSpeed = OBD_EngineSpeed.getIntData();
  Serial.println(engineSpeed);

  Serial.print(OBD_Throttle.getName()); 
  throttlePos = OBD_Throttle.getIntData();
  Serial.println(throttlePos);
    
  Serial.print(OBD_Coolant.getName()); 
  coolantTemp = OBD_Coolant.getIntData();
  Serial.println(coolantTemp);
	
  Serial.print(OBD_EngineLoad.getName()); 
  engineLoad = OBD_EngineLoad.getIntData();
  Serial.println(engineLoad); 

  Serial.print(OBD_MAF.getName()); 
  maFlow = OBD_MAF.getIntData();
  Serial.println(maFlow);

  Serial.print(OBD_IAT.getName()); 
  inletAirTemp = OBD_IAT.getIntData();
  Serial.println(inletAirTemp);         	
}

// pack 32-bit OBD2 data into a byte buffer
// Return error state
bool packOBD2Data(uint16_t offset, uint16_t len, byte bufferMem[]) {
uint16_t i;
uint16_t bInd;
uint32_t param;
  // Check if the read is within bounds of parameter buffer, and that we are reading from a 32-bit boundary
  if ((offset+len) > (OBD2_parameters_len * 4) || (len % 4 != 0) || (offset % 4 != 0))
    return true;

  // Pack the 32-bit values into the byte array in big endian format  
  for (i=0; i < len/4; i++) {
    bInd = i * 4;
    param = *OBD2_parameters[i+offset/4];
    bufferMem[bInd++] = (param >> 24) & 0xff;
    bufferMem[bInd++] = (param >> 16) & 0xff;
    bufferMem[bInd++] = (param >> 8) & 0xff;
    bufferMem[bInd++] = param & 0xff;
  }
  return false;
}
