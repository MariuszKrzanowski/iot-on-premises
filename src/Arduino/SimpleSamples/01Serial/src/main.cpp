#include <Arduino.h>

int counter = 0;

// Runs ones after processor boot
void setup()  
{
  delay(50);
  Serial.begin(115200);
  Serial.println();
  Serial.println(); 
  Serial.println("In setup");
}

// It called non stop time. 
// NOTE!!! Do not block this loop.
void loop()  
{
  delay(1000); // This is system friendly method so it is safe.
  Serial.println("....");
  Serial.printf("   %4d\r\n", counter);
  counter = (++counter) % 100;
}