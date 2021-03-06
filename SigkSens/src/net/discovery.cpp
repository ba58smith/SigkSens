#ifdef ESP8266
  #include <ESP8266mDNS.h>        // Include the mDNS library
#elif defined(ESP32)
  #include <ESPmDNS.h>
#endif

#include <StreamString.h>
#include <uSSDP.h>
#include <ESPAsyncWebServer.h>

#include "discovery.h"
#include "../../config.h"
#include "../../sigksens.h"

uDevice device;
uSSDP SSDP;

//SSDP properties
const char * modelName = "WifiSensorNode";
const char * modelNumber = "12345";

static const char* ssdpTemplate =
  "<?xml version=\"1.0\"?>"
  "<root xmlns=\"urn:schemas-upnp-org:device-1-0\">"
    "<specVersion>"
      "<major>1</major>"
      "<minor>0</minor>"
    "</specVersion>"
    "<URLBase>http://%u.%u.%u.%u/</URLBase>"
    "<device>"
      "<deviceType>upnp:rootdevice</deviceType>"
      "<friendlyName>%s</friendlyName>"
      "<presentationURL>index.html</presentationURL>"
      "<serialNumber>%u</serialNumber>"
      "<modelName>%s</modelName>"
      "<modelNumber>%s</modelNumber>"
      "<modelURL>http://www.espressif.com</modelURL>"
      "<manufacturer>Espressif Systems</manufacturer>"
      "<manufacturerURL>http://www.espressif.com</manufacturerURL>"
      "<UDN>uuid:38323636-4558-4dda-9188-cda0e6%02x%02x%02x</UDN>"
    "</device>"
  "</root>\r\n"
  "\r\n";


void handleSSDP() {
  SSDP.process();
}

void setupDiscovery() {
  if (!MDNS.begin(myHostname)) {             // Start the mDNS responder for esp8266.local
    Serial.println(F("Error setting up MDNS responder!"));
  } else {
    Serial.print (F("mDNS responder started at "));
    Serial.print (myHostname);
    Serial.println(F(""));
  }
  MDNS.addService("http", "tcp", 80);
  
  
  byte mac[6];
  char base[UUIDBASE_SIZE];
  WiFi.macAddress(mac); 
  


  device.begin((const char*)base, mac);
  device.serialNumber((char*)"1241");
  device.manufacturer((char*)"SigKSens");
  device.manufacturerURL((char*)"http://www.signalk.org");
  device.modelName((char*)"ESP-32");
  device.modelNumber(1, 0);
  device.friendlyName((char*)"ESP32-WROVER");
  device.presentationURL((char*)"/");
  SSDP.begin(&device);
  Serial.println("SSDP Started");
/*
  Serial.println(F("Starting SSDP..."));
  SSDP.setSchemaURL("description.xml");
  SSDP.setHTTPPort(80);
  SSDP.setName(myHostname);
  SSDP.setSerialNumber("12345");
  SSDP.setURL("index.html");
  SSDP.setModelName("WifiSensorNode");
  SSDP.setModelNumber("12345");
  SSDP.setModelURL("http://www.signalk.org");
  SSDP.setManufacturer("SigK");
  SSDP.setManufacturerURL("http://www.signalk.org");
  SSDP.setDeviceType("upnp:rootdevice");
  SSDP.begin();
*/

  app.onRepeat(30, handleSSDP);
  
}


void setupSSDPHttpCallback(AsyncWebServer &server) {

  server.on("/description.xml", HTTP_GET, [](AsyncWebServerRequest *request){
      StreamString output;
      if(output.reserve(1024)){
        IPAddress ip = WiFi.localIP();
        uint32_t chipId = 12345;
        output.printf(ssdpTemplate,
          ip[0], ip[1], ip[2], ip[3],
          myHostname,
          chipId,
          modelName,
          modelNumber,
          (uint8_t) ((chipId >> 16) & 0xff),
          (uint8_t) ((chipId >>  8) & 0xff),
          (uint8_t)   chipId        & 0xff
        );
        request->send(200, "text/xml", (String)output);
      } else {
        request->send(500);
      }
    });


}