 // Enable debug prints to serial monitor
#define MY_DEBUG 

// Enable and select radio type attached
#define MY_RADIO_NRF24
//#define MY_RADIO_RFM69

// Enabled repeater feature for this node
#define MY_REPEATER_FEATURE

#define MY_NODE_ID 9

#include <SPI.h>
#include <MySensors.h>
#include <Bounce2.h>
#include <Time.h>
//#include <DS3232RTC.h>  // A  DS3231/DS3232 library
//#include <Wire.h>
#include <LiquidCrystal.h>
#include <DallasTemperature.h>
#include <OneWire.h>
#include <FastLED.h> 

/*** RGB Defintions ***/
#define DATA_PIN 5 // data pin for RGB strip
#define NUM_LEDS 30 //The number of RGB's
#define RGB_LIGHT_CHILD 5
#define RGB_DAWN_CHILD 6
#define RGB_DAY_CHILD 7
#define RGB_NIGHT_CHILD 8

CRGB leds[NUM_LEDS];

#define BRIGHTNESS  128

char controllerRGBvalue[] = "0000FF";                       // Controller sent RGB value, default
uint16_t curBrightness, actualBrightness, controllerRGBbrightness = 0x7F ;  // Brightness globals
unsigned long updateBrightnessDelay, lastBrightnessUpdate ; // Brightness timers
int RGBonoff ;                                              // OnOff flag

enum {Off, Dawn, Day, Night}; // Color globals (stored in int for convenience)
const int lastColorIdx = Night + 1; // use last Color for Color count
int curColor;
unsigned long updateColorDelay, lastColorUpdate; // Color timers

unsigned long idleTimer = millis(); // return to idle timer
int idleTime = 10000; // return to idle after 10 secs

MyMessage lightRGBMsg(RGB_LIGHT_CHILD,  V_RGB); // standard messages, light
MyMessage lightdimmerMsG(RGB_LIGHT_CHILD ,V_DIMMER); 
MyMessage lightOnOffMessage(RGB_LIGHT_CHILD, V_STATUS);

/*** Button Definitions ***/
#define CHILD_ID 4
#define BUTTON_PIN  4  // Arduino Digital I/O pin for button/reed switch
Bounce debouncer = Bounce(); 
int oldValue = 0;

bool timeReceived = false;
unsigned long lastUpdate=0, lastRequest=0;

// Initialize button switch
// Change to V_LIGHT if you use S_LIGHT in presentation below
MyMessage msg2(CHILD_ID,V_TRIPPED);

/*** Temperature Definitions ***/
#define ONE_WIRE_BUS 3 // Pin where dallas sensor is connected 
#define MAX_ATTACHED_DS18B20 4
OneWire oneWire(ONE_WIRE_BUS); // Setup a oneWire instance to communicate with any OneWire devices (not just Maxim/Dallas temperature ICs)
DallasTemperature sensors(&oneWire); // Pass the oneWire reference to Dallas Temperature. 
float lastTemperature[MAX_ATTACHED_DS18B20];
int numSensors=0;
bool receivedConfig = false;
bool metric = false;

// Initialize temperature message
MyMessage msg(0,V_TEMP);

/*
  LiquidCrystal Library

 Hitachi HD44780 driver. 20X2

 * LCD RS pin to digital pin A0
 * LCD E  pin to digital pin A1 (LCD Enable Pin)
 * LCD D4 pin to digital pin A2
 * LCD D5 pin to digital pin A3
 * LCD D6 pin to digital pin A4
 * LCD D7 pin to digital pin A5
 * LCD R/W pin to ground
 * ends to +5V and ground
 * wiper to LCD VO pin (pin 3)

*/

LiquidCrystal lcd(A0, A1, A2, A3, A4, A5); // LCD interface pins

void before()
{
  // Startup OneWire library
  sensors.begin();
}

void setup()  
{  
  // requestTemperatures() will not block current thread
  sensors.setWaitForConversion(false);

  // the function to get the time from the RTC
  //setSyncProvider(RTC.get);  

  // Request latest time from controller at startup
  requestTime();

  lcd.clear();
  lcd.begin(16,2); // set up the LCD's number of columns and rows: 16 ROWS / 2 LINES
  lcd.setCursor(0,0); lcd.print("  Axolotl Tank");
  lcd.setCursor(14,1); lcd.print((char)178); lcd.print("F");

  // Setup the button & Activate internal pull-up
  pinMode(BUTTON_PIN,INPUT_PULLUP);
  
  // After setting up the button, setup debouncer
  debouncer.attach(BUTTON_PIN);
  debouncer.interval(5);

  FastLED.addLeds<NEOPIXEL, DATA_PIN >(leds, NUM_LEDS) ;

  // initialize strip with color and show (strip expects long, so convert from String)
  for(int i = 0 ; i < 6 ; i++) {                          // get color value from EEPROM (6 char)
    controllerRGBvalue[i] = loadState(i) ;
    }
  setColor(Off) ;                         // default controller Solid 
  FastLED.show();
}

void presentation() {
  // Send the sketch version information to the gateway and Controller
  sendSketchInfo("LED, LCD & TEMP", "1.0");

  // Fetch the number of attached temperature sensors  
  numSensors = sensors.getDeviceCount();

  // Present all sensors to controller
  for (int i=0; i<numSensors && i<MAX_ATTACHED_DS18B20; i++) {   
     present(i, S_TEMP);
  }

  // Register binary input sensor to gw (they will be created as child devices)
  // You can use S_DOOR, S_MOTION or S_LIGHT here depending on your usage. 
  // If S_LIGHT is used, remember to update variable type you send in. See "msg" above.
  present(CHILD_ID, S_DOOR);

  present(RGB_LIGHT_CHILD, S_DIMMER, "RGB Wall Light 0");
  present(RGB_DAWN_CHILD, S_LIGHT, "RGB Actinic");
  present(RGB_DAY_CHILD, S_LIGHT, "RGB White");
  present(RGB_NIGHT_CHILD, S_LIGHT, "RGB Dark Red");

}

