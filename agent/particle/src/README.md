Firmware Psuedocode
===================

The main particle loop will be responsible for:
 - Maintaining a connection with the cloud
 - Keeping the time of the RTC up to date
 - Sending data from the message Queue to the cloud
 - Putting data in the SD backlog if there is no cloud connection
 - Sending data from the backlog to the cloud
 - writing data from the message Queue to the SD card
 - Triggering the generation of periodic data collection
 - Servicing the power state change detector
 - Servicing the watchdog
 - maintaining the state of the light
