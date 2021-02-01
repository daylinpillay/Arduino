#include <WiServer.h>
// Wireless configuration parameters -
unsigned char local_ip[]= {xxx,xxx,xxx,xxx};   // IP address of WiShield (modify it)
unsigned char gateway_ip[]= {xxx,xxx,xxx,xxx};   // router or gateway IP address (modify it)
unsigned char subnet_mask[]= {255,255,255,0}; // subnet mask for the local network
//char ssid[] = {"Arduino"};   // max 32 bytes  (modify it)
char ssid[] = {"SSIDname"};
unsigned char security_type = 0;               // 0 - open; 1 - WEP; 2 - WPA; 3 - WPA2   (modify it)
// WPA/WPA2 passphrase
const prog_char security_passphrase[] PROGMEM = {"Wifipassword"}; // max 64 characters

// WEP 128-bit keys
prog_uchar wep_keys[] PROGMEM = {
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // Key 0
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // Key 1
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // Key 2
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 // Key 3
};

// setup the wireless mode; infrastructure - connect to AP; adhoc - connect to another WiFi device
#define WIRELESS_MODE_INFRA 1
#define WIRELESS_MODE_ADHOC 2
unsigned char wireless_mode = WIRELESS_MODE_INFRA;
unsigned char ssid_len;
unsigned char security_passphrase_len;
// End of wireless configuration parameters ----------------------------------------

uint8 ip[] = {213,186,33,19}; //IP address of api.pushingbox.com
char hostName[] = "api.pushingbox.com"; // api.pushingbox.com
char url[]="/pushingbox?devid=vA91B3703541BE92";

GETrequest getEverDevice(ip, 80, hostName, url);
int pirPin = 3;    //the digital pin connected to the PIR sensor's output
int ledPin = 7;

/////////////////////////////
//VARS
//the time we give the sensor to calibrate (10-60 secs according to the datasheet)
int calibrationTime = 30;
//the time when the sensor outputs a low impulse
long unsigned int lowIn;

//the amount of milliseconds the sensor has to be low
//before we assume all motion has stopped
long unsigned int pause = 5000;

boolean lockLow = true;
boolean takeLowTime;
boolean detected = false;
void setup()
{
   Serial.begin(57600);
   pinMode(pirPin, INPUT);

  digitalWrite(pirPin, LOW);
  digitalWrite(ledPin,LOW);
  Serial.print("calibrating sensor ");
    for(int i = 0; i < calibrationTime; i++){
      Serial.print(".");
      delay(1000);
      }
    Serial.println(" done");
    Serial.println("SENSOR ACTIVE");
    Serial.println("WiFi init...");
    WiServer.init(NULL);
    WiServer.enableVerboseMode(true);
    getEverDevice.setReturnFunc(printData);
    delay(50);
}

void loop()
{
  if(PIR_detected() && (detected))
  {  // PIR : HIGH
      getEverDevice.submit();
      Serial.println("emailing");
   }
   // Run WiServer
   WiServer.server_task();
    delay(1000);
}

void printData(char* data, int len) {
  while (len-- > 0) {
    Serial.print(*(data++));
  }
}

boolean PIR_detected()
{
  boolean bPIR;

     if(digitalRead(pirPin) == HIGH){

       detected = false;
       if(lockLow){
         //makes sure we wait for a transition to LOW before any further output is made:
         lockLow = false;
         pinMode(ledPin, OUTPUT);
         digitalWrite(ledPin, HIGH);   //the led visualizes the sensors output pin state
         Serial.println("---");
         Serial.print("motion detected at ");
         Serial.print(millis()/1000);
         Serial.println(" sec");

         detected = true;
         delay(50);
         }
         takeLowTime = true;

         bPIR = true;

       }

     if(digitalRead(pirPin) == LOW){
       digitalWrite(ledPin, LOW);  //the led visualizes the sensors output pin state

       if(takeLowTime){
        lowIn = millis();          //save the time of the transition from high to LOW
        takeLowTime = false;       //make sure this is only done at the start of a LOW phase
        }
       //if the sensor is low for more than the given pause,
       //we assume that no more motion is going to happen
       if(!lockLow && millis() - lowIn > pause){
           //makes sure this block of code is only executed again after
           //a new motion sequence has been detected
           lockLow = true;
           Serial.print("motion ended at ");      //output
           Serial.print((millis() - pause)/1000);
           Serial.println(" sec");
           delay(50);
           }

           bPIR = false;
       }
    return bPIR;
}
