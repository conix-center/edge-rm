/**
 * SD Card related operations.
 */

#pragma once

#include <Particle.h>
#include <SdFat.h>

class SDCard {

	#define SCK A3
	#define  MISO A4
	#define MOSI A5
	#define  SS A2

	String result;

public:
	SdFat sd;
	void setup();
	String getResult();

  	/**
	 * Power cycles SD Card. Blocking call.
	 */
	void PowerOn();
	void PowerOff();
	
	String getLastLine(String filename);
	bool removeLastLine(String filename);
	bool readLastLine(String filename);
 	bool Write(String filename, String to_write);
	int getSize(String filename);

	String ReadLine(String filename, uint32_t position);
	String Read(String filename);
};
