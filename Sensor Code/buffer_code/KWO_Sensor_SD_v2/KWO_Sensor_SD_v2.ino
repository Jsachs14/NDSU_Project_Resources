
//written by Jonah Sachs
//with reference from Joseph Kasti
//SD Card Reading for Medical Prototype

//NECCESSARY LIBRARY INTRODUCTIONS
#include <Adafruit_Si7021.h>
#include <Wire.h>
#include <LCD_I2C.h>
#include <RTClib.h>
#include <SPI.h>
#include <SD.h>

#define CH0 9
#define CH1 8
#define CH2 7
#define CH3 6
#define CH4 5
#define CH5 4


//SD CARD MODULE CONNECTIONS
#define SD_CS_PIN 10
#define SD_MOSI_PIN 11
#define SD_MISO_PIN 12
#define SD_SCK_PIN 13

float data_points[100];
int datacounter = 0;
int filecounter = 0;
unsigned long previousMillis = 0;


bool valueChanged = true;

//LCD Setup
LCD_I2C lcd(0x27,20,4);

//Microcontroller Setup
Adafruit_Si7021 sensor = Adafruit_Si7021();

//RTC setup
RTC_DS1307 rtc;


//variable defintion

byte ch_number;
bool enableHeater = false;
uint8_t loopCnt = 0;
uint32_t res;
const uint32_t res_table[6] = {1000,10000,100000,1000000,1500000,2000000};
char _buffer[11];


//methods
int states[6] = {1,1,1,1,1,1};


//helper method for channel select
//0 is LOW 1 is HIGH
void ch_select_helper(int spot){
  states[spot] = 0;
  digitalWrite(CH0,states[0]);
  digitalWrite(CH1,states[1]);
  digitalWrite(CH2,states[2]);
  digitalWrite(CH3,states[3]);
  digitalWrite(CH4,states[4]);
  digitalWrite(CH5,states[5]);
  states[spot] = 1;  
}

//CHANNEL SELECTION METHOD
void channel_select(byte n){
  switch(n) {
    case 0:
      ch_select_helper(0);
      break;
    case 1:
      ch_select_helper(1);
      break;
    case 2:
      ch_select_helper(2);
      break;
    case 3:
      ch_select_helper(3);
      break;
    case 4:
      ch_select_helper(4);
      break;
    case 5:
      ch_select_helper(5);
      break;
  }
  res = res_table[n];
}

void setup() {


 //LCD INITIALIZATION
 Serial.begin(115200);
 lcd.begin();
 lcd.backlight();
 lcd.setCursor(0,0);
 lcd.print("R (\364) =");
 lcd.setCursor(0,2);
 lcd.print("Temp (C) =");
 lcd.setCursor(0,3);
 lcd.print("Humidity% =");

 //RTC INITIALIZATION
 rtc.begin();
 rtc.adjust(DateTime(F(__DATE__),  F(__TIME__)));

 //PIN INTIALIZATION
 pinMode(CH0, OUTPUT);
 pinMode(CH1, OUTPUT);
 pinMode(CH2, OUTPUT);
 pinMode(CH3, OUTPUT);
 pinMode(CH4, OUTPUT);
 pinMode(CH5, OUTPUT);

 ch_number = 5;
 channel_select(ch_number);

 if(!sensor.begin()){
  Serial.println("Did not find Si7021 sensor!");
  while(true);  
 }

 Serial.print("Found model ");
 switch(sensor.getModel()){
  case SI_Engineering_Samples:
    Serial.print("SI engineering samples");
    break;
  case SI_7013:
    Serial.print("Si7013");
    break;
  case SI_7020:
    Serial.print("Si7020");
    break;
  case SI_7021:
    Serial.print("Si7021");
    break;
  case SI_UNKNOWN:
  default:
    Serial.print("Unknown");
 }

 Serial.print(" Rev(");
 Serial.print(sensor.getRevision());
 Serial.print(")");
 Serial.print(" Serial #");
 Serial.print(sensor.sernum_a,HEX);
 Serial.println(sensor.sernum_b,HEX);
 
}

void loop() {
  DateTime time = rtc.now();

  

//  Serial.print("Humidity:    ");
//  Serial.print(sensor.readHumidity(), 2);
//  Serial.print("\tTemperature: ");
//  Serial.println(sensor.readTemperature(),2);
    
//    Serial.println(_buffer);
//    Serial.println();
 
  //toggle heater every 30 seconds
//  if(++loopCnt == 30){
//    enableHeater = !enableHeater;
//    sensor.heater(enableHeater);
//    Serial.print("Heater Enabled State: ");
//    if(sensor.isHeaterEnabled()){
//      Serial.println("ENABLED");
//    }
//    else{
//      Serial.println("DISABLED");
//    }
//
//    loopCnt = 0;
//  }

  lcd.setCursor(11,2);
  lcd.print(sensor.readTemperature(), 2);
  lcd.setCursor(12,3);
  lcd.print(sensor.readHumidity(), 2);


  uint16_t volt_image = analogRead(A1) + 1;

  //CHANNEL INCREASE
  if(volt_image>= 550 && ch_number <5){
    ch_number++;
    channel_select(ch_number);
    delay(50);
    return;
  }

  //CHANNEL DECREASE
  if(volt_image<=90 && ch_number > 0){
    ch_number--;
    channel_select(ch_number);
    delay(50);
    return;
  }

  if(volt_image < 900){
    float value = (float)volt_image*res/(1023-volt_image);
    data_points[datacounter%100] = value;
    datacounter++;
    if(value<res_table[0]){
      sprintf(_buffer, "%03u.%1u Ohm", (uint16_t)value,(uint16_t)(value*10)%10);
    }
    else if(value<res_table[1]){
      sprintf(_buffer, "%1u.%03u kOhm", (uint16_t)(value/1000),(uint16_t)value%1000);    
    }
    else if(value<res_table[2]){
      sprintf(_buffer, "%02u.%02u kOhm", (uint16_t)(value/1000),(uint16_t)(value/10)%100);
    }
    else if(value<res_table[3]){
      sprintf(_buffer, "%03u.%1u kOhm", (uint16_t)(value/1000),(uint16_t)(value/100)%10);
    }
    else{
      sprintf(_buffer, "%1u.%03u MOhm", (uint16_t)(value/1000000),(uint16_t)(value/1000)%1000);
    }

  }


  


  uint16_t resistance = 0;

//  if(volt_image>900){
//    float value = (float)volt_image*res/(1023-volt_image);
//    uint16_t valueINT = 0;
//
//    if(value<res_table[0]){
//      resistance = (valueINT*10)%10;
//    }
//    else if(value<res_table[1]){
//      resistance = (valueINT/1000)%1000;
//    }
//    else if(value<res_table[2]){
//      resistance = ((valueINT/1000)/10)%100;
//    }
//    else if(value<res_table[3]){
//      resistance = ((valueINT/1000)/100)%10;
//    }
//    else{
//      resistance = ((valueINT/1000000)/1000)%1000;
//    }
//  }
//
// else{
//  sprintf(_buffer, "Over Load ");
// }
//
// 



 lcd.setCursor(8,0);
 lcd.print(_buffer);



 
 Serial.println(_buffer);
 unsigned long currentMillis = millis();
 if(currentMillis-previousMillis >= 10000){
    previousMillis = currentMillis;
    Serial.print("10 seconds passed: TEST ");
    Serial.println(filecounter);
    filecounter++;  
  }
 
 datacounter++;

 
  
}
