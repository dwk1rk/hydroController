// Include LCD library and initialize library on given pins
#include <LiquidCrystal.h>
LiquidCrystal lcd(7, 8, 9, 10, 11, 12);

//Initialize DHT library, and create instance of DHT sensor
#include "DHT.h"
#define DHTPIN 2
#define DHTTYPE DHT22
DHT dht(DHTPIN, DHTTYPE);

//Include RTC library, Wire library,
#include <Wire.h>
#include "RTClib.h"

//create instace of RTC and create days of the week array
RTC_DS3231 rtc;
char daysOfTheWeek[7][12] = {"Sunday", "Monday", "Tuesday", "Wednesday", "Thursday", "Friday", "Saturday"};

//Initialize relay pins
int pumpRelay = 39;
int bubblerRelay = 31;
int lightRelay1 = 41;
int lightRelay2 = 43;
int fanRelay = 33;
int humidifierRelay = 35;


//Control Switch Pins
int pumpSwitch = 47;
int bubblerSwitch = 49;
int lightSwitch = 51;
int fanSwitch = 45; // NOT WORKING

//Cycle Status Booleans
boolean pumpCycle = false;
boolean bubblerCycle = false;
boolean lightCycle = false;
boolean fanCycle = false;

//Plant state control
boolean isVegetative;
boolean isFlowering;
boolean chooseStatus;
int vegSwitch = 3;
int flowerSwitch = 4;

boolean lightOverride;

void setup() {
  //Begin serial connection
  Serial.begin(9600);

  // Set number of columns & rows for LCD
  lcd.begin(16, 2);

  //Start dht sensor
  dht.begin();

  //Set relay pins to outputs
  pinMode(pumpRelay, OUTPUT);
  pinMode(bubblerRelay, OUTPUT);
  pinMode(lightRelay1, OUTPUT);
  pinMode(lightRelay2, OUTPUT);
  pinMode(fanRelay, OUTPUT);
  pinMode(humidifierRelay, OUTPUT);

  //Set relays HIGH (off)
  digitalWrite(pumpRelay, HIGH);
  digitalWrite(bubblerRelay, HIGH);
  digitalWrite(lightRelay1, HIGH);
  digitalWrite(lightRelay2, HIGH);
  digitalWrite(fanRelay, HIGH);
  digitalWrite(humidifierRelay, HIGH);

  //Set plant state pins to input
  pinMode(vegSwitch, INPUT);
  pinMode(flowerSwitch, INPUT);

  //Set switch pins to inputs
  pinMode(pumpSwitch, INPUT);
  pinMode(bubblerSwitch, INPUT);
  pinMode(lightSwitch, INPUT);
  pinMode(fanSwitch, INPUT);


  //If RTC does not start, print error to LCD
  if (! rtc.begin()) {
    lcd.print("Cannot Get Time");
    while (1);
  }

  // Print a message to the LCD.
  lcd.print("Booting Up...");
  delay (2000);
  lcd.clear();
}


void loop() {
  DateTime now = rtc.now();
  updateDisplay(now.hour(), now.minute(), now.second());
  switchControl(pumpSwitch, pumpRelay, pumpCycle);
  switchControl(bubblerSwitch, bubblerRelay, bubblerCycle);
  switchControl(fanSwitch, fanRelay, fanCycle);
  //switchControl(lightSwitch, lightRelay1, lightCycle);
  //switchControl(lightSwitch, lightRelay2, lightCycle);
  //Set plant state
  checkPlantState(vegSwitch, flowerSwitch);
  if (chooseStatus) {
    lcd.setCursor (9, 0);
    lcd.print ("RST");
  }
  if (isVegetative) {
    lcd.setCursor (9, 0);
    lcd.print ("VEG");
    int minTemp = 70;
    int maxTemp = 85;
    int minHumid = 40;
    int maxHumid = 70;
    if (now.hour() >= 18 && now.hour() <= 23 && digitalRead(lightSwitch) == HIGH) {
      digitalWrite(lightRelay1, LOW);
      digitalWrite(lightRelay2, LOW);
      lightCycle = true;
    }
    else if (now.hour() >= 0 && now.hour() < 12 && digitalRead(lightSwitch) == HIGH) {
      digitalWrite(lightRelay1, LOW);
      digitalWrite(lightRelay2, LOW);
      lightCycle = true;
    }
    else {
      digitalWrite(lightRelay1, HIGH);
      digitalWrite(lightRelay2, HIGH);
      lightCycle = false;
    }
    if (now.hour() == 18 && now.minute() == 30) {
      digitalWrite (pumpRelay, LOW);
      pumpCycle = true;
    }
    if (now.hour() == 18 && now.minute() == 45) {
      digitalWrite (pumpRelay, HIGH);
      pumpCycle = false;
    }
    if (now.hour() == 3 && now.minute() == 30) {
      digitalWrite (pumpRelay, LOW);
      pumpCycle = true;
    }
    if (now.hour() == 3 && now.minute() == 45) {
      digitalWrite (pumpRelay, HIGH);
      pumpCycle = false;
    }
    float h = dht.readHumidity();

    if (h <= 65) {
      digitalWrite(humidifierRelay, LOW);

    }
    if (h == 70) {
      digitalWrite(humidifierRelay, HIGH);
    }
  }

  if (isFlowering) {
    lcd.setCursor (9, 0);
    lcd.print ("FLW");
    int minTemp = 65;
    int maxTemp = 80;
    int minHumid = 40;
    int maxHumid = 50;
    if (now.hour() >= 18 && now.hour() <= 23 && digitalRead(lightSwitch) == HIGH) {
      digitalWrite(lightRelay1, LOW);
      digitalWrite(lightRelay2, LOW);
      lightCycle = true;
    }
    else if (now.hour() >= 0 && now.hour() < 6 && digitalRead(lightSwitch) == HIGH) {
      digitalWrite(lightRelay1, LOW);
      digitalWrite(lightRelay2, LOW);
      lightCycle = true;
    }
    else {
      digitalWrite(lightRelay1, HIGH);
      digitalWrite(lightRelay2, HIGH);
      lightCycle = false;
    }
    if (now.hour() == 0 && now.minute() == 0) {
      digitalWrite (pumpRelay, LOW);
      pumpCycle = true;
    }
    if (now.hour() == 0 && now.minute() == 20) {
      digitalWrite(pumpRelay, HIGH);
      pumpCycle = false;
    }
    float h = dht.readHumidity();
    if (h <= 40) {
      digitalWrite(humidifierRelay, LOW);

    }
    if (h == 50) {
      digitalWrite(humidifierRelay, HIGH);
    }
  }

  vent();
  delay(250);
  lcd.clear();
  lcd.begin(16, 2);

}


