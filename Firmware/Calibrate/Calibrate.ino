/****************************************************************************
 * Calibrate.ino
 * ESP32 Scale Calibration Routine
 * Mike Hord @ SparkFun Electronics
 * 14 March 2018
 * https://github.com/sparkfun/Logging_Scale
 * 
 * Allows user to calculate the proper scaling value for the raw data from an
 * HX711. Use with ESP32 for Logging Scale project with serial 7-seg display
 * 
 * Resources:
 * HX711_ADC library
 * 
 * Development environment specifics:
 * Arduino 1.8.5
 * SparkFun ESP32 Thing
 * 
 * This code is beerware; if you see me (or any other SparkFun employee) at the
 * local, and you've found our code helpful, please buy us a round!
 * ****************************************************************************/

//-------------------------------------------------------------------------------------
// HX711_ADC.h
// Arduino master library for HX711 24-Bit Analog-to-Digital Converter for Weigh Scales
// Olav Kallhovd sept2017
// Tested with      : HX711 asian module on channel A and YZC-133 3kg load cell
// Tested with MCU  : Arduino Nano
//-------------------------------------------------------------------------------------
/* This is an example sketch on how to find correct calibration factor for your HX711:
   - Power up the scale and open Arduino serial terminal
   - After stabelizing and tare is complete, put a known weight on the load cell
   - Observe values on serial terminal
   - Adjust the calibration factor until output value is same as your known weight:
      - Sending 'l' from the serial terminal decrease factor by 1.0
      - Sending 'L' from the serial terminal decrease factor by 10.0
      - Sending 'h' from the serial terminal increase factor by 1.0
      - Sending 'H' from the serial terminal increase factor by 10.0
      - Sending 't' from the serial terminal call tare function
   - Observe and note the value of the new calibration factor
   - Use this new calibration factor in your sketch
*/

#include <HX711_ADC.h>

//HX711 constructor (dout pin, sck pin)
HX711_ADC LoadCell(5, 4);
HardwareSerial Serial2(2); // Enable 3rd serial port on ESP32

long t;

void setup() {
  Serial.begin(115200);
  Serial2.begin(9600);
  Serial.println("Wait...");
  LoadCell.begin();
  long stabilisingtime = 2000; // tare preciscion can be improved by adding a few seconds of stabilising time
  LoadCell.start(stabilisingtime);
  LoadCell.setCalFactor(10000.0); // user set calibration factor (float)
  Serial.println("Startup + tare is complete");
}

void loop() {
  //update() should be called at least as often as HX711 sample rate; >10Hz@10SPS, >80Hz@80SPS
  //longer delay in scetch will reduce effective sample rate (be carefull with delay() in loop)
  LoadCell.update();

  //get smoothed value from data set + current calibration factor
  if (millis() > t + 250) {
    float i = fabs(LoadCell.getData());
    float v = LoadCell.getCalFactor();
    Serial.print("Load_cell output val: ");
    Serial.print(i);
    Serial.print("      Load_cell calFactor: ");
    Serial.println(v);
    
    // Create a string which is the integer value of the weight times 10,
    //  to remove the decimal point.
    String weight = String(int(i*10));
    Serial2.write(0x76); // Clear the display
    Serial2.print(weight); // Write out the weight value to the display
    
    // Identify which decimal point to set, and set it.
    int shiftBy = 5-weight.length();
    int decimalPoint = 0x08>>(shiftBy);
    Serial2.write(0x77);
    Serial2.write(decimalPoint & 0x0F);
    
    t = millis();
  }

  //receive from serial terminal
  if (Serial.available() > 0) {
    float i;
    char inByte = Serial.read();
    if (inByte == 'l') i = -1.0;
    else if (inByte == 'L') i = -10.0;
    else if (inByte == 'h') i = 1.0;
    else if (inByte == 'H') i = 10.0;
    else if (inByte == 't') LoadCell.tareNoDelay();
    if (i != 't') {
      float v = LoadCell.getCalFactor() + i;
      LoadCell.setCalFactor(v);
    }
  }

  //check if last tare operation is complete
  if (LoadCell.getTareStatus() == true) {
    Serial.println("Tare complete");
  }

}
