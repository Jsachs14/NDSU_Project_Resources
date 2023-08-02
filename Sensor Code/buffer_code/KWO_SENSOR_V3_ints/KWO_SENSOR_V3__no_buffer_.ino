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
byte ch_number;
unsigned long previousMillis = 0;

//LCD Setup
LCD_I2C lcd(0x27,20,4);

//Microcontroller Setup
Adafruit_Si7021 sensor = Adafruit_Si7021();

//RTC setup
RTC_DS1307 rtc;

const uint32_t res_table[6] = {1000,10000,100000,1000000,1500000,2000000};
uint32_t res;
int states[6] = {1,1,1,1,1,1};
char ohm_levels[3] = {' ','K','M'};

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

 //CHANNEL SETUP
 ch_number = 5;
 channel_select(ch_number);

 //TEMP SENSOR VERIFICATION
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

  //TEMP and HUMIDITY on LCD
  lcd.setCursor(11,2);
  lcd.print(sensor.readTemperature(), 2);
  lcd.setCursor(12,3);
  lcd.print(sensor.readHumidity(), 2);


  //Initial voltage reading
  uint32_t volt_image = analogRead(A1) + 1;

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

  if(volt_image<900){
    float res_value = (float)((volt_image*res)/(1023-volt_image));
    char ohm_helper = ' ';
    int whole_tracker;
    float frac_tracker;
    


    //K / M / converter
    if(res_value<res_table[0]){
      whole_tracker = (int)res_value;
//      frac_tracker = (uint16_t)(res_value*10)%10;
//      frac_tracker = ((float)(res_value))/1000;
      ohm_helper = ohm_levels[0];
    }
    else if(res_value<res_table[1]){
      whole_tracker =(int)(res_value/1000);
//      frac_tracker = (uint16_t)(res_value)%1000;
//      frac_tracker = ((float)(res_value))/1000;
      ohm_helper = ohm_levels[1];
    }
    else if(res_value<res_table[2]){
      whole_tracker = (int)(res_value/1000);
//      frac_tracker = (uint16_t)(res_value/10)%100;
//      frac_tracker = ((float)(res_value))/1000;
      ohm_helper = ohm_levels[1];
    }
    else if(res_value<res_table[3]){
      whole_tracker = (int)(res_value/1000);
//      frac_tracker = (uint16_t)(res_value/100)%10);
//      frac_tracker = ((float)(res_value))/1000;
      ohm_helper = ohm_levels[1];
    }   
    else{
      whole_tracker = (int)(res_value/1000000);
//      frac_tracker = (uint16_t)(res_value/1000)%1000;
      ohm_helper = ohm_levels[2];
    }

    res_value = (float)whole_tracker;

    Serial.print(res_value);
    Serial.print(" ");
    Serial.print(ohm_helper);
    Serial.println("Ohms");

    unsigned long currentMillis = millis();
 if(currentMillis-previousMillis >= 10000){
    previousMillis = currentMillis;
    Serial.print("10 seconds passed: TEST ");
    Serial.println(filecounter);
    filecounter++;  
  }


    
    



    

    
    




    data_points[datacounter%100]=res_value;
    datacounter++;
    
  }

  
  

}
