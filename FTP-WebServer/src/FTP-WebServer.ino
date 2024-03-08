/*
  SDWebServer - Example WebServer with SD Card backend for esp8266 and ESP32-xxx
  It has a FTP backend to upload the file to the server and has a WIFI Manger
  Holding down the boot button for 5 second will reset the WIFI Manager to AP mode so that you can reconfigure the WIFI Manager.
  It will also log the startup
  This version only supports FAT and FAT32 max is 32 GB sd-card
*/

#include <Arduino.h>        //  https://github.com/espressif/arduino-esp32/tree/master/cores/esp32

#if defined(ESP8266)
#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <ESP8266mDNS.h>
#else
#include <WiFi.h>           // https://github.com/espressif/arduino-esp32/tree/master/libraries/WiFi
#include <WebServer.h>      // https://github.com/espressif/arduino-esp32/tree/master/libraries/WebServer
#include <ESPmDNS.h>        // https://github.com/espressif/arduino-esp32/tree/master/libraries/ESPmDNS
#endif

#include <WiFiClient.h>     // https://github.com/espressif/arduino-esp32/tree/master/libraries/WiFi
#include <NTPClient.h>      //  https://github.com/arduino-libraries/NTPClient
#include <WiFiManager.h>    // WifiManager by tzapu  https://github.com/tzapu/WiFiManager
#include <ArduinoOTA.h>     // ArduinoOTA by Arduino, Juraj  https://github.com/JAndrassy/ArduinoOTA
#include <SimpleFTPServer.h> // https://github.com/xreef/SimpleFTPServer

#include <SPI.h>
#include <SD.h>
#include "NTP.hpp"


#define DBG_OUTPUT_PORT Serial
// USBSerial Serial Serial1 Serial2

#if defined(ESP8266)
#warning "ESP8266 Pins You can not change them"

/*
WeMos D1 Mini pinout
Label GPIO    Input        Output Notes
RTS   RESET 
A0    ADC0    Analog Input X
D0    GPIO16  no interrupt no     PWM or I2C support HIGH at boot used to wake up from deep sleep
D5    GPIO14  OK           OK     SPI (SCLK)
D6    GPIO12  OK           OK     SPI (MISO)
D7    GPIO13  OK           OK     SPI (MOSI)
D8    GPIO15  pulled toGND OK     SPI (CS) Boot fails if pulled HIGH
3V3 

TX    GPIO1   TX pin       OK     HIGH at boot debug output at boot, boot fails if pulled LOW
RX    GPIO3   OK           RX pin HIGH at boot
D1    GPIO5   OK           OK     often used as SCL (I2C)
D2    GPIO4   OK           OK     often used as SDA (I2C)
D3    GPIO0   pulled up    OK     connected to FLASH button, boot fails if pulled LOW
D4    GPIO2   pulled up    OK     HIGH at boot connected to on-board LED, boot fails if pulled LOW
GND
5V
*/

#define SD_PIN_SCK         14 // D5 GPIO14 WeMos D1 Mini 
#define SD_PIN_MOSI        13 // D7 GPIO13 WeMos D1 Mini 
#define SD_PIN_MISO        12 // D6 GPIO12 WeMos D1 Mini 
#define SD_PIN_CS          15 // D8 GPIO15 WeMos D1 Mini 
#define BOOTPIN            16 // D0 GPIO16 WeMos D1 Mini

#define LED_RED_PIN        2  // D4 GPIO2 WeMos D1 Mini on-board LED
#define LED_ORANGE_PIN     5  // D1 GPIO5 WeMos D1 Mini 
#define LED_GREEN_PIN      4  // D2 GPIO4 WeMos D1 Mini 

#define RXD               3
#define TXD               1

#elif defined(CONFIG_IDF_TARGET_ESP32S2)
#warning "TARGET_ESP32S2"
/*
https://deguez07.medium.com/esp32-with-sd-card-modules-the-master-guide-5d391f6785d7
https://github.com/Xinyuan-LilyGO/LilyGo-esp32s2-base
https://github.com/Xinyuan-LilyGO/ESP32_S2
*/

#define SD_PIN_SCK         12
#define SD_PIN_MOSI        11
#define SD_PIN_MISO        13
#define SD_PIN_CS          10
#define BOOTPIN            0

