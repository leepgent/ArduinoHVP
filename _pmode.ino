//Functions to enter / exit HV programming mode
#include "pins.hpp"

void start_pmode() {
  if (!pmode){
    start_boost_converter();

    pinMode(SDO, OUTPUT);
    digitalWrite(VCC, 1);
    delayMicroseconds(20);
    digitalWrite(RST, 0);
    delayMicroseconds(10);
    pinMode(SDO, INPUT);
    delayMicroseconds(200);
    //HVP mode entered.  Now command Flash Writing
    pmode = 1;

    digitalWrite(LED_PMODE, HIGH);
  }
}


void end_pmode() {
  if (pmode){
    digitalWrite(RST, 1);
    digitalWrite(VCC, 0);
    pinMode(SDO, OUTPUT);
    stop_boost_converter();
    pmode = 0;
    digitalWrite(LED_PMODE, LOW);
  }
}
