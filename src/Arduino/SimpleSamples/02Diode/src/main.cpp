#include <Arduino.h>

#define MY_LED_PIN 2
#define baseTime 300

String text = String("... --- ... .-- --. -. . -"); // SOS WG NET in Morse Code
int position;

void setup()
{
  delay(10);
  pinMode(LED_BUILTIN, OUTPUT); // Initialize the LED_BUILTIN pin as an output

  delay(50);
  Serial.begin(115200);
  Serial.println();
  Serial.println();
  Serial.println();
  Serial.println("In setup");
  position = text.length();
}

// the loop function runs over and over again forever
void loop()
{
  if (position < text.length())
  {
    position++;
  }
  else
  {
    Serial.println();
    position = 0;
  }

  char c = text.charAt(position);

  if ('.' == c)
  {
    Serial.print('.');
    digitalWrite(LED_BUILTIN, LOW); // Turn the LED on (Note that LOW is the voltage level)
    delay(baseTime);
    Serial.print(' ');
  }

  if ('-' == c)
  {
    Serial.print('_');
    digitalWrite(LED_BUILTIN, LOW); // Turn the LED on (Note that LOW is the voltage level)
    delay(2*baseTime);
    Serial.print(' ');
  }

  if(' '==c)
  {
    Serial.print(' ');
  }

  digitalWrite(LED_BUILTIN, HIGH); // Turn the LED off by making the voltage HIGH
  delay(baseTime);
}