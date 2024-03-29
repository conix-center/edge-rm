#include <Particle.h>

#include "lib/CellStatus.h"

String CellStatus::read() {

  uint32_t freemem = System.freeMemory();
  CellularDevice dev;
  dev.size = sizeof(dev);
  cellular_device_info(&dev, NULL);
  if(Cellular.ready()) {
    CellularSignal sig = Cellular.RSSI();

    result = String(System.version().c_str());
    //result = result + String(MINOR_DLIM) + dev.imei.substring(8) + String(MINOR_DLIM) + dev.iccid;
    result = result + String(MINOR_DLIM) + String(dev.imei).substring(8);
    result = result + String(MINOR_DLIM) + String(freemem);
    result = result + String(MINOR_DLIM) + String(sig.rssi) + String(MINOR_DLIM) + String(sig.qual);

    CellularBand band_avail;
    if (Cellular.getBandSelect(band_avail)) {
      result  = result + String(MINOR_DLIM) + String(band_avail);
    } else {
      result = result + String(MINOR_DLIM) + String("!");
    }
  } else {
    result = String(System.version().c_str());
    //result = result + String(MINOR_DLIM) + dev.imei + String(MINOR_DLIM) + dev.iccid;
    result = result + String(MINOR_DLIM) + String(dev.imei).substring(8);
    result = result + String(MINOR_DLIM) + String(freemem);
    result = result + String(MINOR_DLIM) + "!" + String(MINOR_DLIM) + "!";
    result = result + String(MINOR_DLIM) + "!";
  }

  return result;
}
