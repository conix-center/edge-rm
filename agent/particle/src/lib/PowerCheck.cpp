#include <Particle.h>

#include "lib/PowerCheck.h"

void PowerCheck::setup() {
	// This can't be part of the constructor because it's initialized too early.
	// Call this from setup() instead.

	// BATT_INT_PC13
	attachInterrupt(LOW_BAT_UC, &PowerCheck::interruptHandler, this, FALLING);

	//Drive the pin low
	pinMode(AC_PWR_EN, OUTPUT);
	digitalWrite(AC_PWR_EN, LOW);

	// Enable charging
	pmic.begin();
	pmic.enableCharging();
	setChargeCurrent();
	lowerChargeVoltage();
}

bool PowerCheck::getHasPower() {
	// Bit 2 (mask 0x4) == PG_STAT. If non-zero, power is good
	// This means we're powered off USB or VIN, so we don't know for sure if there's a battery
	byte systemStatus = pmic.getSystemStatus();
	hasPower = ((systemStatus & 0x04) != 0);
	return hasPower;
}

bool PowerCheck::enableCharging() {
	return pmic.enableCharging();
}

/**
 * Returns true if the Electron has a battery.
 */
bool PowerCheck::getHasBattery() {
	if (millis() - lastChange < 100) {
		// When there is no battery, the charge status goes rapidly between fast charge and
		// charge done, about 30 times per second.

		// Normally this case means we have no battery, but return hasBattery instead to take
		// care of the case that the state changed because the battery just became charged
		// or the charger was plugged in or unplugged, etc.
		return hasBattery;
	}
	else {
		// It's been more than a 100 ms. since the charge status changed, assume that there is
		// a battery
		return true;
	}
}

int PowerCheck::getChargeCurrent() {
	return pmic.getChargeCurrent();
}

uint16_t* PowerCheck::getbuf() {
    digitalWrite(AC_PWR_EN, HIGH);
    setADCSampleTime(ADC_SampleTime_84Cycles);
    delay(1000);
    static uint16_t buf[4000];
    int count = 0;
    while(count < 2000) {
        int L = analogRead(AC_L_HV_OUT);
        int N = analogRead(AC_N_HV_OUT);
	buf[count*2] = L;
	buf[count*2+1] = N;
        count++;
    }
    digitalWrite(AC_PWR_EN, LOW);

    return buf;
}

float PowerCheck::getHVVoltage() {
    Serial.println("getting voltage");
    digitalWrite(AC_PWR_EN, HIGH);
    int count = 0;
    int L_max = 0;
    int L_min = 4096;
    int N_measure = 0;
    setADCSampleTime(ADC_SampleTime_84Cycles);

    //Alright take the max and min so that we can get both the magnitude
    //and the average
    while(count < 6000) {
        int L = analogRead(AC_L_HV_OUT);
        int N = analogRead(AC_N_HV_OUT);
        if(L > L_max) {
            L_max = L;
            N_measure = N;
        }
        if(L < L_min) {
            L_min = L;
        }
        count++;
    }

    int L_avg = (L_max + L_min)/2;
    
    int larray[10];
    int narray[10];
    for(uint8_t i = 0; i < 10; i++) {
	count = 0;
	L_max = 0;
	while(count < 6000) {
    	    int L = analogRead(AC_L_HV_OUT);
    	    int N = analogRead(AC_N_HV_OUT);
    	    if(L > L_max) {
    	        larray[i] = L;
    	        narray[i] = N;
		L_max = L;
    	    }
    	    count++;
    	}
    }

    //Use the average as our "zero point" to calculate frequency
    //Take 5 samples at the zero point - this should give use two periods to average
    uint8_t mcount = 0;
    int m[3];
    bool ready = false;
    int startMillis = millis();
    while(mcount < 3 && millis() - startMillis < 1000) {
        int L = analogRead(AC_L_HV_OUT);
	if(L > L_avg && ready) {
	    m[mcount] = micros();
	    mcount++;
	    ready = false;
	} else if (L < L_avg){
	    ready = true; 
	}
    }
    if(millis() - startMillis >= 1000) {
	periodMicros = 1;
    } else {
	//now calculate the microseconds for each period
    	int p1 = m[2] - m[1];
    	int p2 = m[1] - m[0];
    	periodMicros = (p1 + p2)/2;
    }
    
    Serial.printlnf("L voltage count: %d", L_max);
    Serial.printlnf("N voltage count: %d", N_measure);

    float vtotal = 0;
    for(uint8_t i = 0; i < 10; i++) {
	float l_volt = (larray[i]*(3.3/4096)*(953/1.00))-(1.5*(953/1.00));
	float n_volt = (narray[i]*(3.3/4096)*(953/1.00))-(1.5*(953/1.00));
	vtotal += (l_volt - n_volt);
    }

    float volt = (vtotal/10.0);
    digitalWrite(AC_PWR_EN, LOW);
    Serial.printlnf("Calculated voltage: %d",volt);
    return volt;
}

