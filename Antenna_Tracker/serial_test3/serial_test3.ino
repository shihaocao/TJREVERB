//char dataString[50] = {0};
#include <Servo.h>

String data = "";
int a =0; 
int mid = 0;
Servo s1;

void setup() {
  Serial.begin(9600);              //Starting serial communication
  s1.attach(3);
}
  
void loop() {
  data = Serial.readString();
  mid = data.indexOf(" ");
  Serial.println(data.substring(0,mid));
  s1.write(data.substring(0,mid).toInt());
}