#define LED_RED_PIN        2
#define LED_ORANGE_PIN     3
#define LED_GREEN_PIN      4

#define RXD               40
#define TXD               39

#elif defined(CONFIG_IDF_TARGET_ESP32S3)
#warning "TARGET_ESP32S3"
// https://docs.espressif.com/projects/esp-idf/en/latest/esp32s3/hw-reference/esp32s3/user-guide-devkitm-1.html
// https://learn.adafruit.com/adafruit-metro-esp32-s3/pinouts
// gpio.0 Boot Mode. Weak pullup during reset. (Boot Mode 0=Boot from Flash, 1=Download)
// gpio.3 JTAG Mode. Weak pull down during reset. (JTAG Config)
// gpio.45 SPI voltage. Weak pull down during reset. (SPI Voltage 0=3.3v 1=1.8v)
// gpio.46 Boot mode. Weak pull down during reset. (Enabling/Disabling ROM Messages Print During Booting)

// ESP32-S3 DEV Board / Lolin S3 https://www.wemos.cc/en/latest/s3/s3_pro.html / Lilygo T Display S3 :
// SS=10; MOSI=11; MISO=13; SCK=12
// Adafruit Feather S3 / QTPY S3 :
// SS=42; MOSI=35; MISO=37; SCK=36
// XIAO_ESP32S3 :
// SS=44; MOSI=9; MISO=8; SCK=7
// DFRobot Firebeetle S3:
// SS=10; MOSI=15; MISO=16; SCK=17

// The SPI2 (FSPI) default pins are
#define SD_PIN_SCK 12
#define SD_PIN_MOSI 11
#define SD_PIN_MISO 13
#define SD_PIN_CS 10
#define BOOTPIN 0

#define LED_RED_PIN 4
#define LED_ORANGE_PIN 5
#define LED_GREEN_PIN 6
#define LED_RGB_PIN 48 // GPIO 38  

#define RXD 44
#define TXD 43

#elif defined(CONFIG_IDF_TARGET_ESP32C3)

#define SD_PIN_SCK         3
#define SD_PIN_MOSI        4
#define SD_PIN_MISO        5
#define SD_PIN_CS          10
#define BOOTPIN            9

#define LED_RED_PIN        2
#define LED_ORANGE_PIN     6
#define LED_GREEN_PIN      7

#define RXD               20
#define TXD               21

#elif defined(CONFIG_IDF_TARGET_ESP32C6)
// https://github.com/wuxx/nanoESP32-C6

#define SD_PIN_SCK    21
#define SD_PIN_MOSI   19
#define SD_PIN_MISO   20
#define SD_PIN_CS     18

#define PIN_NEOPIXEL  8
#define BOOTPIN       9

#define LED_RED_PIN    4
#define LED_ORANGE_PIN 5
#define LED_GREEN_PIN  6

#define RXD 17
#define TXD 16
#elif defined(CONFIG_IDF_TARGET_ESP32)
/****************************************************************************
    pin setup
    Left
    1  - 3.3 V                                        1
    2  - EN - RESET                                   2
    3  - GPIO36 - ADC0  - INPUT ONLY                  3
    4  - GPIO39 - ADC3  - INPUT ONLY                  4
    5  - GPIO34 - ADC6  - INPUT ONLY                  5
    6  - GPIO35 - ADC7  - INPUT ONLY                  6
    7  - GPIO32 - ADC4  - TOUCH9                      7
    8  - GPIO33 - ADC5  - TOUCH8                      8
    9  - GPIO25 - ADC18 - DAC1                        9
    10 - GPIO26 - ADC19 - DAC2                        10
    11 - GPIO27 - ADC17 - TOUCH7                      11
    12 - GPIO14 - ADC16 - TOUCH6 - SPICLK             12
    13 - GPIO12 - ADC15 - TOUCH5 - SPIMISO            13
    14 - GND                                          14 - GND
    15 - GPIO13 - ADC14 - TOUCH4 - SPIMOSI            15
    16 - GPIO9  - RX1 - Do not USE Connected to Flash 16
    17 - GPIO10 - TX1 - Do not USE Connected to Flash 17
    18 - GPIO11       - Do not USE Connected to Flash 18
    19 - Vin                                          19

    1  - GND                                          1  - GND
    2  - GPIO23 - V SPIMOSI                           2  - MOSI
    3  - GPIO22 - I2C SCL                             3
    4  - TX0    - GPIO1                               4  - USB-TX
    5  - RX0    - GPIO3                               5  - USB-RX
    6  - GPIO21 - I2CSDA                              6
    7  - GND                                          7  - GND
    8  - GPIO19 - V SPIMISO                           8  - MISO
    9  - GPIO18 - V SPISCK                            9  - SCK
    10 - GPIO5  - V SPI SS                            10 - CSN
    11 - GPIO17 - TX2                                 11
    12 - GPIO16 - RX2                                 12
    13 - GPIO4  - ADC10 - TOUCH0                      13 -
    14 - GPIO0  - ADC11 - TOUCH1 - BOOT               14
    15 - GPIO2  - ADC12 - TOUCH2 - OnBoardLed         15 -- LEDPIN
    16 - GPIO15 - ADC13 - TOUCH3 - SPISS              16
    17 - GPIO8  - Do not USE Connected to Flash       17
    18 - GPIO7  - Do not USE Connected to Flash       18
    19 - GPIO6  - Do not USE Connected to Flash       19

    GPIO2 = Debug led on Board LOW = OFF        HIGH = on
*/