float PowerCheck::getVoltage() {
    Serial.println("getting voltage");
    digitalWrite(AC_PWR_EN, HIGH);
    int count = 0;
    int L_max = 0;
    int L_min = 4096;
    int N_measure = 0;
    setADCSampleTime(ADC_SampleTime_84Cycles);

    //Alright take the max and min so that we can get both the magnitude
    //and the average
    while(count < 6000) {
        int L = analogRead(AC_L_LV_OUT);
        int N = analogRead(AC_N_LV_OUT);
        if(L > L_max) {
            L_max = L;
            N_measure = N;
        }
        if(L < L_min) {
            L_min = L;
        }
        count++;
    }

    int L_avg = (L_max + L_min)/2;
    
    int larray[10];
    int narray[10];
    for(uint8_t i = 0; i < 10; i++) {
	count = 0;
	L_max = 0;
	while(count < 6000) {
    	    int L = analogRead(AC_L_LV_OUT);
    	    int N = analogRead(AC_N_LV_OUT);
    	    if(L > L_max) {
    	        larray[i] = L;
    	        narray[i] = N;
		L_max = L;
    	    }
    	    count++;
    	}
    }

    //Use the average as our "zero point" to calculate frequency
    //Take 5 samples at the zero point - this should give use two periods to average
    uint8_t mcount = 0;
    int m[3];
    bool ready = false;
    int startMillis = millis();
    
    bool l_high = true;

    while(mcount < 3 && millis() - startMillis < 1000) {
        int L = analogRead(AC_L_LV_OUT);
        int N = analogRead(AC_N_LV_OUT);
	if(L >= N) {
	    if(l_high == false) {
		//set l_high to true
		l_high = true;
		m[mcount] = micros();
		mcount++;
	    }
	} else {
	    l_high = false;
	}
    }

    if(millis() - startMillis >= 1000) {
	periodMicros = 1;
    } else {
	//now calculate the microseconds for each period
    	int p1 = m[2] - m[1];
    	int p2 = m[1] - m[0];
    	periodMicros = (p1 + p2)/2;
    }
    
    Serial.printlnf("L voltage count: %d", L_max);
    Serial.printlnf("N voltage count: %d", N_measure);

    float vtotal = 0;
    for(uint8_t i = 0; i < 10; i++) {
	float l_volt = (larray[i]*(3.3/4096)*(953/3.74))-(1.5*(953/3.74));
	float n_volt = (narray[i]*(3.3/4096)*(953/3.74))-(1.5*(953/3.74));
	vtotal += (l_volt - n_volt);
    }

    float volt = (vtotal/10.0);
    digitalWrite(AC_PWR_EN, LOW);
    Serial.printlnf("Calculated voltage: %d",volt);
    return volt;
}

int PowerCheck::getPeriod() {
    return periodMicros;
}

int PowerCheck::getLCycles() {
    return 0;
}

int PowerCheck::getNCycles() {
    return 0;
}

void PowerCheck::setChargeCurrent() {
	pmic.setInputCurrentLimit(900);
	pmic.setChargeCurrent(0,0,0,0,0,0);
}

void PowerCheck::lowerChargeVoltage() {
	pmic.setChargeVoltage(4040);
}

/**
 * Returns true if the Electron is currently charging (red light on)
 */
bool PowerCheck::getIsCharging() {
	if (getHasBattery()) {
		byte systemStatus = pmic.getSystemStatus();

		// Bit 5 CHRG_STAT[1] R
		// Bit 4 CHRG_STAT[0] R
		// 00 – Not Charging, 01 – Pre-charge (<VBATLOWV), 10 – Fast Charging, 11 – Charge Termination Done
		byte chrgStat = (systemStatus >> 4) & 0x3;

		// Return true if battery is charging if in pre-charge or fast charge mode
		return (chrgStat == 1 || chrgStat == 2);
	}
	else {
		// Does not have a battery, can't be charging.
		// Don't just return the charge status because it's rapidly switching
		// between charging and done when there is no battery.
		return false;
	}
}

void PowerCheck::interruptHandler() {
	bool p = hasPower;
	if(p != getHasPower()) {
		if(p == true) {
			// We had power and now don't
			lastUnplugMillis = millis();
			lastUnplugTime = Time.now();
		} else {
			// We didn't have power and now do
			lastPlugMillis = millis();
			lastPlugTime = Time.now();
		}
	}
	lastChange = millis();
}
