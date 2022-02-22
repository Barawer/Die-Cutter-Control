#include <Wire.h>
#include <Stepper.h>
#include <SparkFun_APDS9960.h>
#include <LiquidCrystal.h>
#define TCAADDR 0x70

void tcaselect(uint8_t i) {
  if (i > 7) return;
 
  Wire.beginTransmission(TCAADDR);
  Wire.write(1 << i);
  Wire.endTransmission();  
}

SparkFun_APDS9960 apds_0=SparkFun_APDS9960(); //Set 2 sensors
SparkFun_APDS9960 apds_1=SparkFun_APDS9960();
uint8_t proximity_data_0 = 0; //Zero the proximity data
uint8_t proximity_data_1 = 0;
const int stepsPerRevolution = 200; //each step is 1.8 deg (0.025mm), so 360/1.8=200
Stepper myStepper(stepsPerRevolution, 8, 9, 10, 11); //Initalize the stepper
uint8_t Start_flag=1;
uint8_t End_flag=0;
uint8_t Saftey=0; //saftey feature flag
const int rs = 12, en = 13, d4 = 5, d5 = 4, d6 = 3, d7 = 2;
LiquidCrystal lcd(rs, en, d4, d5, d6, d7);


void setup() {
  lcd.begin(16, 2);
  lcd.print("Initializing");
  pinMode(7,INPUT); // PIR input data
  pinMode(6,OUTPUT); //Relay Output
  digitalWrite(6,LOW); //Make sure the press is shutdown before starting
  tcaselect(0); //Proximity Sensor 1 //ADD TESTS TO MAKE SURE THE SENSORS WORK
  apds_0.init(); //Initalize Proximity Sensor 1
  apds_0.enableProximitySensor(false);
  tcaselect(1); //Proximity Sensor 2
  apds_1.init(); // Initalize Proximity Sensor 2
  apds_1.enableProximitySensor(false);
  myStepper.setSpeed(60); //Sets stepper speed


}

void loop() {
  int PIR_Value=digitalRead(7);
  if (PIR_Value==LOW||Saftey==0){
    tcaselect(0);
    apds_0.readProximity(proximity_data_0); //Read the first sensor proximity 
    tcaselect(1);
    apds_1.readProximity(proximity_data_1); //Read the second sensor proximity 
    //check what state were in 
    if (proximity_data_0<5 && proximity_data_1<5){ //move to cut the next part   
      lcd.clear();
      lcd.print("Working...");
      delay(500);
      myStepper.step(19.57*stepsPerRevolution);//each step is 0.025mm, so 97.85mm is 3914 steps  
    }
    else if(proximity_data_0>5 && proximity_data_1<5){
      delay(500);
      myStepper.step(stepsPerRevolution); //after cutting move until the end of the matrix
      End_flag=1;
    }
    else{
      if (Start_flag==1){
        lcd.clear();
        lcd.print("Initial");
        lcd.setCursor(0, 1);
        lcd.print("Positioning");
        tcaselect(0);
        while (proximity_data_0>5){ //keep moving until you see that you're over the first sensor
          myStepper.step(0.2*stepsPerRevolution); //move 1mm each loop
          delay(250);
          apds_0.readProximity(proximity_data_0);
        }
        myStepper.step(18*stepsPerRevolution);
        tcaselect(1);
        while (proximity_data_1>5){ //keep moving until you see that you're over the second sensor as well
          myStepper.step(0.2*stepsPerRevolution);
          delay(250);
          apds_1.readProximity(proximity_data_1);
        } 
        Start_flag=0;
        digitalWrite(6,HIGH); //Turn on the press, begin cutting      
      }
      if (End_flag==1){
        digitalWrite(6,LOW); //Stop cutting     
      }
    }
  }
  else {
    digitalWrite(6,LOW); //Shutdown the press
    Saftey=1;
    lcd.clear();
    lcd.print("Saftey Feature");
    lcd.setCursor(0, 1);
    lcd.print("Engaged!");
  }
}
