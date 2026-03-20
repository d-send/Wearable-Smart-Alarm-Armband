#include <Wire.h>


#define R_TOP 47000.0
#define R_BOTTOM 4700.0
#define ADC_REF 3.3
#define ADC_MAX 4095.0
#define VC_Sense 4
#define VBat_Sense 3
#define LED_Builtin 8

#define D_PWM 20
const int VC_Set = 15;
bool Dpwm_Enabled = true;

#define TZ_BTN 0
#define SONG_BTN 1

#define TZ 21

#define Song_PWM 10

#define SCL_PIN 6
#define SDA_PIN 7

#define EEPROM_ADDR 0x50   // 24C16 base address



void eepromWrite(uint16_t addr, uint8_t data)
{
  Wire.beginTransmission(EEPROM_ADDR | ((addr >> 8) & 0x07));
  Wire.write(addr & 0xFF);
  Wire.write(data);
  Wire.endTransmission();
  delay(5);
}

uint8_t eepromRead(uint16_t addr)
{
  Wire.beginTransmission(EEPROM_ADDR | ((addr >> 8) & 0x07));
  Wire.write(addr & 0xFF);
  Wire.endTransmission();

  Wire.requestFrom(EEPROM_ADDR | ((addr >> 8) & 0x07), 1);
  return Wire.read();
}

float readBatteryVoltage(uint8_t ADCpin,float R_top,float R_bottom)
{
  int adc = analogRead(ADCpin);
  float v = (adc / ADC_MAX) * ADC_REF;
  return v * ((R_top + R_bottom) / R_bottom);
}

volatile bool triggerPulse = false;
volatile uint32_t pulseWidth_us = 200;   // Change pulse width here

void IRAM_ATTR gpioInterrupt()
{
  // Interrupt fires on falling edge
  triggerPulse = true;
}

void setup() {
  // put your setup code here, to run once:
  Serial.begin(115200);
  
  pinMode(SONG_BTN,INPUT);
  pinMode(VC_Sense,INPUT);
  pinMode(TZ,OUTPUT);
  pinMode(LED_Builtin,OUTPUT);
  pinMode(Song_PWM,OUTPUT);
  pinMode(D_PWM,OUTPUT);
  ledcAttach(D_PWM,10000,8);
  pinMode(TZ_BTN,INPUT);
  attachInterrupt(digitalPinToInterrupt(TZ_BTN),
                  gpioInterrupt,
                  FALLING);

  digitalWrite(TZ,LOW);
  digitalWrite(LED_Builtin,HIGH);
  digitalWrite(Song_PWM,LOW);

  Wire.begin(SDA_PIN, SCL_PIN);

  ledcWrite(D_PWM, 128);
}

void loop() {
  // put your main code here, to run repeatedly:

  uint8_t v = eepromRead(0);
  uint8_t u = eepromRead(56);
  Serial.print(v);
  Serial.print("\t");
  Serial.print(u);
  Serial.print("\t");

  float vc = readBatteryVoltage(VC_Sense,47000.0,4700.0);

  if(vc > 18.0)
  {
    digitalWrite(LED_Builtin,LOW);
    ledcDetach(D_PWM);
    digitalWrite(D_PWM,LOW);
    Dpwm_Enabled = false;
  }
  else
  {
    if((vc < 8.0) && (Dpwm_Enabled == false))
    {
      Dpwm_Enabled = true;
      ledcAttach(D_PWM,10000,8);
      ledcWrite(D_PWM, 128);
      digitalWrite(LED_Builtin,HIGH);

    }
  }

  float vbat = readBatteryVoltage(VBat_Sense,390000.0,300000.0);
  vbat = (5*vbat/5.78);

  Serial.print(vc);
  Serial.print("\t");
  Serial.print(vbat);
  Serial.print("\t");

  if(digitalRead(SONG_BTN) == 0)
  { 
    digitalWrite(Song_PWM,HIGH);
  }
  else
  {
    digitalWrite(Song_PWM,LOW);
  }

  if (triggerPulse) 
  {
    triggerPulse = false;

    digitalWrite(TZ, HIGH);
    delayMicroseconds(pulseWidth_us);
    digitalWrite(TZ, LOW);
  }


  Serial.print(digitalRead(TZ_BTN));
  Serial.print("\t");

  
  Serial.print(digitalRead(SONG_BTN));
  Serial.println("\t");


}
