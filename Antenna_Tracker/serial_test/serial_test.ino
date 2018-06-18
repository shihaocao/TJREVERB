//char dataString[50] = {0};
String dataString = "";
int a =0; 

void setup() {
Serial.begin(9600);              //Starting serial communication
}
  
void loop() {
  a++;  // a value increase every loop
  //sprintf(dataString,"%02X",a); // convert a value to hexa
  dataString = String("hello");
  Serial.println(dataString);   // send the data
  delay(1000);                  // give the loop some break
}
