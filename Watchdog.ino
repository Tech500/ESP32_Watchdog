#include <WiFi.h>
#include <WiFiUdp.h>
#include <ESP8266FtpServer.h>
#include <sys/time.h>
#include <time.h>
#include "SPIFFS.h"
#include <Ticker.h>

// Replace with your network details
const char* ssid = "R2D2";
const char* password = "sissy4357";

///Are we currently connected?
boolean connected = false;

FtpServer ftpSrv;

WiFiUDP udp;
// local port to listen for UDP packets
const int udpPort = 1337;
char incomingPacket[255];
char replyPacket[] = "Hi there! Got the message :-)";
//NTP Time Servers
const char * udpAddress1 = "us.pool.ntp.org";
const char * udpAddress2 = "time.nist.gov";

#define TZ "EST+5EDT,M3.2.0/2,M11.1.0/2"

int DOW, MONTH, DATE, YEAR, HOUR, MINUTE, SECOND;

char strftime_buf[64];

String dtStamp(strftime_buf);

int lc = 0;
time_t tnow = 0;

Ticker secondTick;

portMUX_TYPE mux = portMUX_INITIALIZER_UNLOCKED;

volatile int watchdogCounter;
volatile int watchDog = 0;

void IRAM_ATTR ISRwatchdog()
{

     portENTER_CRITICAL_ISR(&mux);

     watchdogCounter++;

     if (watchdogCounter == 75)  //Number of seconds until "Watchdog event" is triggered.
     {

          watchDog = 1;  //Flag used to determine when logWatchdog function is called.

     }

     portEXIT_CRITICAL_ISR(&mux);

}


void setup()
{

     Serial.begin(9600);

     while(!Serial);

     Serial.println("\nWatchdog.ino 01/31/2020 @ 06:02 EDT");

     WiFi.begin(ssid, password);

     //Server settings
     IPAddress ip {10,0,0,110};
     IPAddress subnet {255,255,255,0};
     IPAddress gateway {10,0,0,1};
     IPAddress dns {10,0,0,1};

     WiFi.config(ip, gateway, subnet, dns);

     Serial.println("\nWaiting for the Network Connection...");

     while (WiFi.status() != WL_CONNECTED)
     {

          delay(500);
          Serial.print(".");

     }

     // Printing the ESP IP address
     Serial.print("\nIP Address:  ");
     Serial.println(WiFi.localIP());
     Serial.println("");

     secondTick.attach(1, ISRwatchdog);

     configTime(0, 0, udpAddress1, udpAddress2);
     setenv("TZ", "EST+5EDT,M3.2.0/2,M11.1.0/2", 3);   // this sets TZ to Indianapolis, Indiana
     tzset();

     while (time(nullptr) < 100000ul)
     {

          Serial.print(".");
          delay(5000);

     }

     Serial.println("\nSystem Time Set");

#ifdef ESP32       //esp32 we send true to format spiffs if cannot mount
     if (SPIFFS.begin(true))
     {
#elif defined ESP8266
     if (SPIFFS.begin())

#endif
          Serial.println("SPIFFS opened!");
          ftpSrv.begin("admin", "password");

     }

     Serial.println("Ready\n");

}

void loop()
{

     //udp only send data when connected
     if (connected)
     {

          //Send a packet
          udp.beginPacket(udpAddress1, udpPort);
          udp.printf("Seconds since boot: %u", millis() / 1000);
          udp.endPacket();

     }

     for (int x = 1; x < 5000; x++)
     {
          ftpSrv.handleFTP();
     }

     if ( watchDog == 1)
     {

          logWatchdog();

          watchDog = 0;

     }

     //watchdogCounter = 0;  //For testing watchdog Interrupt.  This will cause a "Waterdog triggered event."
     //Uncomment the above line; for normal use:   watchdogCounter = 0;  
     //This prevents a "Watchdog triggered event" from normal use; unless triggered by a skech issue.
}

String getDateTime()
{
     struct tm *ti;

     tnow = time(nullptr) + 1;
     //strftime(strftime_buf, sizeof(strftime_buf), "%c", localtime(&tnow));
     ti = localtime(&tnow);
     DOW = ti->tm_wday;
     YEAR = ti->tm_year + 1900;
     MONTH = ti->tm_mon + 1;
     DATE = ti->tm_mday;
     HOUR  = ti->tm_hour;
     MINUTE  = ti->tm_min;
     SECOND = ti->tm_sec;

     strftime(strftime_buf, sizeof(strftime_buf), "%a , %m/%d/%Y , %H:%M:%S %Z", localtime(&tnow));
     dtStamp = strftime_buf;
     return (dtStamp);

}

void logWatchdog()
{

     yield();

     Serial.println("");
     Serial.println("Watchdog event triggered.");

     // Open a "log.txt" for appended writing
     File log = SPIFFS.open("/WATCHDOG.TXT", FILE_APPEND);

     if (!log)
     {
          Serial.println("file 'WATCHDOG.TXT' open failed");
     }

     getDateTime();

     log.print("Watchdog Restart:  ");
     log.print("  ");
     log.print(dtStamp);
     log.println("");
     log.close();

     Serial.println("Watchdog Restart  " + dtStamp + "\n");

     ESP.restart();

}
