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
#include <Pinger.h>
#include <LittleFS.h>
#include "hamqtt.hpp"
#include "web.h"
#include "Global.h"
#include "Mqtt.hpp"
#include "GlStatus.h"  
#include "HFunc.hpp"  
#include "BatHVAC.hpp"
#include "Params.h" 
#include "DeltaLTime.h"

TBMSCom TBMSComobj;  
Pinger Pinger_;
int PingErrCntr=0;
ESP8266Timer ITimer;


 

void IRAM_ATTR hwTimerHandler()
{
  TBMSComobj.period();
}
void statusLoop(void *);
void checkNetConnection(void *);
void hystReg(bool heatMode,float in,float low_lev,float high_lev,int out_pin, bool & output);

int Loop_runs_perSec;

DeltaLTime LowLoopTimer;
DeltaLTime MiddleLoopTimer;
DeltaLTime HighLoopTimer;

void setup() {
  Serial.begin(CPUINTERFACE_SPEED);
  DEBUG_PART(Serial.println("Starting..."));
  delay(100);
  pinMode(RED_LED_PIN, OUTPUT);
  pinMode(GREEN_LED_PIN, OUTPUT);
  pinMode(RELE_HEATING_PIN, OUTPUT);
  pinMode(RELE_VENTILATION_PIN, OUTPUT);
  pinMode(RELE_MAIN_PIN, OUTPUT);
  //pinMode(RELE_RESERVE_PIN, OUTPUT);
  pinMode(WIFICONF_BUT_PIN, INPUT_PULLUP);

  digitalWrite(RED_LED_PIN, LED_ACTIVELEV);  //On - indicate is starting
  digitalWrite(GREEN_LED_PIN, !LED_ACTIVELEV);  
  digitalWrite(RELE_HEATING_PIN, !RELE_ACTIVELEV);  
  digitalWrite(RELE_VENTILATION_PIN, !RELE_ACTIVELEV); 
  digitalWrite(RELE_MAIN_PIN, !RELE_ACTIVELEV);  
  //digitalWrite(RELE_RESERVE_PIN, !RELE_ACTIVELEV); 

  Serial.begin(CPUINTERFACE_SPEED);

  bool statBool = LittleFS.begin();
  assert(statBool); //An Error has occurred while mounting LittleFS

  if(!Params.init())            // must be the first 
    Status.gerror.bits.defPar=1;

  TBMSComobj.init(&Serial,CPUINTERFACE_SPEED);

  DEBUG_PART(Serial.println("Part1 finished..."));
  delay(100);
  WiFi.mode(WIFI_STA); // explicitly set mode, esp defaults to STA+AP
  DEBUG_PART(Serial.println("Waiting for possible pressing WIFI Configuration."));
  delay(3000);          
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
#ifndef REAL_TARGET_HW
 if(false){//suppress info from buttom, set standard connect to WIFI 
#else
 if(digitalRead(WIFICONF_BUT_PIN)==WIFICONF_BUT_ACTLEV){
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

    web_init();
    DEBUG_PART(Serial.println("HTTP server started"));

    }
    else{
      DEBUG_PART(Serial.println("Not Connected into WIFI."));
    } 
    DEBUG_PART(Serial.println(""));
  }
  Mqtt_init(); 

 // adk::set_interval(BatHVAC::process, 5000);           // function call

  while(Serial.available()) {Serial.read();} // clear any chaos before 
  
 
  //delay(5000); //wait for 5 sec to start running
  // Init HW timer
//  bool startStatus = ITimer.attachInterruptInterval(HWTIMER_PERIOD * 1000, hwTimerHandler); Test
//  assert(startStatus);
  
  
  
  Pinger_.OnReceive([](const PingerResponse& response)
  {
    if(!(response.ReceivedResponse)){
      PingErrCntr++;
    }
    else{
      PingErrCntr = 0;
    }
    DEBUG_LOG(true,"PingErrCntr=",PingErrCntr);
    // Return true to continue the ping sequence.
    // If current event returns false, the ping sequence is interrupted.
    return false;
  });

  Pinger_.OnEnd([](const PingerResponse& response)
  {
    return true;
  });
   
  
  ESP.wdtDisable();   //disable SW WDT to enable HW WDT -->time out is in about 4 sec 

  LowLoopTimer.setRef();
  HighLoopTimer.setRef();
  MiddleLoopTimer.setRef();
}


uint8_t RX_buffer[BUFFER_LEN];
uint32_t RX_time[BUFFER_LEN];
int RX_buffer_IND=0;
bool startMes = true;



void loop() {
  static int loop_diag_cntr=0;
  static unsigned long last_mes_time=0;
  loop_diag_cntr++;
  TBMSComobj.main();
  Hamqtt::main();
  WebServer.handleClient();
  
  //***************************** LOW LOOP *************************
  if(LowLoopTimer.delta()>= LOWLOOP_TIME){ 
    LowLoopTimer.setRef();
    LowLoopCntr++;
    Mqtt_loopS((void *) 0);
  }
  //***************************** Middle LOOP *************************
  if(MiddleLoopTimer.delta()>= MIDLELOOP_TIME){ 
    MiddleLoopTimer.setRef();
    LoopCntr++;
    Mqtt_loopQ((void *) 0);          
    checkNetConnection((void *) 0);
    BatHVAC::process((void *) 0);
  }
  //***************************** High LOOP *************************
  if(HighLoopTimer.delta()>= HIGHLOOP_TIME){
    HighLoopTimer.setRef();
    HighLoopCntr++;
    Status.refresh();
  }

  if((millis() - last_mes_time) > 1000){
    Loop_runs_perSec=loop_diag_cntr;
    last_mes_time=millis();
    loop_diag_cntr=0;
  }

}




void checkNetConnection(void *){
//    DEBUG_PART(Serial.println("checkNetConnection"));
/*
  if(PingErrCntr >=3){
    
    ESP.restart(); // not possible communicate with net, may be some error, reset system
  }
  Pinger_.Ping(WiFi.gatewayIP());   
*/
  if(millis() < START_TIME_OF_MQTT_MONITORING || MQTT_Check())
    ESP.wdtFeed();  // HW WD do not work
  else{
    DEBUG_LOG0(true,"Restart from MQTT check");  
    ESP.restart();  
  }
}




  


    



 