#define SD_PIN_SCK         18
#define SD_PIN_MOSI        23
#define SD_PIN_MISO        19
#define SD_PIN_CS          5
#define BOOTPIN            0

#define LED_RED_PIN        15
#define LED_ORANGE_PIN     2
#define LED_GREEN_PIN      32

#define RXD               3
#define TXD               1

#endif

#define USEWIFIMANAGER 
#ifndef USEWIFIMANAGER
const char* ssid = "SSID_ROUTER";
const char* password = "Password_Router";
#endif

#if defined(ESP8266) 
const char* host = "ESP-FTP-12E";
#elif defined(CONFIG_IDF_TARGET_ESP32)
const char* host = "ESP-FTP-ESP";
#elif defined(CONFIG_IDF_TARGET_ESP32C3)
const char* host = "ESP-FTP-C3";
#elif defined(CONFIG_IDF_TARGET_ESP32C6)
const char* host = "ESP-FTP-C6";
#elif defined(CONFIG_IDF_TARGET_ESP32S2)
const char* host = "ESP-FTP-S2";
#elif defined(CONFIG_IDF_TARGET_ESP32S3)
const char* host = "ESP-FTP-S3";
#endif

#if defined(ESP8266) 
ESP8266WebServer server(80);
#else
WebServer server(80);
#endif

#if not defined(ESP8266) || not defined(CONFIG_IDF_TARGET_ESP32)
SPIClass * sd_spi = NULL;
#endif
#define PASSWORDFILE "/password.txt"
char username[16] = "esp";
char password[16] = "esp32";

FtpServer ftpSrv; // set #define FTP_DEBUG in ESP8266FtpServer.h to see ftp verbose on serial

WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP);

unsigned long PreviousTimeSeconds;
unsigned long PreviousTimeMinutes;
unsigned long PreviousTimeHours;
unsigned long PreviousTimeDay;
uint16_t Config_Reset_Counter=0;
int OTAUploadBusy=0;
static bool hasSD = false;
int SDmaxSpeed = 0;

File uploadFile;

