/**
 * The MySensors Arduino library handles the wireless radio link and protocol
 * between your home built sensors/actuators and HA controller of choice.
 * The sensors forms a self healing radio network with optional repeaters. Each
 * repeater and gateway builds a routing tables in EEPROM which keeps track of the
 * network topology allowing messages to be routed to nodes.
 *
 * Created by Henrik Ekblad <henrik.ekblad@mysensors.org>
 * Copyright (C) 2013-2015 Sensnology AB
 * Full contributor list: https://github.com/mysensors/Arduino/graphs/contributors
 *
 * Documentation: http://www.mysensors.org
 * Support Forum: http://forum.mysensors.org
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * version 2 as published by the Free Software Foundation.
 *
 *******************************
 *
 * REVISION HISTORY
 * Version 1.0 - Henrik EKblad
 * Contribution by a-lurker and Anticimex,
 * Contribution by Norbert Truchsess <norbert.truchsess@t-online.de>
 * Contribution by Tomas Hozza <thozza@gmail.com>
 *
 *
 * DESCRIPTION
 * The EthernetGateway sends data received from sensors to the ethernet link.
 * The gateway also accepts input on ethernet interface, which is then sent out to the radio network.
 *
 * The GW code is designed for Arduino 328p / 16MHz.  ATmega168 does not have enough memory to run this program.
 *
 * LED purposes:
 * - To use the feature, uncomment WITH_LEDS_BLINKING in MyConfig.h
 * - RX (green) - blink fast on radio message recieved. In inclusion mode will blink fast only on presentation recieved
 * - TX (yellow) - blink fast on radio message transmitted. In inclusion mode will blink slowly
 * - ERR (red) - fast blink on error during transmission error or recieve crc error
 *
 * See http://www.mysensors.org/build/ethernet_gateway for wiring instructions.
 *
 */

// Enable debug prints to serial monitor
#define MY_DEBUG 

// Enable and select radio type attached
#define MY_RADIO_NRF24
//#define MY_RADIO_RFM69

// Enable gateway ethernet module type 
#define MY_GATEWAY_W5100

// W5100 Ethernet module SPI enable (optional if using a shield/module that manages SPI_EN signal)
//#define MY_W5100_SPI_EN 4  

// Enable Soft SPI for NRF radio (note different radio wiring is required)
// The W5100 ethernet module seems to have a hard time co-operate with 
// radio on the same spi bus.
#if !defined(MY_W5100_SPI_EN) && !defined(ARDUINO_ARCH_SAMD)
  #define MY_SOFTSPI
  #define MY_SOFT_SPI_SCK_PIN A0
  #define MY_SOFT_SPI_MISO_PIN A2
  #define MY_SOFT_SPI_MOSI_PIN A1
#endif  

// When W5100 is connected we have to move CE/CSN pins for NRF radio
#ifndef MY_RF24_CE_PIN 
  #define MY_RF24_CE_PIN 5
#endif
#ifndef MY_RF24_CS_PIN 
  #define MY_RF24_CS_PIN 6
#endif

// Enable to UDP          
//#define MY_USE_UDP

#define MY_IP_ADDRESS 192,168,69,46   // If this is disabled, DHCP is used to retrieve address
// Renewal period if using DHCP
//#define MY_IP_RENEWAL_INTERVAL 60000
// The port to keep open on node server mode / or port to contact in client mode
#define MY_PORT 5003      

// Controller ip address. Enables client mode (default is "server" mode). 
// Also enable this if MY_USE_UDP is used and you want sensor data sent somewhere. 
//#define MY_CONTROLLER_IP_ADDRESS 192, 168, 178, 254   
 
// The MAC address can be anything you want but should be unique on your network.
// Newer boards have a MAC address printed on the underside of the PCB, which you can (optionally) use.
// Note that most of the Ardunio examples use  "DEAD BEEF FEED" for the MAC address.
#define MY_MAC_ADDRESS 0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED

