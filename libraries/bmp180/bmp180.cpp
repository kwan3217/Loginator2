#include "bmp180.h"
#include "DirectTask.h"
#include "Time.h"

#undef BMP180_DEBUG 

BMP180::BMP180(TwoWire &Lport):port(Lport),OSS(3) {}

// Stores all of the bmp085's calibration values into global variables
// Calibration values are required to calculate temp and pressure
// This function should be called at the beginning of the program
bool BMP180::begin(unsigned int Ltimer_ch) {
  timer_ch=Ltimer_ch;
  port.beginTransmission(ADDRESS);
  port.write(0xE0);
  port.write(0x6B);
  port.endTransmission();
  delay(25);
  return readCalibration();
}

bool BMP180::readCalibration() {
  ac1 = read_int16(0xAA);
  ac2 = read_int16(0xAC);
  ac3 = read_int16(0xAE);
  ac4 = read_int16(0xB0);
  ac5 = read_int16(0xB2);
  ac6 = read_int16(0xB4);
  b1 = read_int16(0xB6);
  b2 = read_int16(0xB8);
  mb = read_int16(0xBA);
  mc = read_int16(0xBC);
  md = read_int16(0xBE);
  if (ac1==0 || ac1==0xFFF) return false;
  if (ac2==0 || ac2==0xFFF || ac2==ac1) return false;
  return true;
}

void BMP180::printCalibration(Stream *Louf) {
  ouf=Louf;
  ouf->println("BMP180 calibration constants");
  ouf->print("ac1: ");
  ouf->println(ac1,DEC);
  ouf->print("ac2: ");
  ouf->println(ac2,DEC);
  ouf->print("ac3: ");
  ouf->println(ac3,DEC);
  ouf->print("ac4: ");
  ouf->println((unsigned int)ac4,DEC);
  ouf->print("ac5: ");
  ouf->println((unsigned int)ac5,DEC);
  ouf->print("ac6: ");
  ouf->println((unsigned int)ac6,DEC);
  ouf->print("b1: ");
  ouf->println(b1,DEC);
  ouf->print("b2: ");
  ouf->println(b2,DEC);
  ouf->print("mb: ");
  ouf->println(mb,DEC);
  ouf->print("mc: ");
  ouf->println(mc,DEC);
  ouf->print("md: ");
  ouf->println(md,DEC);
}

void BMP180::fillCalibration(Packet& pkt) {
  pkt.filli16(ac1,"ac1");
  pkt.filli16(ac2,"ac2");
  pkt.filli16(ac3,"ac3");
  pkt.fillu16(ac4,"ac4");
  pkt.fillu16(ac5,"ac5");
  pkt.fillu16(ac6,"ac6");
  pkt.filli16(b1,"b1");
  pkt.filli16(b2,"b2");
  pkt.filli16(mb,"mb");
  pkt.filli16(mc,"mc");
  pkt.filli16(md,"md");
}  
 
// Calculate temperature given ut.
// Value returned will be in units of 0.1 deg C
int16_t BMP180::getTemperature(uint16_t ut) {
  int32_t x1, x2;
  int16_t result;
  
  print("x1t: ",x1 = (((int32_t)ut - (int32_t)ac6)*(int32_t)ac5) >> 15);
  print("x2t: ",x2 = ((int32_t)mc << 11)/(x1 + md));
  print("b5:  ",b5 = x1 + x2);
  print("getT:",result=((b5 + 8)>>4));

  return result;  
}

// Calculate pressure given up
// calibration values must be known
// b5 is also required so getTemperature(...) must be called first.
// Value returned will be pressure in units of Pa.
int32_t BMP180::getPressure(uint32_t up) {
  int32_t x1, x2, x3, b3, b6, p;
  uint32_t b4, b7;
  
  print("b6: ",b6 = b5 - 4000);
  // Calculate B3
  print("x1a: ",x1 = (b2 * ((b6 * b6)>>12))>>11);
  print("x2a: ",x2 = (ac2 * b6)>>11);
  print("x3a: ",x3 = x1 + x2);
  print("b3:  ",b3 = (((((int32_t)ac1)*4 + x3)<<OSS) + 2)>>2);
  
  // Calculate B4
  print("x1b: ",x1 = (ac3 * b6)>>13);
  print("x2b: ",x2 = (b1 * ((b6 * b6)>>12))>>16);
  print("x3b: ",x3 = ((x1 + x2) + 2)>>2);
  print("b4:  ",b4 = (ac4 * (uint32_t)(x3 + 32768))>>15);
  
  print("b7:  ",b7 = ((uint32_t)(up - b3) * (50000>>OSS)));
  if (b7 < 0x80000000)
    print("pa:   ",p = (b7<<1)/b4);
  else
    print("pb:   ",p = (b7/b4)<<1);
    
  print("x1c: ",x1= (p>>8) * (p>>8));
  print("x1d: ",x1 = (x1 * 3038)>>16);
  print("x2:  ",x2 = (-7357 * p)>>16);
  print("pc:  ",p += (x1 + x2 + 3791)>>4);
  
  return p;
}

