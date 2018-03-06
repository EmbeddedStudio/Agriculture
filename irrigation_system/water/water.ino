int analogPin = A0;
int data = 0;
int led = 2;
int val = 0;

void setup()
{

  pinMode(led, OUTPUT); //定义 led 为输出引脚
  pinMode(analogPin, INPUT); //定义 led 为输出引脚
  Serial.begin(115200); //设定波特率为 115200
}
void loop()
{
  val = analogRead(analogPin); 
  if (val > 700) 
  { 
    digitalWrite(led, HIGH);
    
  }
  else 
  {
    digitalWrite(led, LOW);
  }
  data = val; 
  Serial.println(data); //串口打印变量 data
  delay(100);
}
