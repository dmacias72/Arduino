/*
  LiquidCrystal Library - Hello World

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

#include <SPI.h>
#include <EEPROM.h>  
#include <RF24.h>
#include <Sensor.h>

// LCD only code start
#include <LiquidCrystal.h>
// LCD only code end

// Relay only code start
#include <Relay.h>
// Relay only code end

// Temp only code start
#include <DallasTemperature.h>
#include <OneWire.h>
// Temp only code end

// Set RADIO_ID to something unique in your sensor network (1-254)
// or set to AUTO if you want gw to assign a RADIO_ID for you.
#define RADIO_ID 2

// Relay only code start
#define RELAY_1  5  // Arduino Digital I/O pin number for first relay (second on pin+1 etc)
#define NUMBER_OF_RELAYS 4 
#define BUTTON_PIN  3  // Arduino Digital I/O pin number for button (no resistor needed, connect to button then ground)
#define RELAY_CTRL 5  //Arduino Digital I/O pin that the button controls

#define RELAY_ON 0
#define RELAY_OFF 1

int buttVal;
int previousVal;
int state;
int switched = 0;
long pressTime = 0;    // the last time the button pin was pressed
long debounce = 200;   // the debounce time, increase if the output flickers
// Relay only code end

// Temp only code start
#define ONE_WIRE_BUS 4 // Pin where dallas sensor is connected 

unsigned long SLEEP_TIME = 120; // Sleep time between reads (in seconds)

OneWire oneWire(ONE_WIRE_BUS);
DallasTemperature sensors(&oneWire);
// Temp only code end

// LCD only code start
LiquidCrystal lcd(A0, A1, A2, A3, A4, A5); // LCD interface pins
// LCD only code end

Sensor gw(9,10);

// Temp only code start
#define MAX_ATTACHED_DS18B20 3

float lastTemperature[MAX_ATTACHED_DS18B20];
int numSensors=0;
boolean metric; 
// Temp only code end

void setup()  
{  
  Serial.begin(BAUD_RATE);  // Used to write debug info
  sensors.begin();
  gw.begin(RADIO_ID);
  
  // LCD only code start
  lcd.clear();
  lcd.begin(20, 4); // set up the LCD's number of columns and rows: 20 ROWS / 4 LINES
  // LCD only code end

  // Set buttonPin as Input
  pinMode(BUTTON_PIN, INPUT);
  digitalWrite(BUTTON_PIN,HIGH); //Turn On internal Pull-up resistor
  buttVal = digitalRead(BUTTON_PIN); //read initial state

  // Fetch the number of attached sensors  
  numSensors = sensors.getDeviceCount();
  // Register all sensors to gw (they will be created as child devices)
  for (int i=0; i<numSensors; i++) {   
     gw.sendSensorPresentation(i, S_TEMP);
  }
  metric = gw.isMetricSystem();
  
  // Register all sensors to gw (they will be created as child devices)
  for (int i=0; i<NUMBER_OF_RELAYS;i++) {
    gw.sendSensorPresentation(RELAY_1+i, S_LIGHT);
  }

  // Fetch relay status
  for (int i=0; i<NUMBER_OF_RELAYS;i++) {
    // Make sure relays are off when starting up
    digitalWrite(RELAY_1+i, RELAY_OFF);
    // Then set relay pins in output mode
    pinMode(RELAY_1+i, OUTPUT);   
      
    // Request/wait for relay status
    gw.getStatus(RELAY_1+i, V_LIGHT);
    setRelayStatus(gw.getMessage()); // Wait here until status message arrive from gw
  }
  
}


/*
*  Example on how to asynchronously check for new messages from gw
*/
void loop() 
{
// LCD only code start
  lcd.setCursor(0,0); lcd.print("Arduino Reef Display");
  lcd.setCursor(0,1); lcd.print("Temp 1:");
  lcd.setCursor(0,2); lcd.print("Temp 2:");
  lcd.setCursor(0,3); lcd.print("Temp 3:");
  lcd.setCursor(18,1); lcd.print("F");
  lcd.setCursor(18,2); lcd.print("F");
  lcd.setCursor(18,3); lcd.print("F");
// LCD only code end
  
// Temp only code start
  sensors.requestTemperatures(); // Fetch temperatures from Dallas
  delay(100);
  for (int i=0; i<numSensors; i++) {
    // Fetch and round temperature to one decimal
    float temperature = static_cast<float>(static_cast<int>((metric?sensors.getTempCByIndex(i):sensors.getTempFByIndex(i)) * 10.)) / 10.;
    // Only send data if temperature has changed and no error
    if (lastTemperature[i] != temperature && temperature != -127.00) {
      // Send variable (using registered shortcut) to gw
      gw.sendVariable(i, V_TEMP, temperature, 1);
      lastTemperature[i]=temperature;
      lcd.setCursor(12,1); lcd.print(temperature);
      lcd.setCursor(12,2); lcd.print(temperature);
      lcd.setCursor(12,3); lcd.print(temperature);
    }
  }
// Temp only code end

// Relay only code start
  if (gw.messageAvailable()) {
    message_s message = gw.getMessage(); 
    setRelayStatus(message);
  }
  
  buttVal = digitalRead(BUTTON_PIN);      // read input value and store it in val
  
  if (buttVal == LOW && previousVal == HIGH) {
      // Start counter from when button was pressed
      pressTime = millis();
      switched = 0;
  }

  if (buttVal == LOW && switched == 0 && millis() - pressTime > debounce) {
    // Switch state if button pressed more than 200 msec
    state = state==1?0:1;
    gw.sendVariable(RELAY_CTRL, V_LIGHT, state); // We will receive an ack message
    switched = 1; // No more switches until button is released
    digitalWrite(RELAY_CTRL, state==1?0:1); // Change the state of the local relay
   }
   previousVal = buttVal;  
// Relay only code end
}

void setRelayStatus(message_s message) {
  if (message.header.messageType=M_SET_VARIABLE &&
      message.header.type==V_LIGHT) {
     int incomingRelayStatus = atoi(message.data);
     // Change relay state
     digitalWrite(message.header.childId, incomingRelayStatus==1?RELAY_ON:RELAY_OFF);
     // Write some debug info
     Serial.print("Incoming change for relay on pin:");
     Serial.print(message.header.childId);
     Serial.print(message.header.messageType == M_ACK_VARIABLE ? "Button":"Gateway");
     Serial.print(", New status: ");
     Serial.println(incomingRelayStatus);
   }
}