bool loadFromSdCard(String path)
{
  String dataType = "text/plain";
  if (path.endsWith("/"))
    path += "index.htm";

  if (path.endsWith(".src"))
    path = path.substring(0, path.lastIndexOf("."));
  else if (path.endsWith(".htm"))
    dataType = "text/html";
  else if (path.endsWith(".html"))
    dataType = "text/html";
  else if (path.endsWith(".css"))
    dataType = "text/css";
  else if (path.endsWith(".js"))
    dataType = "application/javascript";
  else if (path.endsWith(".png"))
    dataType = "image/png";
  else if (path.endsWith(".gif"))
    dataType = "image/gif";
  else if (path.endsWith(".jpg"))
    dataType = "image/jpeg";
  else if (path.endsWith(".ico"))
    dataType = "image/x-icon";
  else if (path.endsWith(".xml"))
    dataType = "text/xml";
  else if (path.endsWith(".pdf"))
    dataType = "application/pdf";
  else if (path.endsWith(".zip"))
    dataType = "application/zip";

  File dataFile = SD.open(path.c_str());
  if (dataFile.isDirectory())
  {
    path += "/index.htm";
    dataType = "text/html";
    dataFile = SD.open(path.c_str());
  }

  if (!dataFile)
    return false;

  uint64_t filesize = dataFile.size();
  if (server.hasArg("download"))
    dataType = "application/octet-stream";
  if (path.endsWith("gz") &&
      dataType != "application/x-gzip" &&
      dataType != "application/octet-stream")
  {
    server.sendHeader(F("Content-Encoding"), F("gzip"));
  }
  server.setContentLength(filesize);
  server.send(200, dataType, "");

  size_t blocksize = 0;
  char databuffer[1024];
  while (filesize > 0)
  {
    blocksize = dataFile.read((uint8_t*)databuffer, sizeof(databuffer));
    if (blocksize > 0) server.sendContent(databuffer, blocksize);
        else
        filesize = 0;  
    if (filesize > blocksize)
      filesize -= blocksize;
    else
      filesize = 0;
  }
  server.client().flush();
  dataFile.close();
  return true;
}

String urlDecode(const String &text)
{
  String decoded = "";
  char temp[] = "0x00";
  unsigned int len = text.length();
  unsigned int i = 0;
  while (i < len)
  {
    char decodedChar;
    char encodedChar = text.charAt(i++);
    if ((encodedChar == '%') && (i + 1 < len))
    {
      temp[2] = text.charAt(i++);
      temp[3] = text.charAt(i++);
      decodedChar = strtol(temp, NULL, 16);
    }
    else
    {
      if (encodedChar == '+')
      {
        decodedChar = ' ';
      }
      else
      {
        decodedChar = encodedChar; // normal ascii char
      }
    }
    decoded += decodedChar;
  }
  return decoded;
}

void handleNotFound()
{
  if (hasSD && loadFromSdCard(urlDecode(server.uri())))
    return;
  String message = "SDCARD Not Detected\n\n";
  message += "SD_PIN_MISO = " + String(SD_PIN_MISO) + "\n";
  message += "SD_PIN_MOSI = " + String(SD_PIN_MOSI) + "\n";
  message += "SD_PIN_SCK = " + String(SD_PIN_SCK) + "\n";
  message += "SD_PIN_CS = " + String(SD_PIN_CS) + "\n";
  message += "CardSpeed = " + String(SDmaxSpeed)+ "\n";
  message += "URI: ";
  message += server.uri();
  message += "\nMethod: ";
  message += (server.method() == HTTP_GET)?"GET":"POST";
  message += "\nArguments: ";
  message += server.args();
  message += "\n";
  for (uint8_t i=0; i<server.args(); i++){
    message += " NAME:"+server.argName(i) + "\n VALUE:" + server.arg(i) + "\n";
  }
  server.send(404, "text/plain", message);
  server.client().flush();
  DBG_OUTPUT_PORT.print(message);
}

void Passwordflie()
{
  server.send(404, "text/plain", "Forbidden");
  server.client().flush();
}

void _callback(FtpOperation ftpOperation, unsigned int freeSpace, unsigned int totalSpace)
{
  Serial.print(">>>>>>>>>>>>>>> _callback ");
  Serial.print(ftpOperation);
  /* FTP_CONNECT,
   * FTP_DISCONNECT,
   * FTP_FREE_SPACE_CHANGE
   */
  Serial.print(" ");
  Serial.print(freeSpace);
  Serial.print(" ");
  Serial.println(totalSpace);

  // freeSpace : totalSpace = x : 360

  if (ftpOperation == FTP_CONNECT)
    Serial.println(F("CONNECTED"));
  if (ftpOperation == FTP_DISCONNECT)
    Serial.println(F("DISCONNECTED"));
};
void _transferCallback(FtpTransferOperation ftpOperation, const char *name, unsigned int transferredSize)
{
  Serial.print(">>>>>>>>>>>>>>> _transferCallback ");
  Serial.print(ftpOperation);
  /* FTP_UPLOAD_START = 0,
   * FTP_UPLOAD = 1,
   *
   * FTP_DOWNLOAD_START = 2,
   * FTP_DOWNLOAD = 3,
   *
   * FTP_TRANSFER_STOP = 4,
   * FTP_DOWNLOAD_STOP = 4,
   * FTP_UPLOAD_STOP = 4,
   *
   * FTP_TRANSFER_ERROR = 5,
   * FTP_DOWNLOAD_ERROR = 5,
   * FTP_UPLOAD_ERROR = 5
   */
  Serial.print(" ");
  Serial.print(name);
  Serial.print(" ");
  Serial.println(transferredSize);
};

