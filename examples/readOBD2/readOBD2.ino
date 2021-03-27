#include "OBD2_CAN.h"

void setup()
{
  Serial.begin(115200);
  while(!Serial);
  Serial.println("Waiting for 3s");
  pinMode(LED_BUILTIN, OUTPUT);
  pinMode(CAN_5V_EN, OUTPUT);
  digitalWrite(CAN_5V_EN, HIGH);  // Not required for OBD, but cannot access Pods RS485 without this voltage
  Serial.println("Starting setup");        
  initOBD2_CAN();
}

uint32_t loopCnt = 0;
bool ledOn;

void loop()
{

  // flash an led
  loopCnt++;
  if ((loopCnt % 100000) == 0) {
    if (ledOn == false) {
      ledOn = true; 
      digitalWrite(LED_BUILTIN, HIGH);
    }
    else {
      ledOn = false; 
      digitalWrite(LED_BUILTIN, LOW);
    }
  }  
  delay(1000);

  // service the read and write requests generated in the spi isr
  readOBD2Data();
  
}
