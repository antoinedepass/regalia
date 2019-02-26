#include <Wire.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_BNO055.h>
#include <utility/imumaths.h>
#include <SPI.h>
#include <WiFi101.h>
#include <WiFiUdp.h>
#include <OSCMessage.h>

/* This driver reads raw data from the BNO055

   Connections
   ===========
   Connect SCL to analog 5
   Connect SDA to analog 4
   Connect VDD to 3.3V DC
   Connect GROUND to common ground

   History
   =======
   2015/MAR/03  - First release (KTOWN)
*/

/* Set the delay between fresh samples */
#define BNO055_SAMPLERATE_DELAY_MS (1)

Adafruit_BNO055 bno = Adafruit_BNO055();

WiFiUDP Udp;
char ssid[] = "SM-A530W7257";     //  your network SSID (name)
char pass[] = "mugc5288";  // your network password
int status = WL_IDLE_STATUS;     // the Wifi radio's status
int destPort=12000;
byte destIp[]  = { 192, 168, 43, 54 };
// NETWORK: Static IP details...
IPAddress ip(192, 168, 43, 125);
IPAddress dns(192, 168, 43, 1);
IPAddress gateway(192, 168, 43, 1);
IPAddress subnet(255, 255, 255, 0);

/**************************************************************************/
/*
    Arduino setup function (automatically called at startup)
*/
/**************************************************************************/
void setup(void)
{
  //Configure pins for Adafruit ATWINC1500 Feather
  WiFi.setPins(8,7,4,2);
  WiFi.config(ip, dns, gateway, subnet);
  WiFi.begin(ssid, pass);
  
  /* Initialise the sensor */
  if(!bno.begin())
  {
    //while(1);
  }

  delay(1000);
  bno.setExtCrystalUse(true);
  Udp.begin(5555);

}

/**************************************************************************/
/*
    Arduino loop function, called once 'setup' is complete (your own code
    should go here)
*/
/**************************************************************************/
void loop(void)
{
  // Quaternion data
  imu::Quaternion quat = bno.getQuat();
  float quat_w = float(quat.w());
  float quat_x = float(quat.x());
  float quat_y = float(quat.y());
  float quat_z = float(quat.z());

  // Calculate Euler angles (roll, pitch, and yaw) from the unit quaternion.
  float roll = atan2(2.0f * (quat_w * quat_x + quat_y * quat_z), 1.0f - 2.0f * (quat_x * quat_x + quat_y * quat_y));
  float pitch = asin(2.0f * (quat_w * quat_y - quat_z * quat_x));
  float yaw = atan2(2.0f * (quat_w * quat_z + quat_x * quat_y), 1.0f - 2.0f * (quat_y * quat_y + quat_z * quat_z));

  int yaw_degrees_clock = int(map((yaw * 180) / PI, -180, 180, 0, 23));

  
  OSCMessage msag("/imu");
//  msag.add(yaw_degrees_clock);
  msag.add(yaw);
  msag.add(pitch);
  msag.add(roll);
  Udp.beginPacket(destIp, destPort);
    msag.send(Udp); // send the bytes to the SLIP stream
  Udp.endPacket(); // mark the end of the OSC Packet
  msag.empty(); // free space occupied by message
 
  delay(BNO055_SAMPLERATE_DELAY_MS);
  

}
