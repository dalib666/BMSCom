#include <ESP8266WiFi.h>
#include "tbmscom.hpp"
#include "hamqtt.hpp"
#include "Global.h" 
#include "DebugFnc.h"  

IPAddress BrokerIP(192,168,1,109);     
char MqttUserName[] = "homeassistant"; 
char MqttPass[] = "ol3uuNeek6ke7eich8aiva7ZoxoiVei1aiteith0aighae0ieP7pahFaNgeiP8de";
#define HW_VERSION "1.0"
#define MODEL "BatCC01"
#define EXPIRATION_TIME 3    //[sec] multiplier of parameter "perType" to compute expirtion time  in HA after lost data
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
    DevObj.registerSensorEntity("u_cellMax",Hamqtt::PERTYPE_LOWSPEED,"voltage","V",nullptr,1,true);     
    DevObj.registerSensorEntity("u_cellMin",Hamqtt::PERTYPE_LOWSPEED,"voltage","V",nullptr,1,true);   
    //DEBUG_LOG0(true,"registerEntity");    
}

static bool PublisMqttdata=false;
void Mqtt_loopQ(void *){
  if(millis() > 20000ul || PublisMqttdata) {    // to wait some time for valid data from BMS
    PublisMqttdata=true;
    TBMSCom::Data bmsData;

    TBMSComobj.getData(&bmsData);

    DevObj.publishValue("ibat", bmsData.i_bat); 
    DevObj.publishValue("state", (uint32_t)bmsData.state);   
  }    
}


void Mqtt_loopS(void *){
  if(PublisMqttdata) {
    TBMSCom::Data bmsData;

    TBMSComobj.getData(&bmsData);
    for(int ind=0;ind<TBMSCom::Data::MODUL_NR;ind++){
      DevObj.writeValue("tbat", bmsData.t[ind],ind); 
    }
    DevObj.writeValue("u_cellMin", bmsData.u_cellMin);     
    DevObj.writeValue("u_cellMax", bmsData.u_cellMax); 
    DevObj.writeValue("ubat", bmsData.u_bat); 
    DevObj.writeValue("soc", (uint32_t)bmsData.soc); 
    DevObj.writeValue("warning", bmsData.warning); 
      //DevObj.publishValue("alert", bmsData.); 
      //DevObj.publishValue("Water_Temp", testVal); 

      DevObj.startPublishing();
  }    
}


bool MQTT_Check(){
  //if(!Params::isValid())
  //  return true;
  unsigned long lastConTime=DevObj.connected();
  long deltaTime= millis() - lastConTime;
  DebugCntr=(int)deltaTime;
  if((millis() < lastConTime) || (deltaTime < 10000l))
    return true;
  else
    return false;

}

 //publishConfig(DEVICE_INDEX_NAME,"Req Power",nullptr,PUB_CONFTOPIC2,"W",nullptr,"{{value_json.reqPower}}",PUB_TOPIC2, SUB_TOPIC2);
