#define   LEDR   12
#define   LEDG   5
#define   LEDB   4

void setup() 
{
  pinMode(LEDR,OUTPUT);
pinMode(LEDG,OUTPUT);
pinMode(LEDB,OUTPUT);
}

void loop() 
{
  for(int a=0;a<=255;a++)
  {
    analogWrite(LEDR,a);
    delay(8);  
  }
  for(int a=255;a>=0;a--)
  {
    analogWrite(LEDR,a);
    delay(8);
  }
  
  for(int a=0;a<=255;a++)
  {
    analogWrite(LEDG,a);
    delay(8);  
  }
  for(int a=255;a>=0;a--)
  {
    analogWrite(LEDG,a);
    delay(8);
  }
  for(int a=0;a<=255;a++)
  {
    analogWrite(LEDB,a);
    delay(8);  
  }
  for(int a=255;a>=0;a--)
  {
    analogWrite(LEDB,a);
    delay(8);
  }
  
  for(int a=0;a<=255;a++)
  {
    analogWrite(LEDR,a);
    analogWrite(LEDG,a);
    delay(8);  
  }
  for(int a=255;a>=0;a--)
  {
    analogWrite(LEDR,a);
    analogWrite(LEDG,a);
    delay(8);
  }
  for(int a=0;a<=255;a++)
  {
    analogWrite(LEDR,a);
    analogWrite(LEDB,a);
    delay(8);  
  }
  for(int a=255;a>=0;a--)
  {
    analogWrite(LEDR,a);
    analogWrite(LEDB,a);
    delay(8);
  }

  
}
