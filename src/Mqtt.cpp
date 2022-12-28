#include <ESP8266WiFi.h>
#include "tbmscom.hpp"
#include "hamqtt.hpp"
#include "Global.h" 
#include "DebugFnc.h"  

IPAddress BrokerIP(192,168,1,2);     
char MqttUserName[] = "homeassistant"; 
char MqttPass[] = "ol3uuNeek6ke7eich8aiva7ZoxoiVei1aiteith0aighae0ieP7pahFaNgeiP8de";
#define SW_VERSION "1.0.1"
#define HW_VERSION "1.0"
#define MODEL "BatCC01"
#define EXPIRATION_TIME 3    //[sec] expire time in HA after lost data
//topics - published



// Initialize the client library
WiFiClient Wclient;
Hamqtt DevObj("BMS", nullptr,Hamqtt::PERTYPE_LOWSPEED,"BMS01","DK",SW_VERSION,"001",nullptr,HW_VERSION,nullptr,EXPIRATION_TIME);

void entCallBack(int indOfEnt, String &payload){
  /*DEBUG_LOG(true,"entCallBack:indOfEnt= ",indOfEnt);
  DEBUG_LOG_NOF(true,"payload= ",payload.c_str());
  DevObj.publishValue("Req_Power", payload.toFloat()); */
}


void Mqtt_init(){
    Hamqtt::init(&Wclient, BrokerIP,MqttUserName,MqttPass,MODEL);
    DEBUG_LOG0(true,"Mqtt init");
    //DevObj.registerEntity("sensor","Water_Temp",Hamqtt::PERTYPE_NORMAL,"temperature","°C",nullptr,"{{value_json.wTemp}}","mdi:thermometer");
    DevObj.registerSensorEntity("soc",Hamqtt::PERTYPE_LOWSPEED,"battery","%",nullptr,1,true);
    DevObj.registerSensorEntity("state",Hamqtt::PERTYPE_LOWSPEED,nullptr,"-",nullptr,1);
    //DevObj.registerSensorEntity("alert",Hamqtt::PERTYPE_NORMAL,nullptr);
    DevObj.registerSensorEntity("warning",Hamqtt::PERTYPE_LOWSPEED,nullptr,"-",nullptr,1,true);  
    DevObj.registerSensorEntity("ubat",Hamqtt::PERTYPE_LOWSPEED,"voltage","V",nullptr,1,true);  
    DevObj.registerSensorEntity("ibat",Hamqtt::PERTYPE_NORMAL,"current","A");  
    DevObj.registerSensorEntity("tbat",Hamqtt::PERTYPE_LOWSPEED,"temperature","°C",nullptr,TBMSCom::Data::MODUL_NR,true);      
    //DEBUG_LOG0(true,"registerEntity");
    
}

void Mqtt_loopQ(void *){
  
  TBMSCom::Data bmsData;

  TBMSComobj.getData(&bmsData);

  DevObj.publishValue("ibat", bmsData.i_bat); 
  DevObj.publishValue("state", (uint32_t)bmsData.state.all);       
}


void Mqtt_loopS(void *){
  
  TBMSCom::Data bmsData;

  TBMSComobj.getData(&bmsData);
  for(int ind=0;ind<TBMSCom::Data::MODUL_NR;ind++){
    DevObj.writeValue("tbat", bmsData.t[ind],ind); 
  }
    
  DevObj.writeValue("ubat", bmsData.u_bat); 
  DevObj.writeValue("soc", bmsData.soc); 
  DevObj.writeValue("warning", bmsData.warning); 
    //DevObj.publishValue("alert", bmsData.); 
    //DevObj.publishValue("Water_Temp", testVal); 

    DevObj.startPublishing();
}

 //publishConfig(DEVICE_INDEX_NAME,"Req Power",nullptr,PUB_CONFTOPIC2,"W",nullptr,"{{value_json.reqPower}}",PUB_TOPIC2, SUB_TOPIC2);
