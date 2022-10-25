#include <Arduino.h>
#include "DebugFnc.h"
#include "tbmscom.hpp"
#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>
//#include <ESP8266mDNS.h>
//#include "mdns.h"                //alternatice MDNS against ESP8266mDNS, possible implement also client side of MDNS
#include <WiFiManager.h>         //https://github.com/tzapu/WiFiManager
#include <ESP8266HTTPClient.h>
#include <ESP8266TimerInterrupt.h>
#include "web.h"
#include "Global.h"

TBMSCom TBMSComobj;  //test

ESP8266Timer ITimer;

void IRAM_ATTR hwTimerHandler()
{
  TBMSComobj.period();
}




void setup() {
  pinMode(BUILTIN_LED_PIN, OUTPUT);
  TBMSComobj.init(&Serial,CPUINTERFACE_SPEED);

  Serial.begin(CPUINTERFACE_SPEED);
  DEBUG_PART(Serial.println("Starting..."));
  delay(10);
  /*
  if(!SPIFFS.begin()){
    DEBUG_PART(Serial.println("SPIFFS start error"));
  }
  if(!Params_init())            // must be the first 
    Status.warning.bits.defPar=1;
  */
  

  //WiFiManager
  //Local intialization. Once its business is done, there is no need to keep it around
  WiFiManager wifiManager;
  wifiManager.setConnectTimeout(30);


  //fetches ssid and pass from eeprom and tries to connect
  //if it does not connect it starts an access point with the specified name
  //and goes into a blocking loop awaiting configuration
#ifdef DEBUG_MODE
 if(false){//suppress info from buttom, set standard connect to WIFI 
#else
 if(digitalRead(WIFICONF_BUT_PIN)){
#endif
  DEBUG_PART(Serial.println("startConfigPortal")); 
  //create config portal
  wifiManager.setConfigPortalTimeout(300); 
  wifiManager.startConfigPortal("BMS_Com");
 } 
 else{ 
   wifiManager.setConfigPortalTimeout(1); // suppress config portal  - set minimal time of config portal
  // try to connect with last credentials, if not possible do not create config portal
  if(wifiManager.autoConnect()){
    DEBUG_PART(Serial.println(""));
    DEBUG_PART(Serial.print("Connected..."));
    DEBUG_PART(Serial.print("IP address: "));
    DEBUG_PART(Serial.println(WiFi.localIP()));

    //Mqtt_init();

    web_init();
    DEBUG_PART(Serial.println("HTTP server started"));

    }
    else{
      DEBUG_PART(Serial.println("Not Connected into WIFI."));
    } 
    DEBUG_PART(Serial.println(""));
  }
 
   
  while(Serial.available()) {Serial.read();} // clear any chaos before 
  
  //ESP.wdtDisable();   //not activated WDT to test stability, disable SW WDT to enable HW WDT -->time out is in about 4 sec 


  // Init HW timer
  bool startStatus = ITimer.attachInterruptInterval(HWTIMER_PERIOD * 1000, hwTimerHandler);
  assert(startStatus);


}


uint8_t RX_buffer[BUFFER_LEN];
uint32_t RX_time[BUFFER_LEN];
int RX_buffer_IND=0;
bool startMes = true;

void loop() {
  //static uint32_t lastTime=0xffffffff;
  static bool BuiltInLed=false;
  //uint32_t actTime=0;

  BuiltInLed=!BuiltInLed;    
  digitalWrite(BUILTIN_LED_PIN, BuiltInLed);  

  //debug part
/*
  int char_ = Serial.read();
  if(char_ >0){
    actTime = millis();
    if(char_== 0x1 && ((actTime - lastTime) > 6))
      startMes=true;
    lastTime=actTime;
  }
  if(startMes){
    if(RX_buffer_IND < BUFFER_LEN){
      if(char_ >0){

          RX_time[RX_buffer_IND]=actTime;
          RX_buffer[RX_buffer_IND]=char_;
          RX_buffer_IND++;
      }
    }  
    else  
      startMes = false;
  }  
  */  
  // put your main code here, to run repeatedly:
  TBMSComobj.main();

  WebServer.handleClient();

}

