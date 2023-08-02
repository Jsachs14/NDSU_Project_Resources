/***********************************************************************
 * 
 * Arduino autoranging ohmmeter with 16x2 LCD.
 * This is a free software with NO WARRANTY.
 * https://simple-circuit.com/
 *
 ***********************************************************************/
#include <Adafruit_Si7021.h>
#include <Wire.h> 
#include <LCD_I2C.h>
 
#define CH0  9
#define CH1  8
#define CH2  7
#define CH3  6
#define CH4  5
#define CH5  4

LCD_I2C lcd(0x27,20,4);  // set the LCD address to 0x27 for a 16 chars and 2 line display

Adafruit_Si7021 sensor = Adafruit_Si7021();

bool enableHeater = false;
uint8_t loopCnt = 0;
 
// variables
byte ch_number;
uint32_t res;
const uint32_t res_table[6] = {1000, 10000, 100000, 1000000,1500000,2000000};
char _buffer[11];

void setup(void) {
  Serial.begin(115200);
  lcd.begin();                      // initialize the lcd 
  lcd.backlight();
  lcd.setCursor(0, 0);  // move cursor to column 0, row 0 [position (0, 0)]
  lcd.print("R (\364) ="); //after 7
  lcd.setCursor(0, 2);  // move cursor to column 0, row 0 [position (0, 0)]
  lcd.print("Temp (C) ="); //after 10
  lcd.setCursor(0, 3);  // move cursor to column 0, row 0 [position (0, 0)]
  lcd.print("Humidity% ="); //after 11

 
  pinMode(CH0, OUTPUT);
  pinMode(CH1, OUTPUT);
  pinMode(CH2, OUTPUT);
  pinMode(CH3, OUTPUT);
  pinMode(CH4, OUTPUT);
  pinMode(CH5, OUTPUT);
 
  ch_number = 5;
  ch_select(ch_number);

    if (!sensor.begin()) {
    Serial.println("Did not find Si7021 sensor!");
    while (true)
      ;
  }

  Serial.print("Found model ");
  switch(sensor.getModel()) {
    case SI_Engineering_Samples:
      Serial.print("SI engineering samples"); break;
    case SI_7013:
      Serial.print("Si7013"); break;
    case SI_7020:
      Serial.print("Si7020"); break;
    case SI_7021:
      Serial.print("Si7021"); break;
    case SI_UNKNOWN:
    default:
      Serial.print("Unknown");
  }
  Serial.print(" Rev(");
  Serial.print(sensor.getRevision());
  Serial.print(")");
  Serial.print(" Serial #"); Serial.print(sensor.sernum_a, HEX); Serial.println(sensor.sernum_b, HEX);
}
 
// main loop
void loop() {

  Serial.print("Humidity:    ");
  Serial.print(sensor.readHumidity(), 2);
  Serial.print("\tTemperature: ");
  Serial.println(sensor.readTemperature(), 2);
  delay(1000);

  // Toggle heater enabled state every 30 seconds
  // An ~1.8 degC temperature increase can be noted when heater is enabled
  if (++loopCnt == 30) {
    enableHeater = !enableHeater;
    sensor.heater(enableHeater);
    Serial.print("Heater Enabled State: ");
    if (sensor.isHeaterEnabled())
      Serial.println("ENABLED");
    else
      Serial.println("DISABLED");
       
    loopCnt = 0;
  }
  lcd.setCursor(11, 2);  // move cursor to position (0, 1)
  lcd.print(sensor.readTemperature(), 2);
  lcd.setCursor(12, 3);  // move cursor to position (0, 1)
  lcd.print(sensor.readHumidity(), 2);
    
  uint16_t volt_image = analogRead(A1) + 1;
 
  if(volt_image >= 550 && ch_number < 5) {
    ch_number++;
    ch_select(ch_number);
    delay(50);
    return;
  }
 
  if(volt_image <= 90 && ch_number > 0) {
    ch_number--;
    ch_select(ch_number);
    delay(50);
    return;
  }
 
  if(volt_image < 900) {
    float value = (float)volt_image*res/(1023 - volt_image);
    if(value < 1000.0)
      sprintf(_buffer, "%03u.%1u Ohm ", (uint16_t)value, (uint16_t)(value*10)%10);
    else if(value < 10000.0)
           sprintf(_buffer, "%1u.%03u kOhm", (uint16_t)(value/1000), (uint16_t)value%1000);
    else if(value < 100000.0)
           sprintf(_buffer, "%02u.%02u kOhm", (uint16_t)(value/1000), (uint16_t)(value/10)%100);
    else if(value < 1000000.0)
           sprintf(_buffer, "%03u.%1u kOhm", (uint16_t)(value/1000), (uint16_t)(value/100)%10);
    else
      sprintf(_buffer, "%1u.%03u MOhm", (uint16_t)(value/1000000), (uint16_t)(value/1000)%1000);
  }
 
  else
    sprintf(_buffer, "Over Load ");
 
  lcd.setCursor(8,0);  // move cursor to position (0, 1)
  lcd.print(_buffer);
  Serial.println(_buffer);
  Serial.println();
 
  delay(500);   // wait some time
}
 
void ch_select(byte n) {
  switch(n) {
    case 0:
      digitalWrite(CH0, LOW);
      digitalWrite(CH1, HIGH);
      digitalWrite(CH2, HIGH);
      digitalWrite(CH3, HIGH);
      digitalWrite(CH4, HIGH);
      digitalWrite(CH5, HIGH);
      break;
    case 1:
      digitalWrite(CH0, HIGH);
      digitalWrite(CH1, LOW);
      digitalWrite(CH2, HIGH);
      digitalWrite(CH3, HIGH);
      digitalWrite(CH4, HIGH);
      digitalWrite(CH5, HIGH);
      break;
    case 2:
      digitalWrite(CH0, HIGH);
      digitalWrite(CH1, HIGH);
      digitalWrite(CH2, LOW);
      digitalWrite(CH3, HIGH);
      digitalWrite(CH4, HIGH);
      digitalWrite(CH5, HIGH);
      break;
    case 3:
      digitalWrite(CH0, HIGH);
      digitalWrite(CH1, HIGH);
      digitalWrite(CH2, HIGH);
      digitalWrite(CH3, LOW);
      digitalWrite(CH4, HIGH);
      digitalWrite(CH5, HIGH);
      break;
    case 4:
      digitalWrite(CH0, HIGH);
      digitalWrite(CH1, HIGH);
      digitalWrite(CH2, HIGH);
      digitalWrite(CH3, HIGH);
      digitalWrite(CH4, LOW);
      digitalWrite(CH5, HIGH);

    case 5:
      digitalWrite(CH0, HIGH);
      digitalWrite(CH1, HIGH);
      digitalWrite(CH2, HIGH);
      digitalWrite(CH3, HIGH);
      digitalWrite(CH4, HIGH);
      digitalWrite(CH5, LOW);
  }
  res = res_table[n];
}
