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
#include <TM1637TinyDisplay.h>

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


/*******************************
  Variables
*******************************/
const int readTempEvery = 1000; // how often should we read the temp, ms

const int flipScreenEvery = 500;
boolean showTemp = HIGH;

float currentMillis = 0;
float lastReadMillis = 0;
float lastFlipMillis = 0;

const int minPWM = 40;

int PWMValue = 0;
boolean PWMEnabled = LOW;

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
    Serial.println();
    Serial.print("--- Requesting temperatures...");

    sensors.requestTemperatures(); // Send the command to get temperature readings
    currentTemp = sensors.getTempCByIndex(0);

    Serial.println(" DONE");
    Serial.print("Temperature is: ");
    Serial.println(currentTemp);

    /*******************************
      Calculate PWM
    *******************************/
    if (currentTemp >= 30 && PWMEnabled ) {
      PWMValue = map(currentTemp, 30, 60, 0, 255);
      if (PWMValue > 255) {
        PWMValue = 255;
      }
      if (PWMValue < minPWM / 100 * 255) {
        PWMValue = minPWM / 100 * 255;
      }

    } else {
      PWMValue = 0;
    }

    Serial.print("PWM value: ");
    Serial.print(PWMValue);
    Serial.print("\t");
    Serial.print(PWMValue / 255 * 100);
    Serial.println("%");

    lastReadMillis = currentMillis;

  }

  /*******************************
    Read inputs
  *******************************/
  PWMEnabled = digitalRead(enablePin);

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
      //Serial.println("Showing temp on screen...");
      display.showNumber(currentTemp, 0);
      showTemp = LOW;
    } else {
      //Serial.println("Showing PWM on screen...");
      display.showNumber(PWMValue / 255 * 100, 0);
      showTemp = HIGH;
    }
    
    lastFlipMillis = currentMillis;
  }
}
