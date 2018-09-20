#include <Wire.h>


//----define command set for easy use-----//

#define GET_BOARD_STATUS 0x01
#define SET_PCM_RESET 0x02
#define GET_VERSION_NUMBER 0x04
#define SET_WATCHDOG_TIMEOUT 0x06
#define RESET_SYSTEM_WATCHDOG 0x07
#define GET_NUMBER_SYSTEM_RESETS 0x09
#define SET_PDM_INITIAL_ON 0x0A
#define SET_PDM_INITIAL_OFF 0x0B
#define SET_ALL_PDM_ON 0x0C
#define SET_ALL_PDM_OFF 0x0D
#define GET_PDM_STATUS 0x0E
#define GET_PDM_INITIAL_STATE 0x0F
#define GET_BOARD_TELEM 0x10
#define GET_WATCHDOG_TIMEOUT 0x11
#define SET_PDM_ON 0x12
#define SET_PDM_OFF 0x13
#define GET_SOFT_RESET_NUM 0x14
#define GET_EXPECTED_PDM_STATE 0x16
#define GET_BOARD_TEMP 0x17
#define RESET_NODE 0x80

//-------------- PDM Pins ----------------//


int SW0=2;
int SW1=3;
int SW2=4;
int SW3=5;
int SW4=6;
int SW5=7;
int SW6=8;
int SW7=9;
int SW8=10;
int SW9=11;

//-------------- PCM Pins ----------------//

int BV = 14;
int V12 = 15;
int V5 = 16;
int V3_3 = 17;

//----- variables for board status -------//

int last_command=0;
int last_command_range=0;
int wdt=0;
int por=1;
int bor=0;

//---------------------------------------//


//------------ PDM Initial States -------//

int in0 = 0;
int in1 = 0;
int in2 = 0;
int in3 = 0;
int in4 = 0;
int in5 = 0;
int in6 = 0;
int in7 = 0;
int in8 = 0;
int in9 = 0;

//---------------------------------------//



int watchdog = 4;


int x=0;
int written=0;

int reset_freq=0;
int soft_reset_freq=0;

void setup() {
  Serial.begin(9600);
  pinMode(SW0, OUTPUT);
  pinMode(SW1, OUTPUT);
  pinMode(SW2, OUTPUT);
  pinMode(SW3, OUTPUT);
  pinMode(SW4, OUTPUT);
  pinMode(SW5, OUTPUT);
  pinMode(SW6, OUTPUT);
  pinMode(SW7, OUTPUT);
  pinMode(SW8, OUTPUT);
  pinMode(SW9, OUTPUT);

  pinMode(BV, OUTPUT);
  pinMode(V12, OUTPUT);
  pinMode(V5, OUTPUT);
  pinMode(V3_3, OUTPUT);

  digitalWrite(SW0, in0);
  digitalWrite(SW1, in1);
  digitalWrite(SW2, in2);
  digitalWrite(SW3, in3);
  digitalWrite(SW4, in4);
  digitalWrite(SW5, in5);
  digitalWrite(SW6, in6);
  digitalWrite(SW7, in7);
  digitalWrite(SW8, in8);
  digitalWrite(SW9, in9);


  Wire.begin(43);
  Wire.onReceive(writeEvent);
  Wire.onRequest(readEvent);
  
}

void writeEvent() {
  int command = Wire.read();
  int data = Wire.read();
  
  switch(command) {
    case GET_BOARD_STATUS: 
      get_board_status();
      break;
    case SET_PCM_RESET: 
      set_pcm_reset(data);
      break;
    case GET_VERSION_NUMBER:
      get_version_number();
      break;
    case SET_WATCHDOG_TIMEOUT: 
      set_system_watchdog(data);
      break;
    case RESET_SYSTEM_WATCHDOG: 
      reset_system_watchdog();
      break;
    case GET_NUMBER_SYSTEM_RESETS: 
      get_number_resets(data);
      break;
    case SET_PDM_INITIAL_ON:
      set_PDM_Initial_State_on(data);
      break; 
    case SET_PDM_INITIAL_OFF:
      set_PDM_Initial_State_off(data);
      break;
    case SET_ALL_PDM_ON :
      set_all_PDM_on();
      break;
    case SET_ALL_PDM_OFF :
      set_all_PDM_off();
      break;
    case GET_PDM_STATUS :
      get_PDM_status(data);
      break;
    case GET_PDM_INITIAL_STATE: 
      get_PDM_initial_state(data);
      break;
    case GET_BOARD_TELEM :
      get_board_telem(data);
      break;
    case GET_WATCHDOG_TIMEOUT: 
      get_watchdog_timeout();
      break;
    case SET_PDM_ON :
      set_PDM_on(data);
      break;
    case SET_PDM_OFF :
      set_PDM_off(data);
      break;
    case GET_SOFT_RESET_NUM: 
      get_num_soft_reset();
      break;
    case GET_EXPECTED_PDM_STATE: 
      get_all_pdm_initial_state();
      break;
    case GET_BOARD_TEMP :
      get_board_temp();
      break;

    case RESET_NODE :
      reset_node();
      break;
  }
}

