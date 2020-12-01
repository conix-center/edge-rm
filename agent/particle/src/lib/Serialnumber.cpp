#include <OneWire.h>
#include "board.h"
#include "lib/Serialnumber.h"

uint8_t* Serialnumber::readBytes() {
  OneWire ds(SERIAL_NUM);
  byte i;
  boolean present;
  byte crc_calc;    //calculated CRC
  byte crc_byte;    //actual CRC as sent by DS2401
  //1-Wire bus reset, needed to start operation on the bus,
  //returns a 1/TRUE if presence pulse detected
  present = ds.reset();
  if (present == TRUE)
  {
    ds.write(0x33);  //Send Read data command
    data[0] = ds.read();
    for (i = 1; i <= 6; i++)
    {
      data[i] = ds.read(); //store each byte in different position in array
    }
    crc_byte = ds.read(); //read CRC, this is the last byte
    crc_calc = OneWire::crc8(data, 7); //calculate CRC of the data

    if(crc_calc == crc_byte) {
        return data;
    } else {
        return NULL;
    }
  } else {
    return NULL;
  }
}

String Serialnumber::read() {
    uint8_t* r = this->readBytes();
    if(!r) {
        return "";
    }

    result = "";
    for(uint8_t i = 1; i < 7; i++) {
        char t[3];
        snprintf(t,3,"%02X",data[i]);
        result += String(t);
    }

    return result;
}
