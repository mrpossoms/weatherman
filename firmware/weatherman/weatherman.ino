#include <WiFi101.h>
#include <WiFiUdp.h>
#include <avr/power.h>

#include "Adafruit_Sensor.h"
#include "Adafruit_AM2320.h"

#include "AM2320.h"

const char SSID[] = "PossomsHouse";
const char PASS[] = "519sierra";

const uint16_t WEATHERMAN_PORT = 31337;

struct header_t {
  int32_t rssi;
  uint8_t measurement_count;
};

struct measurement_t {
  char sensor[4];
  char unit[4];
  float value;
};

Adafruit_AM2320 am2320 = Adafruit_AM2320();

void send_measurements(const measurement_t** meas_ptr, unsigned meas_num)
{
  int status = WL_IDLE_STATUS;

  for (unsigned tries = 3; status != WL_CONNECTED && tries > 0; tries--) 
  {
    Serial.print("Connecting, status: ");
    Serial.println(status);

    // Connect to WPA/WPA2 network. Change this line if using open or WEP network:
    status = WiFi.begin(SSID, PASS);
    // wait 10 seconds for connection:
    delay(1000);
  }

  { // send here
    WiFiUDP udp;

    Serial.print("Sending packet to: ");
    Serial.println(WiFi.gatewayIP());

    udp.beginPacket(IPAddress(192, 168, 1, 118), WEATHERMAN_PORT);

    header_t hdr = {
      .rssi = WiFi.RSSI(),
      .measurement_count = meas_num,
    };

    udp.write((const uint8_t*)&hdr, sizeof(hdr));

    for (unsigned i = 0; i < meas_num; i++)
    {
      Serial.write(meas_ptr[i]->sensor, sizeof(meas_ptr[i]->sensor));
      Serial.print(": ");
      Serial.print(meas_ptr[i]->value);
      Serial.write(meas_ptr[i]->unit, sizeof(meas_ptr[i]->unit));
      Serial.println();
      udp.write((const uint8_t*)(meas_ptr[i]), sizeof(measurement_t));
    }
    udp.endPacket();
    delay(1000);
  }

  WiFi.end();
};

void setup() {
  // put your setup code here, to run once:
  Serial.begin(9600);
  Serial.println("Weatherman");
  am2320.begin();
}

void loop() {
  measurement_t humidity = {
    "HUMI",
    "%",
    NAN,  
  };

  measurement_t temperature = {
    "TEMP",
    "C",
    NAN,  
  };

  measurement_t* measurements[] = { &humidity, &temperature };

  Serial.println("Polling devices");
  temperature.value = am2320.readTemperature();
  humidity.value = am2320.readHumidity();

  send_measurements(measurements, sizeof(measurements) / sizeof(measurement_t*));

  // TODO: determine if we can switch into a low power state here.
  Serial.println("Sleeping");
  Serial.println("-------------");
  delay(6000);
}