// This is called when a new time value was received
void receiveTime(unsigned long controllerTime) {
  // Ok, set incoming time 
  Serial.print("Time value received: ");
  Serial.println(controllerTime);
  setTime(controllerTime);
  //RTC.set(controllerTime); // this sets the RTC to the time from controller - which we do want periodically
  timeReceived = true;
}

void loop() 
{
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
    printTime();
    lastUpdate = now;
  }

  // Fetch temperatures from Dallas sensors
  sensors.requestTemperatures(); 

  // Read temperatures and send them to controller 
  for (int i=0; i<numSensors && i<MAX_ATTACHED_DS18B20; i++) {

    // Fetch and round temperature to one decimal
    float temperature = static_cast<float>(static_cast<int>((getConfig().isMetric?sensors.getTempCByIndex(i):sensors.getTempFByIndex(i)) * 10.)) / 10.;

    // Only send data if temperature change is greater than 0.2
    if (static_cast<unsigned int>(lastTemperature[i]*10 - temperature*10) > 2) {

      // Send the new temperature
      send(msg.setSensor(i).set(temperature,1));
      // Print to LCD Screen
      printTemp(temperature);
      // Save new temperatures for next compare
      lastTemperature[i]=temperature;
    }
  }
  debouncer.update();
  // Get the update value
  int value = debouncer.read();

  if (value != oldValue) {
     // Send in the new value
     send(msg2.set(value==HIGH ? 0 : 1));
    oldValue = value;
  }
  updateLightBrightness(); // update Brightness if time
}

// Sets the light brightness, takes value and time (ms) as input
void setLightBrightness(int newBrightness, unsigned long updateTime){
  updateBrightnessDelay = updateTime / 0xFF ; // delay = time / max steps
  actualBrightness = curBrightness ; // assume curBrightness is actual
  curBrightness = newBrightness ; // set curBrightness to new value, rest is done in update
}   
 
// Update the light brightness if time
void updateLightBrightness(){
  unsigned long now = millis() ;
  if (now > lastBrightnessUpdate + updateBrightnessDelay){// check if time for update
    if ( actualBrightness > curBrightness) {
      FastLED.setBrightness( actualBrightness-- );
      FastLED.show();
    } else if ( actualBrightness < curBrightness){
      FastLED.setBrightness( actualBrightness++ );
      FastLED.show();
    }
    lastBrightnessUpdate = now ;
  }
}

// **** Color routines *****
// Sets and initializes the light Color if nescessary
void setColor( int newColor){
  curColor = newColor ;
  switch(curColor){
    case Dawn:
      for(int i = 0 ; i < NUM_LEDS ; i++) leds[i] = CRGB(0x0000FF);
      FastLED.show();
      break ;
    case Day:
      for(int i = 0 ; i < NUM_LEDS ; i++) leds[i] = CRGB(0xFFFFFF);
       FastLED.show();
       break ;
    case Night:
      for(int i = 0 ; i < NUM_LEDS ; i++) leds[i] = CRGB(0x880000);
      FastLED.show();
      break ;
    case Off:
      for(int i = 0 ; i < NUM_LEDS ; i++) leds[i] = 0 ;
      FastLED.show();
      break ;
    default :
      break ;
  }
}   

// Incoming messages from MySensors
void receive(const MyMessage &message) {
  int ID = message.sensor;
  Serial.print("Sensor: ");
  Serial.println(ID);
  switch (ID){
    case RGB_LIGHT_CHILD:
      if (message.type == V_RGB) {// check for RGB type
        strcpy(controllerRGBvalue, message.getString()); // get the payload
        setColor(Day);
      } else if (message.type == V_DIMMER) { // if DIMMER type, adjust brightness
        controllerRGBbrightness = map(message.getLong(), 0, 100, 0, 255);
        setLightBrightness(controllerRGBbrightness, 2000) ;
      } else if (message.type == V_STATUS) { // if on/off type, toggle brightness
        RGBonoff = message.getInt();
        setLightBrightness((RGBonoff == 1)?controllerRGBbrightness:0, 2000);
      }
      break ;
    case RGB_DAWN_CHILD:
      setColor(Dawn);
      break ;
    case RGB_DAY_CHILD:
      setColor(Day);
      break ;
    case RGB_NIGHT_CHILD:
      setColor(Night);
      break ;
    }
  FastLED.show();
  dispRGBstat();
}

// display the status of all RGB: controller, requested, real
void dispRGBstat(void){
  Serial.print(" Color: "); Serial.print(controllerRGBvalue); 
  Serial.print(" Brightness: "); Serial.println(controllerRGBbrightness);
}

void printTime(){
  //tmElements_t tm;
  //RTC.read(tm);

  // Print time 
  lcd.setCursor(0,1);
  if (hourFormat12() < 10)
    lcd.print(" ");
  lcd.print(hourFormat12());
  lcd.print(":");
  printDigits(minute());
  lcd.setCursor(6,1);
  if (isAM())
    lcd.print("AM");
  if (isPM())
    lcd.print("PM");
}

void printTemp(float temperature){
  // Go to next line and print temperature
    lcd.setCursor(10,1); printTempDigits(temperature);
}

void printTempDigits(float digits){
  if (digits > -127.00 && digits < 185.00)
    lcd.print(digits,1);
  else
    lcd.print("NA");
}

void printDigits(int digits){
  if(digits < 10)
    lcd.print('0');
  lcd.print(digits);
}

