// Example showing how to create an atuator for a servo.
// Connect red to +5V, Black or brown to GND and the last cable to Digital pin 3.
// The servo consumes much power and should probably have its own powersource.'
// The arduino might spontanally restart if too much power is used (happend
// to me when servo tried to pass the extreme positions = full load).

#include <Sensor.h>
#include <SPI.h>
#include <EEPROM.h>  
#include <Servo.h> 
#include <RF24.h>

#define SERVO_DIGITAL_OUT_PIN 3
#define SERVO_MIN 0 // Fine tune your servos min. 0-180
#define SERVO_MAX 210  // Fine tune your servos max. 0-180

// Set RADIO_ID to something unique in your sensor network (1-254)
// or set to AUTO if you want gw to assign a RADIO_ID for you.
#define RADIO_ID 6
#define CHILD_ID 10   // Id of the sensor child

Sensor gw(9, 10);
Servo myservo;  // create servo object to control a servo 
                // a maximum of eight servo objects can be created Sensor gw(9,10);

void setup() 
{ 
  Serial.begin(BAUD_RATE);
  gw.begin(RADIO_ID);
  myservo.attach(SERVO_DIGITAL_OUT_PIN);   

  // Register all sensors to gw (they will be created as child devices)
  gw.sendSensorPresentation(CHILD_ID, S_COVER);

  // Fetch servo state at startup
  gw.getStatus(CHILD_ID, V_DIMMER);
  setRelayStatus(gw.getMessage()); // Wait here until status message arrive from gw
} 

void loop() 
{ 
   if (gw.messageAvailable()) {
      // New messsage from gw
      message_s message = gw.getMessage(); 
      setRelayStatus(message);
  }
} 

void setRelayStatus(message_s message) {
  if (message.header.type==V_DIMMER) { // This could be M_ACK_VARIABLE or M_SET_VARIABLE
     int val = atoi(message.data);
     myservo.attach(SERVO_DIGITAL_OUT_PIN);   
     myservo.write(SERVO_MAX + (SERVO_MIN-SERVO_MAX)/100 * val); // sets the servo position 0-180
     // Write some debug info
     Serial.print("Servo changed. new state: ");
     Serial.println(val);
     delay(900);
     myservo.detach();
   } else if (message.header.type==V_UP) {
     Serial.println("Servo UP command");
     myservo.attach(SERVO_DIGITAL_OUT_PIN);   
     myservo.write(SERVO_MIN);
     gw.sendVariable(CHILD_ID, V_DIMMER, 100);
     delay(900);
     myservo.detach();
   } else if (message.header.type==V_DOWN) {
     Serial.println("Servo DOWN command");
     myservo.attach(SERVO_DIGITAL_OUT_PIN);   
     myservo.write(SERVO_MAX); 
     gw.sendVariable(CHILD_ID, V_DIMMER, 0);
     delay(900);
     myservo.detach();
   } else if (message.header.type==V_STOP) {
     Serial.println("Servo STOP command");
    // Servo is pretty fast.. don't think we have time to send a stop
     myservo.detach();
 }
}

