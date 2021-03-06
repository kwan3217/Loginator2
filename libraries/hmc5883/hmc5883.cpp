#include "hmc5883.h"

// Sets the configuration and mode registers such that the part is continuously
// generating measurements. This function should be called at the beginning of 
// the program
void HMC5883::begin() {
  //Set it to single-shot mode
  port.beginTransmission(ADDRESS);
  //Address the configuration register A
  port.write(0x00);  
  //Write config register A
  port.write(3<<5 | 6<<2 | 0<<0); //MA - 0b11  = 8 samples average
                                  //DO - 0b110 = 75Hz measurment rate
                                  //MS - 0b00  = normal measurement mode
  //Write config register B (auto register address increment in HMC5883)
  port.write(1<<5 );              //GN - 0b001 = +-1.3Ga (1090DN/Ga)
  //Write mode register
  port.write(0<<0 );              //MD - 0b00  = continuous measurement mode
  port.endTransmission();
}

// Read 1 byte from the BMP085 at 'address'
int8_t HMC5883::read(uint8_t address) {
  port.beginTransmission(ADDRESS);
  port.write(address);
  port.endTransmission();
  
  port.requestFrom(ADDRESS, 1);
  return port.read();
}

// Read 2 bytes from the BMP085
// First byte will be from 'address'
// Second byte will be from 'address'+1
int16_t HMC5883::read16(uint8_t address) {
  uint8_t msb, lsb;
  
  port.beginTransmission(ADDRESS);
  port.write(address);
  port.endTransmission();
  
  port.requestFrom(ADDRESS, 2);
  msb = port.read();
  lsb = port.read();
  
  return (int16_t) msb<<8 | lsb;
}

void HMC5883::read(int16_t& x, int16_t& z, int16_t& y) {
  x=read16(3);
  z=read16(5);
  y=read16(7);
}

void HMC5883::read(int16_t& x, int16_t& y, int16_t& z, uint8_t& status) {
  port.beginTransmission(ADDRESS);
  port.write(3);
  port.endTransmission();
  
  port.requestFrom(ADDRESS, 7);
  uint8_t msb = port.read();
  uint8_t lsb = port.read();
  x=(int16_t) msb<<8 | lsb;
  msb = port.read();
  lsb = port.read();
  z=(int16_t) msb<<8 | lsb;
  msb = port.read();
  lsb = port.read();
  y=(int16_t) msb<<8 | lsb;
  status=port.read();
}

void HMC5883::whoami(char* id) {
  id[0]=read(10);
  id[1]=read(11);
  id[2]=read(12);
  id[3]=0;
}

bool HMC5883::fillConfig(Packet& ccsds) {
  if(!ccsds.fill(read( 0),"ConfigA")) return false;
  if(!ccsds.fill(read( 1),"ConfigB")) return false;
  if(!ccsds.fill(read( 2),"Mode"   )) return false;
  if(!ccsds.fill(read( 9),"Status" )) return false;
  if(!ccsds.fill(read(10),"IdA"    )) return false;
  if(!ccsds.fill(read(11),"IdB"    )) return false;
  if(!ccsds.fill(read(12),"IdC"    )) return false;
  return true;
}



