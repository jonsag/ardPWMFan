/*******************************
  Program info
*******************************/
#include <WString.h>

String programName = "ardPWMFan";
String date = "20210119";
String author = "Jon Sagebrand";
String email = "jonsagebrand@gmail.com";



/*******************************
  TM1637 4 x 7-segment display
*******************************/
#include <Arduino.h>
#include <TM1637TinyDisplay.h> // https://github.com/jasonacox/TM1637TinyDisplay

const int CLK = 8;
const int DIO = 9;

TM1637TinyDisplay display(CLK, DIO);


/*******************************
  DS18B20 1-wire temperature sensor
*******************************/
#include <OneWire.h>
#include <DallasTemperature.h>

#define ONE_WIRE_BUS 3

OneWire oneWire(ONE_WIRE_BUS);

DallasTemperature sensors(&oneWire);

float currentTemp = 0;


/*******************************
  Pins
*******************************/
const int pwmPin = 6; // pin that controls the MOSFET
const int enablePin = 2; // enable this input to output PWM signal

const int minPWM = 20;

int PWMValue = 0;
boolean PWMEnabled = LOW;


/*******************************
  Variables
*******************************/
const int readTempEvery = 1000; // how often should we read the temp, ms

const int flipScreenEvery = 500;
boolean showTemp = HIGH;

float currentMillis = 0;
float lastReadMillis = 0;
float lastFlipMillis = 0;


void setup() {
  /*******************************
    Start serial
  *******************************/
  Serial.begin(9600);

  /*******************************
    Print start information
  *******************************/
  Serial.println(programName); // print information
  Serial.println(date);
  Serial.print("by ");
  Serial.println(author);
  Serial.println(email);
  Serial.println();

  /*******************************
    In- and outputs
  *******************************/
  pinMode(pwmPin, OUTPUT); // this is the output pin for the MOSFET controoling the heater
  pinMode(enablePin, INPUT);

  /*******************************
    DS18B20 sensor
  *******************************/
  sensors.begin();

  /*******************************
    LED display
  *******************************/
  display.clear();
  display.setBrightness(BRIGHT_HIGH);

}

void loop() {

  currentMillis = millis();

  /*******************************
    Read temp
  *******************************/
  if (currentMillis - lastReadMillis >= readTempEvery) {
    //Serial.println();
    Serial.print("--- Requesting temperatures...");

    sensors.requestTemperatures(); // Send the command to get temperature readings
    currentTemp = sensors.getTempCByIndex(0);

    Serial.print(" DONE");
    Serial.print(" \tTemperature is: ");
    Serial.print(currentTemp);

    /*******************************
      Calculate PWM
    *******************************/
    if (currentTemp >= 30 && PWMEnabled ) {
      PWMValue = map(currentTemp, 30, 60, minPWM * 255 / 100, 255);

      Serial.print("\t Calculated PWM value: ");
      Serial.print(PWMValue);

      if (PWMValue > 255) {
        PWMValue = 255;
      } else if (PWMValue < minPWM * 255 / 100 ) {
        PWMValue = minPWM * 255 / 100;
      }

    } else {
      PWMValue = 0;
    }

    Serial.print("\t PWM value: ");
    Serial.print(PWMValue);
    Serial.print(" \t");
    Serial.print(PWMValue * 100 / 255);
    Serial.println("%");

    lastReadMillis = currentMillis;

  }

  /*******************************
    Read inputs
  *******************************/
  PWMEnabled = digitalRead(enablePin);
  analogWrite(pwmPin, PWMValue);
  /*******************************
    PWM
  *******************************/
  if (PWMEnabled) {
    analogWrite(pwmPin, PWMValue);
  } else {
    analogWrite(pwmPin, 0);
  }

  /*******************************
    Screen
  *******************************/
  if (currentMillis - lastFlipMillis >= flipScreenEvery) {
    if (showTemp) {
      display.showString("\xB0", 1, 3);
      display.showNumber(int(currentTemp), false, 3, 0);
      showTemp = LOW;
    } else {
      
      if (PWMEnabled) {
        display.showString("F", 1, 3);
        display.showNumber(PWMValue * 100 / 255, false, 3, 0);
      } else {
        display.showString(" nE ", 4, 0);
      }
      
      showTemp = HIGH;
    }

    lastFlipMillis = currentMillis;
  }
}
