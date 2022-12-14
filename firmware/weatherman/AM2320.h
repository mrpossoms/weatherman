#include <Wire.h>

namespace AM2320
{
  const uint8_t addr = 0x5C;

  static size_t read_reg(uint8_t reg, uint8_t* dst, uint8_t num)
  {
    Wire.setWireTimeout(100000, false);
    Wire.beginTransmission(addr);
    Wire.write(0x03); // will read from dev
    Wire.write(reg); // will read dev id
    Wire.write(num); // which is 1 byte
    Wire.endTransmission(true); // send STOP 

    // auto bytes = Wire.requestFrom(addr, num); // request the 1 byte for the id and stop

    auto reply_code = Wire.read();
    auto bytes = Wire.read();
    

    // if (bytes != num) { return bytes; }

    for (unsigned i = 0; i < bytes; i++)
    {
      dst[i] = Wire.read();
    }

    uint8_t crc[2] = { Wire.read(), Wire.read() };

    return bytes;
  }

  static bool get_temp_c(float* temp_c)
  {
    uint16_t temp = 1;
    auto exp_size = sizeof(temp);
    if (exp_size != read_reg(0x02, (char*)&temp, exp_size)) { return false; }

    Serial.println(temp);

    if (temp == 0xFFFF)
      return false;

    if (temp & 0x8000)
    {
      *temp_c = -(int16_t)(temp & 0x7fff);
    } else 
    {
      *temp_c = (int16_t)temp;
    }

    *temp_c /= 10.0;

    return true;
  }

  static bool get_humidity_per(float* humidity_per)
  {
    uint16_t humi = 0;
    auto exp_size = sizeof(humi);
    if (exp_size != read_reg(0x00, (char*)&humi, exp_size)) { return false; }

    *humidity_per = humi / 10.0;

    return true;
  }

  static bool check()
  {
    uint8_t dev_id;

    return 1 == read_reg(0x0B, &dev_id, sizeof(dev_id)) && dev_id != 0; // Not sure what this should actually contain.
  }

} // namespace AM2320

