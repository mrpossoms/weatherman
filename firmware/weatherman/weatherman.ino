#include <WiFi101.h>
#include <WiFiUdp.h>
#include <avr/power.h>

#include <avr/sleep.h>
#include <avr/wdt.h>
#include <avr/power.h>
#include <avr/interrupt.h>

#include "Adafruit_Sensor.h"
#include "Adafruit_AM2320.h"

#include "LowPower.h"

#include "AM2320.h"

// #define sleep_bod_disable() 										\
// do { 																\
//   unsigned char tempreg; 													\
//   __asm__ __volatile__("in %[tempreg], %[mcucr]" "\n\t" 			\
//                        "ori %[tempreg], %[bods_bodse]" "\n\t" 		\
//                        "out %[mcucr], %[tempreg]" "\n\t" 			\
//                        "andi %[tempreg], %[not_bodse]" "\n\t" 		\
//                        "out %[mcucr], %[tempreg]" 					\
//                        : [tempreg] "=&d" (tempreg) 					\
//                        : [mcucr] "I" _SFR_IO_ADDR(MCUCR), 			\
//                          [bods_bodse] "i" (_BV(BODS) | _BV(BODSE)), \
//                          [not_bodse] "i" (~_BV(BODSE))); 			\
// } while (0)

const char SSID[] = "PossomsHouse";
const char PASS[] = "519sierra";

const uint16_t WEATHERMAN_PORT = 31337;

// enum period_t
// {
// 	SLEEP_15MS,
// 	SLEEP_30MS,
// 	SLEEP_60MS,
// 	SLEEP_120MS,
// 	SLEEP_250MS,
// 	SLEEP_500MS,
// 	SLEEP_1S,
// 	SLEEP_2S,
// 	SLEEP_4S,
// 	SLEEP_8S,
// 	SLEEP_FOREVER
// };

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
    udp.begin(WEATHERMAN_PORT);

    // Serial.print("Sending packet to: ");
    // Serial.println(WiFi.gatewayIP());
    auto dest_addr = IPAddress(192, 168, 1, 255);
    // auto dest_addr = WiFi.gatewayIP();


    if (0 == udp.beginPacket(dest_addr, WEATHERMAN_PORT))
    {
      Serial.println("problem with the supplied IP address or port");
    }

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
    if (0 == udp.endPacket())
    {
      Serial.println("Error sending packet");
    }
    delay(1000);
    udp.stop();
    WiFi.end();
  }
};

void setup() {
  // put your setup code here, to run once:
 
  // // disable ADC
  // ADCSRA &= ~(1 << ADEN);
  // power_adc_disable();

  // // disable brown out detect
  // sleep_bod_disable();

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
  Serial.flush();

  //LowPower.powerDown(SLEEP_1S, ADC_OFF, BOD_OFF);
  delay(1000);
}