// Flash leds on rx/tx/err
#define MY_LEDS_BLINKING_FEATURE
// Set blinking period
#define MY_DEFAULT_LED_BLINK_PERIOD 300

// Enable inclusion mode
#define MY_INCLUSION_MODE_FEATURE
// Enable Inclusion mode button on gateway
#define MY_INCLUSION_BUTTON_FEATURE
// Set inclusion mode duration (in seconds)
#define MY_INCLUSION_MODE_DURATION 60 
// Digital pin used for inclusion mode button
#define MY_INCLUSION_MODE_BUTTON_PIN  3 

// Uncomment to override default HW configurations
#define MY_DEFAULT_ERR_LED_PIN 7  // Error led pin red/orange
#define MY_DEFAULT_RX_LED_PIN  8  // Receive led pin green
#define MY_DEFAULT_TX_LED_PIN  9  // the PCB, on board LED yellow

#include <SPI.h>
#include <DS3232RTC.h>  // A  DS3231/DS3232 library
#include <Wire.h>
#include <LiquidCrystal.h> 


#if defined(MY_USE_UDP)
  #include <EthernetUdp.h>
#endif
#include <Ethernet.h>
#include <MySensors.h>

boolean timeReceived = false;
unsigned long lastUpdate=0, lastRequest=0;

/*LiquidCrystal Library

 Hitachi HD44780 driver. 20X4

 * LCD RS pin to digital pin A4
 * LCD E  pin to digital pin A5 (LCD Enable Pin)
 * LCD D4 pin to digital pin A8
 * LCD D5 pin to digital pin A9
 * LCD D6 pin to digital pin A10
 * LCD D7 pin to digital pin A11
 * LCD R/W pin to ground
 * ends to +5V and ground
 * wiper to LCD VO pin (pin 3)
*/

LiquidCrystal lcd(A4, A5, A8, A9, A10, A11); // LCD interface pins

void setup() {
  // the function to get the time from the RTC
  setSyncProvider(RTC.get);  

  // Request latest time from controller at startup
  requestTime();  
  
  lcd.clear();
  lcd.begin(16,2); // set up the LCD's number of columns and rows: 16 ROWS / 2 LINES
  lcd.home();
  lcd.print(" My Sensors 2.0 ");
  lcd.setCursor(0,1);
  lcd.print("    Gateway     ");
}

// This is called when a new time value was received
void receiveTime(unsigned long controllerTime) {
  // Ok, set incoming time 
  Serial.print("Time value received: ");
  Serial.println(controllerTime);
  RTC.set(controllerTime); // this sets the RTC to the time from controller - which we do want periodically
  timeReceived = true;
  //updateDisplay();
} 

void loop() {
  unsigned long now = millis();

  // If no time has been received yet, request it every 10 second from controller
  // When time has been received, request update every hour
  if ((!timeReceived && (now-lastRequest) > (10UL*1000UL))
    || (timeReceived && (now-lastRequest) > (60UL*1000UL*60UL))) {
    // Request time from controller. 
    Serial.println("requesting time");
    requestTime();  
    lastRequest = now;
  }
  
  // Print time every 30 second
  if (timeReceived && now-lastUpdate > 30000) {
    updateDisplay();
    lastUpdate = now;
  }

}

void updateDisplay(){
  tmElements_t tm;
  RTC.read(tm);

  // Print date and time 
  //MON JUL 03 12:30
  lcd.home();
  lcd.print(dayShortStr(weekday()));
  lcd.print(" ");
  lcd.print(monthShortStr(tm.Month));
  lcd.print(" ");
  printDigits(tm.Day);
  lcd.print(" ");
  printDigits(tm.Hour);
  lcd.print(":");
  printDigits(tm.Minute);

  // Go to next line and print temperature
  lcd.setCursor(0,1);  
  lcd.print("Temp: ");
  lcd.print(((RTC.temperature()/4)*9/5)+32);
  lcd.print((char)178); // Degree-sign
  lcd.print("F ");
}

void printDigits(int digits){
  if(digits < 10)
    lcd.print('0');
  lcd.print(digits);
}


