#include <ESP8266HTTPUpdateServer.h>

void setup()
{
  // put your setup code here, to run once:
  pinMode(2, OUTPUT);
}

void loop()
{
  // put your main code here, to run repeatedly:
  digitalWrite(2, 0);
  delay(1000);
  
  digitalWrite(2, 1);
  delay(1000);
  
}

