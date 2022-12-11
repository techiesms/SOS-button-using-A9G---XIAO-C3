#include "WiFi.h"

#define SOS D3
#define SLEEP_PIN D2 // Make this pin HIGH to make A9G board to go to sleep mode


boolean stringComplete = false;
String inputString = "";
String fromGSM = "";
int c = 0;
String SOS_NUM = "+91xxxxxxxxxx";

int SOS_Time = 5; // Press the button 5 sec

bool CALL_END = 1;
char* response = " ";
String res = "";
void setup()
{

  // Making Radio OFF for power saving
  WiFi.mode(WIFI_OFF);  // WiFi OFF
  btStop();   // Bluetooth OFF

  pinMode(SOS, INPUT_PULLUP);

  pinMode(SLEEP_PIN, OUTPUT);

  Serial.begin(115200); // For Serial Monitor
  Serial1.begin(115200, SERIAL_8N1, D0, D1); // For XIAO C3 Board

  // Waiting for A9G to setup everything for 20 sec
  delay(20000);


  digitalWrite(SLEEP_PIN, LOW); // Sleep Mode OFF

  Serial1.println("AT");               // Just Checking
  delay(1000);

  Serial1.println("AT+GPS = 1");      // Turning ON GPS
  delay(1000);

  Serial1.println("AT+GPSLP = 2");      // GPS low power
  delay(1000);

  Serial1.println("AT+SLEEP = 1");    // Configuring Sleep Mode to 1
  delay(1000);

  digitalWrite(SLEEP_PIN, HIGH); // Sleep Mode ON


}

void loop()
{
  //listen from GSM Module
  if (Serial1.available())
  {
    char inChar = Serial1.read();

    if (inChar == '\n') {

      //check the state
      if (fromGSM == "OK\r") {
        Serial.println("---------IT WORKS-------");
      }
      else if (fromGSM == "RING\r") {
        digitalWrite(SLEEP_PIN, LOW); // Sleep Mode OFF
        Serial.println("---------ITS RINGING-------");
        Serial1.println("ATA");
      }
      else if (fromGSM == "ERROR\r") {
        Serial.println("---------IT DOESNT WORK-------");
      }

      else if (fromGSM == "NO CARRIER\r") {
        Serial.println("---------CALL ENDS-------");
        CALL_END = 1;
        digitalWrite(SLEEP_PIN, HIGH);// Sleep Mode ON
      }

      //write the actual response
      Serial.println(fromGSM);
      //clear the buffer
      fromGSM = "";

    } else {
      fromGSM += inChar;
    }
    delay(20);
  }

  // read from port 0, send to port 1:
  if (Serial.available()) {
    int inByte = Serial.read();
    Serial1.write(inByte);
  }

  // When SOS button is pressed
  if (digitalRead(SOS) == LOW && CALL_END == 1)
  {
    Serial.print("Calling In.."); // Waiting for 5 sec
    for (c = 0; c < SOS_Time; c++)
    {
      Serial.println((SOS_Time - c));
      delay(1000);
      if (digitalRead(SOS) == HIGH)
        break;
    }
    if (c == 5)
    {
      //-------------------------------------  Getting Location and making Google Maps link of it

      digitalWrite(SLEEP_PIN, LOW);
      delay(1000);
      Serial1.println("AT+LOCATION = 2");
      Serial.println("AT+LOCATION = 2");

      while (!Serial1.available());
      while (Serial1.available())
      {
        char add = Serial1.read();
        res = res + add;
        delay(1);
      }

      res = res.substring(17, 38);
      response = &res[0];

      Serial.print("Recevied Data - "); Serial.println(response); // printin the String in lower character form
      Serial.println("\n");

      if (strstr(response, "GPS NOT"))
      {
        Serial.println("No Location data");
      }
      else
      {

        int i = 0;
        while (response[i] != ',')
          i++;

        String location = (String)response;
        String lat = location.substring(2, i);
        String longi = location.substring(i + 1);
        Serial.println(lat);
        Serial.println(longi);

        String Gmaps_link = ( "http://maps.google.com/maps?q=" + lat + "+" + longi); //http://maps.google.com/maps?q=38.9419+-78.3020


        //------------------------------------- Sending SMS with Google Maps Link with our Location
        Serial1.println("AT+CMGF=1");
        delay(1000);
        Serial1.println("AT+CMGS=\"" + SOS_NUM + "\"\r");
        delay(1000);

        Serial1.println ("I'm here " + Gmaps_link);
        delay(1000);
        Serial1.println((char)26);
        delay(1000);
      }
      response = "";
      res = "";

      //------------------------------------- Calling on that same number after sending SMS
      Serial.println("Calling Now");
      Serial1.println("ATD" + SOS_NUM);
      CALL_END = 0;
    }
  }

  //only write a full message to the GSM module
  if (stringComplete) {
    Serial1.print(inputString);
    inputString = "";
    stringComplete = false;
  }



}
