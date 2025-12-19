/* 
 * Project lasPotential
 * Author: Jamie Dowden-Duarte
 * Date: 11-12-2025
 * For comprehensive documentation and examples, please visit:
 * https://docs.particle.io/firmware/best-practices/firmware-template/
 */

#include "Particle.h"

///////////////
///Global//////
///////////////
const int LAS = D3;
const int PHOTO = D16;

int reading, prevRead;

SYSTEM_MODE(SEMI_AUTOMATIC);

void setup() {
  Serial.begin();
  waitFor(Serial.isConnected,10000);

  /////////
  //pinMode
  /////////
  pinMode(LAS,OUTPUT);
  pinMode(PHOTO,INPUT);
}


void loop() {
digitalWrite(LAS,HIGH);

reading = analogRead(PHOTO);
prevRead = 0;
if(reading != prevRead){
Serial.printf("reading %i",reading);

}
}