// Read 1 byte from the BMP085 at 'address'
int8_t BMP180::read(uint8_t address) {
  port.beginTransmission(ADDRESS);
  port.write(address);
  port.endTransmission();
  
  port.requestFrom(ADDRESS, 1);
  return (int8_t)port.read();
}

// Read 2 bytes from the BMP085
// First byte will be from 'address'
// Second byte will be from 'address'+1
int16_t BMP180::read_int16(uint8_t address) {
  uint8_t msb, lsb;
  
  port.beginTransmission(ADDRESS);
  port.write(address);
  port.endTransmission();
  
  port.requestFrom(ADDRESS, 2);
  msb = port.read();
  lsb = port.read();
  
  return (int16_t) msb<<8 | lsb;
}

void BMP180::finishTempTask(void* Lthis) {
  ((BMP180*)Lthis)->finishTemp();
}

void BMP180::finishPresTask(void* Lthis) {
  ((BMP180*)Lthis)->finishPres();
}

void BMP180::startMeasurementCore() {
  // Write 0x2E into Register 0xF4
  // This requests a temperature reading
  port.beginTransmission(ADDRESS);
  port.write(0xF4);//Address the ctrl_meas register (0xF4)
  port.write(0x2E);//Write 0010_1110 to register. This sets the SCO bit to 1, the meas bits to 01110, and the OSS bits to 00.
  port.endTransmission();
}

void BMP180::startMeasurement() {
  startMeasurementCore();
  start=true;
  ready=false;
  //Required wait is 4.5ms, this asks for 5.0ms (500us margin)
  directTaskManager.schedule(timer_ch,5,0,&finishTempTask,this);
}

void BMP180::finishTempCore() {
  // Read two bytes from registers 0xF6 and 0xF7
  UT = read_int16(0xF6);

  //Start pressure measurement 
  // Write 0x34+(OSS<<6) into register 0xF4
  // Request a pressure reading w/ oversampling setting
  port.beginTransmission(ADDRESS);
  port.write(0xF4); //Address the ctrl_meas register (0xF4)
  port.write(0x34 | (OSS<<6)); //Write OS11_0100 to register. This sets the SCO bit to 1, the meas bits to 10100, and the OSS bits to the given OSS
  port.endTransmission();
}

void BMP180::finishTemp() {
  finishTempCore();  
  // Wait for conversion, delay time dependent on OSS
  //Required wait value depends on oversampling, as follows:
  // oss     internal samples      min wait time  calc wait time 2+(3<<OSS)
  //   0                1            4.5ms             5
  //   1                2            7.5ms             8
  //   2                4           13.5ms            14
  //   3                8           25.5ms            26
  //So, all calculated delay times have a 500us margin.
  directTaskManager.schedule(timer_ch,2 + (3<<OSS),0,&finishPresTask,this);
}

void BMP180::finishPresCore() {
  uint8_t msb, lsb, xlsb;
    
  // Read register 0xF6 (MSB), 0xF7 (LSB), and 0xF8 (XLSB)
  port.beginTransmission(ADDRESS);
  port.write(0xF6);
  port.endTransmission();
  port.requestFrom(ADDRESS, 3);
  
  msb = port.read();
  lsb = port.read();
  xlsb = port.read();
  
  UP = (((uint32_t) msb << 16) | ((uint32_t) lsb << 8) | (uint32_t) xlsb) >> (8-OSS);
}

// Read the uncompensated pressure value
void BMP180::finishPres() {
  finishPresCore();
  ready=true;
  start=false;
}

void BMP180::takeMeasurement() {
  startMeasurementCore();
  delay(5);
  finishTempCore();
  delay(2+(3<<OSS));
  finishPresCore();
}