int readline(File sd, char *buffer, int maxlen)
{
  int len = 0;
  int c;
  while ((c = sd.read()) != -1 && len < (maxlen - 1))
  {
    switch(c)
    { // Remove whitespace
    case '\t':
    case '\v':
    case '\f':
    case '\r':
    case ' ':
      break;
    case '\n':
      buffer[len] = 0;
      return len;
    default:
      buffer[len] = c;
      len++;
    }
  }
  buffer[len] = 0;
  return len;
}

void setup(void){
  pinMode(BOOTPIN, INPUT_PULLUP);
  pinMode(LED_RED_PIN, OUTPUT);
  
#if defined(ESP8266) || defined(CONFIG_IDF_TARGET_ESP32S2) || defined(CONFIG_IDF_TARGET_ESP32)
  DBG_OUTPUT_PORT.begin(115200);
#else
  DBG_OUTPUT_PORT.begin(115200,  SERIAL_8N1, RXD, TXD);
#endif 
  DBG_OUTPUT_PORT.setDebugOutput(true);
  DBG_OUTPUT_PORT.print("\n");
#ifdef USEWIFIMANAGER
    //WiFiManager, Local intialization. Once its business is done, there is no need to keep it around
    WiFiManager wm;

    // reset settings - wipe stored credentials for testing
    // these are stored by the esp library
    //  wm.resetSettings();

    // Automatically connect using saved credentials,
    // if connection fails, it starts an access point with the specified name ( "AutoConnectAP"),
    // if empty will auto generate SSID, if password is blank it will be anonymous AP (wm.autoConnect())
    // then goes into a blocking loop awaiting configuration and will return success result

    bool res;
     res = wm.autoConnect(); // auto generated AP name from chipid
    // res = wm.autoConnect(DeviceName); // anonymous ap
    // res = wm.autoConnect("AutoConnectAP","password"); // password protected ap
    if(!res) {
        DBG_OUTPUT_PORT.println("Failed to connect Restarting");
        delay(5000);
         if (digitalRead(BOOTPIN)==LOW){ 
          //wm.resetSettings();
        }
        ESP.restart();
    }
    else {
        //if you get here you have connected to the WiFi    
    }
#else
  WiFi.begin(ssid, password);
  DBG_OUTPUT_PORT.print("Connecting to ");
  DBG_OUTPUT_PORT.println(ssid);
  // Wait for connection
  uint8_t i = 0;
  while (WiFi.status() != WL_CONNECTED && i++ < 20) { //wait 10 seconds
    delay(500);
  }
  if(i == 21){
    DBG_OUTPUT_PORT.print("Could not connect to");
    DBG_OUTPUT_PORT.println(ssid);
    while(1) delay(500);
  }
#endif
    WiFi.setSleep(false);
#if not defined(ESP8266)
    esp_wifi_set_ps(WIFI_PS_NONE); // Esp32 enters the power saving mode by default,
#endif
    DBG_OUTPUT_PORT.print("Connected! IP address: ");
    DBG_OUTPUT_PORT.println(WiFi.localIP());
    DBG_OUTPUT_PORT.print("Connecting to ");
    DBG_OUTPUT_PORT.println(WiFi.SSID());

    if (MDNS.begin(host))
    {
      MDNS.addService("http", "tcp", 80);
      DBG_OUTPUT_PORT.println("MDNS responder started");
      DBG_OUTPUT_PORT.print("You can now connect to http://");
      DBG_OUTPUT_PORT.print(host);
      DBG_OUTPUT_PORT.print(".local or http://");
      DBG_OUTPUT_PORT.println(WiFi.localIP());
  }

  // Port defaults to 3232
  // ArduinoOTA.setPort(3232);

  // Hostname defaults to esp3232-[MAC]
  ArduinoOTA.setHostname(host);

  // No authentication by default
  // ArduinoOTA.setPassword("admin");

  // Password can be set with it's md5 value as well
  // MD5(admin) = 21232f297a57a5a743894a0e4a801fc3
  // ArduinoOTA.setPasswordHash("21232f297a57a5a743894a0e4a801fc3");
  ArduinoOTA.onStart([]() {
    String type;
    if (ArduinoOTA.getCommand() == U_FLASH) {
      type = "sketch";
    } else {  // U_FS
      type = "filesystem";
    }
    // NOTE: if updating FS this would be the place to unmount FS using FS.end()
    OTAUploadBusy=60; // only do a update for 60 sec;
    DBG_OUTPUT_PORT.println("Start updating " + type);
  });
  ArduinoOTA.onEnd([]() {
    OTAUploadBusy=0;
    DBG_OUTPUT_PORT.println("\nEnd");
  });
  ArduinoOTA.onProgress([](unsigned int progress, unsigned int total) {
    DBG_OUTPUT_PORT.printf("Progress: %u%%\r", (progress / (total / 100)));
  });
  ArduinoOTA.onError([](ota_error_t error) {
    OTAUploadBusy=0;
    DBG_OUTPUT_PORT.printf("Error[%u]: ", error);
    if (error == OTA_AUTH_ERROR) {
      DBG_OUTPUT_PORT.println("Auth Failed");
    } else if (error == OTA_BEGIN_ERROR) {
      DBG_OUTPUT_PORT.println("Begin Failed");
    } else if (error == OTA_CONNECT_ERROR) {
      DBG_OUTPUT_PORT.println("Connect Failed");
    } else if (error == OTA_RECEIVE_ERROR) {
      DBG_OUTPUT_PORT.println("Receive Failed");
    } else if (error == OTA_END_ERROR) {
      DBG_OUTPUT_PORT.println("End Failed");
    }
  });
  ArduinoOTA.begin();

  server.on(PASSWORDFILE, HTTP_GET, Passwordflie);
  server.onNotFound(handleNotFound);

  server.begin();
  DBG_OUTPUT_PORT.println("HTTP server started");

#if defined(ESP8266) || defined(CONFIG_IDF_TARGET_ESP32)
  SDmaxSpeed =0;
  if (SD.begin(SD_PIN_CS)){
     DBG_OUTPUT_PORT.println("SD Card initialized.");
     hasSD = true;
  }
#else

#if defined(CONFIG_IDF_TARGET_ESP32C3) || defined(CONFIG_IDF_TARGET_ESP32C6)
  sd_spi = new SPIClass(FSPI);
#else
  sd_spi = new SPIClass(); // HSPI
#endif
SDmaxSpeed =50;
  sd_spi->begin(SD_PIN_SCK,SD_PIN_MISO,SD_PIN_MOSI,SD_PIN_CS);
  while (SDmaxSpeed>4 && hasSD==false)
  {
    if (SD.begin(SD_PIN_CS, *sd_spi, 1000000UL * SDmaxSpeed)){
     DBG_OUTPUT_PORT.println("SD Card initialized.");
     hasSD = true;
    } else SDmaxSpeed-=4;
  }
  if(hasSD == false) DBG_OUTPUT_PORT.println("SD Card initialized failed.");
#endif

  timeClient.begin();
  timeClient.setUpdateInterval(1000 * 60 * 60 * 24);  // 24 uur
  timeClient.update();

  String message = "Reboot from: ";
#if defined(ESP8266) 
    message +=  ESP.getChipId();
#else
    message +=  ESP.getChipModel();
#endif
    message += "_";
    message += WiFi.macAddress(); 
    message += " LocalIpAddres: "+ WiFi.localIP().toString();
    message += " SSID: " +String(WiFi.SSID());
    message += " Rssi: " +String(WiFi.RSSI());

#if not defined(ESP8266) 
    message += " Total heap: " +String( ESP.getHeapSize()  / 1024);
    message += " Free heap: "  +String( ESP.getFreeHeap()  / 1024);
    message += " Total PSRAM: "+String( ESP.getPsramSize() / 1024);
    message += " Free PSRAM: " +String( ESP.getFreePsram() / 1024);
    message += " Temperature: "+String(temperatureRead())+" °C "; // internal TemperatureSensor
#if defined(CONFIG_IDF_TARGET_ESP32)
    message += " HallSensor: " +String(hallRead());
#endif
    message += " FlashChipMode: "+String( ESP.getFlashChipMode());
#else
    message += " FlashChipId: "+String( ESP.getFlashChipId());
    message += " FlashChipRealSize: "+String( ESP.getFlashChipRealSize());
#endif 
    message += " FlashChipSize: "+String( ESP.getFlashChipSize());
    message += " FlashChipSpeed: "+String( ESP.getFlashChipSpeed());
    message += " FlashChipMode: "+String( ESP.getFlashChipMode());
    message += " CardSpeed: " + String(SDmaxSpeed);
    message += " Build Date: "+ String (__DATE__ " " __TIME__);
    Log(message);

// Setup the FTP server    
    //ftpSrv.setCallback(_callback);
    //ftpSrv.setTransferCallback(_transferCallback);


    File PwFile = SD.open(PASSWORDFILE, FILE_READ);
    if(PwFile)
    {
      readline(PwFile, username, sizeof(username));
      readline(PwFile, password, sizeof(password));
      PwFile.close();
    }

    String LogMessage = "";
    LogMessage += "User name:";
    LogMessage += username;
    LogMessage += ": Password:";
    LogMessage += password;
    LogMessage += ":";
    Log(LogMessage);

    ftpSrv.begin(username, password,host); // username, password for ftp.   (default 21, 50009 for PASV)
                                      // ftpSrv.begin("ESP", "ESP32"); // username, password for ftp.   (default 21, 50009 for PASV)
}