void readEvent() {
  int command = Wire.read();
  switch(command){
    case GET_BOARD_STATUS: 
      get_board_status();
      break;
   
    case GET_VERSION_NUMBER:
      get_version_number();
      break;
    
    case RESET_SYSTEM_WATCHDOG: 
      reset_system_watchdog();
      break;
    
    
    case SET_ALL_PDM_ON :
      set_all_PDM_on();
      break;
    case SET_ALL_PDM_OFF :
      set_all_PDM_off();
      break;
   
    
    case GET_WATCHDOG_TIMEOUT: 
      get_watchdog_timeout();
      break;
    
    
    case GET_SOFT_RESET_NUM: 
      get_num_soft_reset();
      break;
    case GET_EXPECTED_PDM_STATE: 
      get_all_pdm_initial_state();
      break;
    case GET_BOARD_TEMP :
      get_board_temp();
      break;

    case RESET_NODE :
      reset_node();
      break;

  }
}


void get_board_status(){
  int board_status[32] = {last_command, last_command_range, 0, 0, 0, wdt, por, bor , 0,0,0,0,0,0,0,0 , 0,0,0,0,0,0,0,0 , 0,0,0,0,0,0,0,0};
  byte board_byte = 0;
  
  for(int i=31; i>=0; i--){
    board_byte |= (board_status[i] << i);
  }
 

  Serial.print(board_byte);
  
  Wire.write(board_byte);
  
}

void set_pcm_reset(int data){
  if(data == 0x01){
    digitalWrite(BV, HIGH);
  }
  else if(data == 0x02){
    digitalWrite(V5, HIGH);
  }
  else if(data == 0x03){
    digitalWrite(V5, HIGH);
    digitalWrite(BV, HIGH);

  }
  else if(data == 0x04){
    digitalWrite(V3_3, HIGH);
    
  }
  else if(data == 0x05){
    digitalWrite(V3_3, HIGH);
    digitalWrite(BV, HIGH);
    
  }
  else if(data == 0x06){
    digitalWrite(V3_3, HIGH);
    digitalWrite(V5, HIGH);
   
  }
  else if(data == 0x07){
    digitalWrite(V3_3, HIGH);
    digitalWrite(BV, HIGH);
    digitalWrite(V5, HIGH);
  }
  else if(data == 0x08){
    digitalWrite(V12, HIGH);
  }
  else if(data == 0x09){
    digitalWrite(BV, HIGH);
    digitalWrite(V12, HIGH);
  }
  else if(data == 0x0A){
    digitalWrite(V12, HIGH);
    digitalWrite(V5, HIGH);

  }
  else if(data == 0x0B){
    digitalWrite(V12, HIGH);
    digitalWrite(V5, HIGH);
    digitalWrite(BV, HIGH);
  }
  else if(data == 0x0C){
    digitalWrite(V12, HIGH);
    digitalWrite(V3_3, HIGH);
  }
  else if(data == 0x0D){
    digitalWrite(V12, HIGH);
    digitalWrite(V3_3, HIGH);
    digitalWrite(BV, HIGH);
  }
  else if(data == 0x0E){
    digitalWrite(V12, HIGH);
    digitalWrite(V5, HIGH);
    digitalWrite(V3_3, HIGH);
  }
  else if(data == 0x0F){
    digitalWrite(V12, HIGH);
    digitalWrite(BV, HIGH);
    digitalWrite(V3_3, HIGH);
    digitalWrite(V5, HIGH);
  }
  else{
    
  }
  delay(500);
  digitalWrite(V12, LOW);
  digitalWrite(BV, LOW);
  digitalWrite(V3_3, LOW);
  digitalWrite(V5, LOW);

}

void get_version_number(){
  Wire.write(0x01);
}


void set_system_watchdog(int data){
  watchdog = data;
}

void reset_system_watchdog(){
  //just kick it//
}

void get_number_resets(int data){
  Wire.write(reset_freq);
}

void get_PDM_status(int data){
  switch(data) {
    case 1:
      Wire.write(digitalRead(SW0));
      break;
    case 2:
      Wire.write(digitalRead(SW1));
      break;
    case 3:
      Wire.write(digitalRead(SW2));
      break;
    case 4:
      Wire.write(digitalRead(SW3));
      break;
    case 5:
      Wire.write(digitalRead(SW4));
      break;
    case 6:
      Wire.write(digitalRead(SW5));
      break;
    case 7:
      Wire.write(digitalRead(SW6));
      break;
    case 8:
      Wire.write(digitalRead(SW7));
      break;
    case 9:
      Wire.write(digitalRead(SW8));
      break;
    case 10:
      Wire.write(digitalRead(SW9));
      break;
  }
}


