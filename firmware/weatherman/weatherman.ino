#include <WiFi101.h>
#include <WiFiUdp.h>

const char SSID[] = "PossomsHouse";
const char PASS[] = "519sierra";

const uint16_t WEATHERMAN_PORT = 31337;

struct header_t {
  int32_t rssi;
  uint8_t measurement_count;
};

struct measurement_t {
  float value;
};

void send_measurements(const measurement_t* meas_ptr, unsigned meas_num)
{
  int status = WL_IDLE_STATUS;

  for (unsigned tries = 3; status != WL_CONNECTED && tries > 0; tries--) 
  {
    // Connect to WPA/WPA2 network. Change this line if using open or WEP network:
    status = WiFi.begin(SSID, PASS);
    // wait 10 seconds for connection:
    delay(10000);
  }

  { // send here
    WiFiUDP udp;

    udp.beginPacket(WiFi.gatewayIP(), WEATHERMAN_PORT);

    header_t hdr = {
      .rssi = WiFi.RSSI(),
      .measurement_count = meas_num,
    };

    udp.write((const uint8_t*)&hdr, sizeof(hdr));

    for (unsigned i = 0; i < meas_num; i++)
    {
      udp.write((const uint8_t*)(meas_ptr + i), sizeof(meas_ptr[i]));
    }
    udp.endPacket();
  }

  WiFi.end();
};

void setup() {
  // put your setup code here, to run once:

}

void loop() {
  // put your main code here, to run repeatedly:

}
