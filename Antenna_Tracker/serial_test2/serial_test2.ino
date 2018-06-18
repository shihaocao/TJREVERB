//char dataString[50] = {0};
String dataString = "";
int a =0; 
int INPUT_SIZE = 60;

void setup() {
  Serial.begin(9600);              //Starting serial communication
}
  
void loop() {
  
  //Serial.println("I'm alive");
  // Get next command from Serial (add 1 for final 0)
  char input[INPUT_SIZE + 1];
  byte size = Serial.readBytes(input, INPUT_SIZE);
  //Serial.print(String(input));
  // Add the final 0 to end the C string
  Serial.print(input);
}