////////////////Functions//////////////////

void updateDisplay(int hourVal, int minuteVal, int secondVal) {
  DateTime now = rtc.now();
  lcd.print(hourVal);
  lcd.print(':');
  lcd.print(minuteVal);
  lcd.print(':');
  lcd.print(secondVal);
  float h = dht.readHumidity();
  float f = dht.readTemperature(true);
  lcd.setCursor(0, 1);
  lcd.print("t:");
  lcd.print(f);
  lcd.print(" ");
  lcd.print ("h:");
  lcd.print (h);
  //lcd.setCursor(2, 1);
  //lcd.print(f);
  //lcd.setCursor(8, 1);
  //lcd.print(h);
  if (isnan(h) || isnan(f)) {
    lcd.print("Cannot Get Temp");
    return;
  }
}

void checkPlantState(int switch1, int switch2) {
  int vegState = digitalRead(switch1);
  int flowerState = digitalRead(switch2);
  if (vegState == HIGH) {
    isVegetative = true;
    isFlowering = false;
    chooseStatus  = false;
  }

  else if (flowerState == HIGH) {
    isFlowering = true;
    isVegetative = false;
    chooseStatus  = false;
  }
  else {
    chooseStatus  = true;
    isVegetative = false;
    isFlowering = false;
  }
}

void switchControl (int switchName, int relayPin, boolean cycleState) {
  if (cycleState == false) {
    int switchState = digitalRead(switchName);
    int relayState = digitalRead(relayPin);
    Serial.print(switchState);
    if (switchState == HIGH && relayState == HIGH) {
      digitalWrite(relayPin, LOW);
    }
    if (switchState == LOW && relayState == LOW) {
      digitalWrite(relayPin, HIGH);
    }
  }
}

void vent() {
  DateTime now = rtc.now();
  int fanRelayState = digitalRead(fanRelay);
  float f = dht.readTemperature(true);

  if (lightCycle == true && digitalRead(lightSwitch) == HIGH) {
    if (f >= 80){
      digitalWrite(fanRelay, LOW);
      fanCycle = true;
      }
    if (f <= 76.5){
      digitalWrite(fanRelay, HIGH);
      fanCycle = false;
      }
    
    /*if (now.minute() >= 1   && now.minute() < 15 && f >= 77) {
      digitalWrite(fanRelay, LOW);
      fanCycle = true;
    }
    else if (now.minute() >= 20  && now.minute() < 35 && f >= 77) {
      digitalWrite(fanRelay, LOW);
      fanCycle = true;
    }

    else if (now.minute() >= 40 && now.minute() < 55 && f >= 77) {
      digitalWrite(fanRelay, LOW);
      fanCycle = true;
    }
    else {
      digitalWrite(fanRelay, HIGH);
      fanCycle = false;
    }
    */
  }
  if (lightCycle == false) {
    if (now.minute() >= 30  && now.minute() <= 35) {
      digitalWrite(fanRelay, LOW);
      fanCycle = true;
    }
    else {
      digitalWrite(fanRelay, HIGH);
      fanCycle = false;
    }
  }
}


void pump() {
  DateTime now = rtc.now();
  if (now.hour() == 16) {
    digitalWrite(pumpRelay, LOW);
    pumpCycle = true;
  }
  if (now.hour() == 18 && now.minute() == 20) {
    digitalWrite(pumpRelay, HIGH);
    pumpCycle = false;
  }
}


