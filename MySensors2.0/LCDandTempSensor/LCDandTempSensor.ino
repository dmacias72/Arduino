// Enable debug prints to serial monitor
#define MY_DEBUG 

// Enable and select radio type attached
#define MY_RADIO_NRF24
//#define MY_RADIO_RFM69

// Enabled repeater feature for this node
#define MY_REPEATER_FEATURE

#define MY_NODE_ID 8

#include <SPI.h>
#include <MySensors.h>
#include <Bounce2.h>
#include <Time.h>
#include <DS3232RTC.h>  // A  DS3231/DS3232 library
#include <Wire.h>
#include <LiquidCrystal.h>
#include <DallasTemperature.h>
#include <OneWire.h>

#define CHILD_ID 4
#define BUTTON_PIN  4  // Arduino Digital I/O pin for button/reed switch
Bounce debouncer = Bounce(); 
int oldValue = 0;

bool timeReceived = false;
unsigned long lastUpdate=0, lastRequest=0;

#define ONE_WIRE_BUS 3 // Pin where dallas sensor is connected 
#define MAX_ATTACHED_DS18B20 4
OneWire oneWire(ONE_WIRE_BUS); // Setup a oneWire instance to communicate with any OneWire devices (not just Maxim/Dallas temperature ICs)
DallasTemperature sensors(&oneWire); // Pass the oneWire reference to Dallas Temperature. 
float lastTemperature[MAX_ATTACHED_DS18B20];
int numSensors=0;
bool receivedConfig = false;
bool metric = false;

/*
  LiquidCrystal Library

 Hitachi HD44780 driver. 20X4

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

LiquidCrystal lcd(A0, A1, A2, A3, 7, 8); // LCD interface pins

// Initialize temperature message
MyMessage msg(0,V_TEMP);

// Initialize button switch
// Change to V_LIGHT if you use S_LIGHT in presentation below
MyMessage msg2(CHILD_ID,V_TRIPPED);

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
  setSyncProvider(RTC.get);  

  // Request latest time from controller at startup
  requestTime();  

  lcd.clear();
  lcd.begin(20,4); // set up the LCD's number of columns and rows: 20 ROWS / 4 LINES
  lcd.setCursor(0,0); lcd.print("Arduino Reef Display");
  lcd.setCursor(0,2); lcd.print("Reef:");
  lcd.setCursor(0,3); lcd.print("Room:");
  lcd.setCursor(17,2); lcd.write(223); lcd.print("F");
  lcd.setCursor(17,3); lcd.write(223); lcd.print("F");

  // Setup the button & Activate internal pull-up
  pinMode(BUTTON_PIN,INPUT_PULLUP);
  
  // After setting up the button, setup debouncer
  debouncer.attach(BUTTON_PIN);
  debouncer.interval(5);

}

void presentation() {
  // Send the sketch version information to the gateway and Controller
  sendSketchInfo("LCD, Time, Temp "1.0");

  // Register binary input sensor to gw (they will be created as child devices)
  // You can use S_DOOR, S_MOTION or S_LIGHT here depending on your usage. 
  // If S_LIGHT is used, remember to update variable type you send in. See "msg" above.
  present(CHILD_ID, S_DOOR);

  // Fetch the number of attached temperature sensors  
  numSensors = sensors.getDeviceCount();

  // Present all sensors to controller
  for (int i=0; i<numSensors && i<MAX_ATTACHED_DS18B20; i++) {   
     present(i, S_TEMP);
  }
}

// This is called when a new time value was received
void receiveTime(unsigned long controllerTime) {
  // Ok, set incoming time 
  Serial.print("Time value received: ");
  Serial.println(controllerTime);
  RTC.set(controllerTime); // this sets the RTC to the time from controller - which we do want periodically
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
}

void printTime(){
  tmElements_t tm;
  RTC.read(tm);

  // Print date and time 
  lcd.setCursor(0,1);  lcd.print(dayShortStr(weekday()));
  lcd.setCursor(5,1);  lcd.print(monthShortStr(tm.Month));
  lcd.setCursor(9,1);
  if (day() < 10)
    lcd.print(" ");
  lcd.print(day());
  lcd.setCursor(12,1);
  if (hourFormat12() < 10)
    lcd.print(" ");
  lcd.print(hourFormat12());
  lcd.print(":");
  printDigits(tm.Minute);
  lcd.setCursor(18,1);
  if (isAM())
    lcd.print("AM");
  if (isPM())
    lcd.print("PM");
}

void printTemp(float temperature){
  // Go to next line and print temperature
    lcd.setCursor(13,2); printTempDigits(temperature);
    lcd.setCursor(13,3); printTempDigits(temperature);
}

void printTempDigits(float digits){
  if (digits > -127.00 && digits < 185.00)
    lcd.print(digits,1);
  else
    lcd.print("N/A ");
}

void printDigits(int digits){
  if(digits < 10)
    lcd.print('0');
  lcd.print(digits);
}

