#include <Arduino.h>

void setup()
{
  delay(50);
  Serial.begin(115200);
  Serial.println(  );
  Serial.println(  );
  Serial.println(  );
  Serial.println("In setup");
}

int counter=0;

void loop()
{
  delay(1000);
  Serial.println("....");
  Serial.printf("   %4d\r\n",counter);
  counter=(++counter) % 100;
 }