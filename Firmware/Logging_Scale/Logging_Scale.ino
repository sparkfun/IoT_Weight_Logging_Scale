/****************************************************************************
Logging_Scale.ino
Arduino code for IoT Scale
Mike Hord @ SparkFun Electronics
20 Feb 2018
https://github.com/sparkfun/Logging_Scale

This simple example (derived from the BasicHttpClient example that comes
with the ESP32 Arduino support package) connects to WiFi then waits for a
weight measurement on the digital scale to be completed, then posts that
measurement to a flask app running on the web.

Resources:
HX711_ADC library by Olav Kallhovd (available through library manager)

Development environment specifics:
Arduino 1.8.5
ESP32 Arduino Support (https://github.com/espressif/arduino-esp32)

This code is beerware; if you see me (or any other SparkFun employee) at the 
local, and you've found our code helpful, please buy us a round!
****************************************************************************/
#include <Arduino.h>
#include <WiFi.h>
#include <WiFiMulti.h>
#include <HTTPClient.h>
#include <HX711_ADC.h>

WiFiMulti wifiMulti;
HX711_ADC LoadCell(5,4);   //HX711 constructor (dout pin, sck pin)
long scaleTick, httpTick;
HardwareSerial Serial2(2); // Enable the third hw serial port on the ESP32

void setup() 
{
  Serial.begin(115200);
  Serial2.begin(9600);
  
  Serial.println();
  Serial.println();
  Serial.println();

  // Give background processes a few seconds to complete
  for(uint8_t t = 4; t > 0; t--) 
  {
    Serial.printf("[SETUP] WAIT %d...\n", t);
    Serial.flush();
    delay(1000);
  }
  
  wifiMulti.addAP("ssid_goes_here", "password_goes_here");

  // Set up the HX711 library
  LoadCell.begin();
  LoadCell.start(10); // Start up the HX711 library with 10ms delay
  LoadCell.setCalFactor(10920); // user set calibration factor (float)
  Serial.println("Startup + tare is complete");
}

void loop() 
{
  static float history[4];
  static float ave = 0;
  static bool stable = false;
  static bool webUpdated = false;
  static float weightData = 0.0;

  // Update the website with weight data IF the weight data is stable AND the
  //  website hasn't been updated yet.
  if (stable && !webUpdated)
  {
    // Only try if the wireless network is connected
    if((wifiMulti.run() == WL_CONNECTED)) 
    {
      HTTPClient http;
  
      Serial.print("[HTTP] begin...\n");
      Serial.println(weightData);
      // Replace 0.0.0.0 in address with your server's IP address
      String address = "http://0.0.0.0:5000/post_weight/";
      // Create a string with one decimal point of the average of the weights
      //  collected
      String weight = String(weightData, 1);
      // cat the two together
      String fullAddress = String(address + weight);
      // Connect to the server with the address you've created
      http.begin(fullAddress);
  
      Serial.print("[HTTP] GET...\n");
      
      // start connection and send HTTP header
      int httpCode = http.GET();
      // httpCode will be negative on error
      if(httpCode > 0) 
      {
        // HTTP header has been send and Server response header has been handled
        Serial.printf("[HTTP] GET... code: %d\n", httpCode);
  
        // file found at server, response 200
        if(httpCode == HTTP_CODE_OK) 
        {
          // clear the stable flag as the data is no longer valid
          stable = false;
          // Set the webUpdated flag as we've successfully updated the website
          webUpdated = true;
          Serial.println("Web updated");
          // Dim the LED display for 500ms, then turn it back on
          Serial2.write(0x7a);
          Serial2.write(0);
          delay(500);
          Serial2.write(0x7a);
          Serial2.write(255);
        }
      } 
      else 
      {
        Serial.printf("[HTTP] GET... failed, error: %s\n", http.errorToString(httpCode).c_str());
      }
  
      http.end();
    }
  }

  // Check the weight measurement every 250ms
  if (millis() - scaleTick > 250)
  {
    scaleTick = millis();
    // Read from the HX711. Must do this before attempting to use data!
    LoadCell.update();
    // We take the abs of the data we get from the load cell because different
    //  scales may behave differently, yielding negative numbers instead of 
    //  positive as the weight increases. This can be handled in hardware by
    //  switching the A+ and A- wires, OR we can do this and never worry about it.
    weightData = abs(LoadCell.getData());
    // Calculate our running average
    history[3] = history[2];
    history[2] = history[1];
    history[1] = history[0];
    history[0] = weightData;
    ave = (history[0] + history[1] + history[2] + history[3])/4;

    // IF the average differs from the current by less than 0.3lbs AND
    //  the average weight is greater than 30 pounds AND
    //  we haven't recently updated the website, set the stable flag so
    //  we know that the weight is stable and can be reported to the web
    if ((abs(ave - weightData) < 0.1) && 
        (ave > 30) && 
        !webUpdated)
    {
      stable = true;
    }
    
    // IF we've updated the website AND
    //  the average weight is close to zero, clear the website updated flag
    //  so we are ready for the next weight reading
    if (webUpdated && ave < 1)
    {
      webUpdated = false;
    }
    
    Serial.print("Load_cell output val: ");
    Serial.println(weightData);

    // Create a string which is the integer value of the weight times 10,
    //  to remove the decimal point.
    String weight = String(int(weightData*10));
    Serial2.write(0x76); // Clear the display
    Serial2.print(weight); // Write out the weight value to the display
    
    // Identify which decimal point to set, and set it.
    int shiftBy = 5-weight.length();
    int decimalPoint = 0x08>>(shiftBy);
    Serial2.write(0x77);
    Serial2.write(decimalPoint & 0x0F);
  }
}