void Log(String Str)
{
if (SD.exists("/Log.txt")){
    String message = "";
#if defined(ESP8266) 
    File LogFile = SD.open("/Log.txt",FILE_WRITE | O_APPEND);
#else
    File LogFile = SD.open("/Log.txt",FILE_APPEND);
#endif
    message += String( getFormattedDateTime(timeClient.getEpochTime()))+",";
    message += Str;
    LogFile.println(message);
    LogFile.close();
  }
}

void loop(void){
  unsigned long currentTimeSeconds=timeClient.getEpochTime();
  yield();
  ArduinoOTA.handle();
  if(OTAUploadBusy ==0 )
  { // Do not do things that take time when OTA is busy
    server.handleClient();
    ftpSrv.handleFTP();
  }

  if(PreviousTimeDay != (currentTimeSeconds/(60*60*24)))
  {   // Day Loop 
    PreviousTimeDay = (currentTimeSeconds/(60*60*24));
    timeClient.update();
  }
  if(PreviousTimeHours != (currentTimeSeconds/(60*60)))
  {   // Hours Loop
    PreviousTimeHours = (currentTimeSeconds/(60*60));
  }

  if(PreviousTimeMinutes != (currentTimeSeconds/60))
  {   // Minutes loop
    PreviousTimeMinutes = (currentTimeSeconds/60);
    if ((WiFi.status() != WL_CONNECTED)) { // if WiFi is down, try reconnecting
      WiFi.disconnect();
      WiFi.reconnect();
    }
  }

  if(PreviousTimeSeconds != currentTimeSeconds)
  { // 1 second loop
    PreviousTimeSeconds = currentTimeSeconds;
      if(digitalRead(LED_RED_PIN )==0)   digitalWrite(LED_RED_PIN, 1); else   digitalWrite(LED_RED_PIN, 0);
      if(OTAUploadBusy >0 ) OTAUploadBusy--;
#ifdef USEWIFIMANAGER      
      if (digitalRead(BOOTPIN)==LOW){     
        if( ++Config_Reset_Counter>5 ){ // press the BOOT 5 sec to reset the WifiManager Settings
          WiFiManager wm;               //WiFiManager, Local intialization. Once its business is done, there is no need to keep it around
          delay(500);
          wm.resetSettings();
          ESP.restart();
        } 
      } else {
        Config_Reset_Counter=0;
      }
#endif
  }  
}