void set_PDM_Initial_State_on(int data){
  switch(data) {
    case 1:
      in0 = 1;
      break;
    case 2:
      in1 = 1;
      break;
    case 3:
      in2 = 1;
      break;
    case 4:
      in3 = 1;
      break;
    case 5:
      in4 = 1;
      break;
    case 6:
      in5 = 1;
      break;
    case 7:
      in6 = 1;
      break;
    case 8:
      in7 = 1;
      break;
    case 9:
      in8 = 1;
      break;
    case 10:
      in9 = 1;
      break;
    default:
      break;

  }
}
void set_PDM_Initial_State_off(int data){
  switch(data) {
    case 1:
      in0 = 0;
      break;
    case 2:
      in1 = 0;
      break;
    case 3:
      in2 = 0;
      break;
    case 4:
      in3 = 0;
      break;
    case 5:
      in4 = 0;
      break;
    case 6:
      in5 = 0;
      break;
    case 7:
      in6 = 0;
      break;
    case 8:
      in7 = 0;
      break;
    case 9:
      in8 = 0;
      break;
    case 10:
      in9 = 0;
      break;
    default:
      break;
  }
}
void set_all_PDM_on() {
  for (int x = 0; x<=9; x++){
    digitalWrite(x+2, HIGH);
  }
}
void set_all_PDM_off() {
  for (int x = 0; x<=9; x++){
    digitalWrite(x+2, LOW);
  }
}
void set_PDM_on(int data) {
  switch(data) {
    case 1:
      digitalWrite(SW0, HIGH);
      break;
    case 2:
      digitalWrite(SW1, HIGH);
      break;
    case 3:
      digitalWrite(SW2, HIGH);
      break;
    case 4:
      digitalWrite(SW3, HIGH);
      break;
    case 5:
      digitalWrite(SW4, HIGH);
      break;
    case 6:
      digitalWrite(SW5, HIGH);
      break;
    case 7:
      digitalWrite(SW6, HIGH);
      break;
    case 8:
      digitalWrite(SW7, HIGH);
      break;
    case 9:
      digitalWrite(SW8, HIGH);
      break;
    case 10:
      digitalWrite(SW9, HIGH);
      break;
    case 11:
    case 12:
    case 13:
    case 14:
    case 15:
      break;
  }
}
void set_PDM_off(int data){
  switch(data) {
    case 1:
      digitalWrite(SW0, LOW);
      break;
    case 2:
      digitalWrite(SW1, LOW);
      break;
    case 3:
      digitalWrite(SW2, LOW);
      break;
    case 4:
      digitalWrite(SW3, LOW);
      break;
    case 5:
      digitalWrite(SW4, LOW);
      break;
    case 6:
      digitalWrite(SW5, LOW);
      break;
    case 7:
      digitalWrite(SW6, LOW);
      break;
    case 8:
      digitalWrite(SW7, LOW);
      break;
    case 9:
      digitalWrite(SW8, LOW);
      break;
    case 10:
      digitalWrite(SW9, LOW);
      break;
    case 11:
    case 12:
    case 13:
    case 14:
    case 15:
      break;
  }
}

void get_PDM_initial_state(int data){
  switch(data) {
    case 1:
      Wire.write(in0);
      break;
    case 2:
      Wire.write(in1);
      break;
    case 3:
      Wire.write(in2);
      break;
    case 4:
      Wire.write(in3);
      break;
    case 5:
      Wire.write(in4);
      break;
    case 6:
      Wire.write(in5);
      break;
    case 7:
      Wire.write(in6);
      break;
    case 8:
      Wire.write(in7);
      break;
    case 9:
      Wire.write(in8);
      break;
    case 10:
      Wire.write(in9);
      break;
    default:
      break;
  }
}

void get_watchdog_timeout(){
  Wire.write(watchdog);
}

void get_num_soft_reset(){
  Wire.write(soft_reset_freq);
}

void get_all_pdm_initial_state(){
  int pdm_status[16] = {0, 0, 0, 0, 0, in9, in8, in7, in6, in5, in4, in3, in2, in1, in0};
  byte status_byte = 0;
  for(int i=0; i<16 ; i++){
    status_byte |= pdm_status[i] << i;
  }
  Wire.write(status_byte);
  
}

void get_board_telem(int data){
  
  
}

void get_board_temp(){
  
}

void reset_node(){
  
}



void loop() {
  // put your main code here, to run repeatedly:

}
