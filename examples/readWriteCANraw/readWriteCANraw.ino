#include "CAN_raw.h"

void setup()
{
  Serial.begin(115200);
  while(!Serial);
  Serial.println("Waiting for 3s");
  pinMode(LED_BUILTIN, OUTPUT);
  pinMode(39, OUTPUT); // PA2, R67 on Rev1.0
  pinMode(38, OUTPUT); // PA3, R68 on Rev1.0
  pinMode(CAN_5V_EN, OUTPUT);
  digitalWrite(CAN_5V_EN, HIGH);  // Not required for OBD, but cannot access Pods RS485 without this voltage
  Serial.println("Starting setup");        
  initRaw_CAN();
}

uint32_t loopCnt = 0;
bool ledOn;
#define RAW_DATA_LEN 16
byte rawByteData [RAW_DATA_LEN];

void loop()
{

  // flash an led
  loopCnt++;
  if ((loopCnt % 100000) == 0) {
    if (ledOn == false) {
      ledOn = true; 
      //digitalWrite(LED_BUILTIN, HIGH);
    }
    else {
      ledOn = false; 
      //digitalWrite(LED_BUILTIN, LOW);
    }
  }  
  delay(1000);

  // service the read and write requests generated in the spi isr
  readRawCANData();
  packRawCANData(0, RAW_DATA_LEN, rawByteData);
  for (int i=0; i<RAW_DATA_LEN; i++) {
    Serial.print("0x");  
    Serial.print(rawByteData[i]);
    Serial.print(" ");
  }
  Serial.println("");
  
}
