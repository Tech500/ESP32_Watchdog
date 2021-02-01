# ESP32_Watchdog

Implementation of a Watchdog timer using the ESP32 Ticker library.  Sketch features Watchdog interrupt, 
logging with date-time stamping of "Watchdog Triggered Events," and a FTP Server

WatchdogCounter set to equal 75 seconds to allow time to "FTP" between "Watchdog Triggered Events".

FTP tested using "Filezilla."

