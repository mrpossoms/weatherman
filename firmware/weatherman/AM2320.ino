#include <Wire.h>

namespace AM2320
{
  const uint8_t addr = 0xB8;
  const uint8_t dev_id = 

  static size_t read_registers(uint8_t reg, uint8_t addr, uint8_t num, uint8_t* dst)
  {

  }

  static bool check()
  {
    Wire.beginTransmission(addr);
    Wire.write(0x03); // will read from dev
    Wire.write(0x0B); // will read dev id
    Wire.write(1); // which is 1 byte
    Wire.endTransmission(true); // send STOP 

    auto bytes = Wire.requestFrom(addr, 1, true); // request the 1 byte for the id and stop

    if (bytes != 1) { return false; }

    return Wire.read() != 0; // Not sure what this should actually contain.
  }

} // namespace AM2320

