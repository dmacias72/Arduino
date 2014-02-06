// Example sketch för a "light switch" where you can control light or something 
// else from both vera and a local physical button (connected between digital
// pin 3 and +5V).

#include <Relay.h>
#include <SPI.h>
#include <EEPROM.h>  
#include <RF24.h>

// Set RADIO_ID to something unique in your sensor network (1-254)
// or set to AUTO if you want gw to assign a RADIO_ID for you.
//#define RADIO_ID AUTO
#define RADIO_ID 1

#define RELAY_PIN  4  // Arduino Digital I/O pin number for relay 
#define BUTTON_PIN  3  // Arduino Digital I/O pin number for button (connect to gnd, no resistor needed)

#define CHILD_ID 1   // Id of the sensor child

#define RELAY_ON 0
#define RELAY_OFF 1

int buttVal;
int previousVal;
int state;
int switched = 0;
long pressTime = 0;    // the last time the button pin was pressed
long debounce = 200;   // the debounce time, increase if the output flickers

Relay gw(9,10);

void setup()  
{  
  Serial.begin(BAUD_RATE);  // Used to write debug info

  gw.begin(RADIO_ID);

  // Set buttonPin as Input
  pinMode(BUTTON_PIN, INPUT);
  digitalWrite(BUTTON_PIN,HIGH); //Turn On internal Pull-up resistor
  buttVal = digitalRead(BUTTON_PIN); //read initial state

  // Register all sensors to gw (they will be created as child devices)
  gw.sendSensorPresentation(CHILD_ID, S_LIGHT);

  // Make sure relays are off when starting up
  digitalWrite(RELAY_PIN, RELAY_OFF);
  // Then set relay pins in output mode
  pinMode(RELAY_PIN, OUTPUT);   
      
  // Request/wait for relay status
  gw.getStatus(CHILD_ID, V_LIGHT);
  setRelayStatus(gw.getMessage()); // Wait here until status message arrive from gw
  
}


/*
*  Example on how to asynchronously check for new messages from gw
*/

void loop() 
{
  if (gw.messageAvailable()) {
    // ot new messsage from gw
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
     
     // Debug the action of the switch
     Serial.println("Button pressed");
     
     gw.sendVariable(CHILD_ID, V_LIGHT, state); // We will receive an ack message
     switched = 1; // No more switches until button is released
     
    // Added this line to change the state of the local relay
     digitalWrite(RELAY_PIN, state==1?0:1);
    }
    previousVal = buttVal;
} 
 
  
  

void setRelayStatus(message_s message) {
  if (message.header.type==V_LIGHT) { // This could be M_ACK_VARIABLE or M_SET_VARIABLE
     state = atoi(message.data);
     // Change relay state
     digitalWrite(RELAY_PIN, state==1?RELAY_ON:RELAY_OFF);
     // Write some debug info
     Serial.print(message.header.messageType == M_ACK_VARIABLE ? "Button":"Gateway");
     Serial.print(" change. New state: ");
     Serial.println(state == 1 ?"on":"off" );
   }
}
