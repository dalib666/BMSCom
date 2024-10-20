#include <ESP8266WiFi.h>
#include "tbmscom.hpp"
#include "hamqtt.hpp"
#include "Global.h" 
#include "DebugFnc.h"  
#include "GlStatus.h" 
#include "Params.h" 
   
#define HW_VERSION "1.0"
#define EXPIRATION_TIME 3    //[sec] multiplier of parameter "perType" to compute expirtion time  in HA after lost data
//topics - published
char ConfURL[]="http://192.168.1.111";
extern ESP8266Timer ITimer;

// Initialize the client library
WiFiClient Wclient;
#ifdef TEST_COMP
  const char * DevIndex="_t";
#else
  const char * DevIndex=nullptr;
#endif
Hamqtt DevObj(Params.DevName.c_str(), DevIndex,Hamqtt::PERTYPE_LOWSPEED,Params.Model.c_str(),"DK",SW_VERSION,Params.Identifier.c_str(),ConfURL,HW_VERSION,nullptr,EXPIRATION_TIME); //

void entCallBack(int indOfEnt, String &payload){
  //DEBUG_LOG(true,"entCallBack:indOfEnt= ",indOfEnt);
  DEBUG_LOG(true,"entCallBack:payload= ",payload.c_str());

  const char * entName=DevObj.getEntName(indOfEnt);
  if(strcmp(entName,"Restart")==0){
    ESP.restart();
    return;
  } 
  /* part of parameters saving */
  if((strcmp(entName,"Heat_ReqTemp")==0) ||(strcmp(entName,"Cool_ReqTemp")==0)){
    int parInd=Params.getIndByName(entName);
    ITimer.disableTimer();
    Params.saveParByInd(parInd,payload.c_str());
    DevObj.publishValue(entName,Params.readParam(parInd).toFloat());
    delay(10);
    ITimer.enableTimer();
    return;
  }   

   // DevObj.publishSwitch("State", Relay,false); 
    
}



void Mqtt_init(){
    

    if(!Params.isValid())
      return;

    String confURL="http://";
    confURL+=WiFi.localIP().toString();
    DevObj.setDynamic(confURL.c_str());
    Hamqtt::init(&Wclient, Params.MqttBrokerIP,Params.MqttUserName.c_str(),Params.MqttPass.c_str(),Params.Identifier.c_str());
    DEBUG_LOG0(true,"Mqtt init");
    if(Hamqtt::is_connected()){
      DevObj.registerSensorEntity("soc",Hamqtt::PERTYPE_LOWSPEED,"battery","%",nullptr,1,true);
      DevObj.registerSensorEntity("state",Hamqtt::PERTYPE_LOWSPEED,nullptr,"-",nullptr,1);
      //DevObj.registerSensorEntity("alert",Hamqtt::PERTYPE_NORMAL,nullptr);
      DevObj.registerSensorEntity("warning",Hamqtt::PERTYPE_LOWSPEED,nullptr,"-",nullptr,1,true);  
      DevObj.registerSensorEntity("ubat",Hamqtt::PERTYPE_LOWSPEED,"voltage","V",nullptr,1,true);  
      DevObj.registerSensorEntity("ibat",Hamqtt::PERTYPE_NORMAL,"current","A");  
      DevObj.registerSensorEntity("tbat",Hamqtt::PERTYPE_LOWSPEED,"temperature","°C",nullptr,TBMSCom::Data::MODUL_NR,true);      
      DevObj.registerSensorEntity("u_cellMax",Hamqtt::PERTYPE_LOWSPEED,"voltage","V",nullptr,1,true);     
      DevObj.registerSensorEntity("u_cellMin",Hamqtt::PERTYPE_LOWSPEED,"voltage","V",nullptr,1,true);   
      DevObj.registerSensorEntity("gl_status",Hamqtt::PERTYPE_LOWSPEED,nullptr,"-",nullptr,1,true);   

//    DevObj.registerBinSensorEntity("discharged",Hamqtt::PERTYPE_LOWSPEED,"battery","mdi:battery-alert-variant-outline");   

      DevObj.registerSwitchEntity("Heat",Hamqtt::PERTYPE_LOWSPEED,"switch","mdi:heating-coil",nullptr);  
      DevObj.registerSwitchEntity("Vent",Hamqtt::PERTYPE_LOWSPEED,"switch","mdi:hvac",nullptr);  

      DevObj.registerButtonEntity("Restart","restart",entCallBack,nullptr); 

      DevObj.registerNumberEntity("Heat_ReqTemp",Hamqtt::PERTYPE_LOWSPEED,nullptr,"°C","mdi:temperature-celsius",entCallBack,false,18,12);
      DevObj.writeValue("Heat_ReqTemp",Params.Heat_ReqTemp);
      DevObj.registerNumberEntity("Cool_ReqTemp",Hamqtt::PERTYPE_LOWSPEED,nullptr,"°C","mdi:temperature-celsius",entCallBack,false,29,24);
      DevObj.writeValue("Cool_ReqTemp",Params.Cool_ReqTemp);    
    }  
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
//    DevObj.publishBinSen("discharged",TBD); 
    
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
    DevObj.writeSwitch("Heat", Rele_heating); 
    DevObj.writeSwitch("Vent", Rele_ventilating);
    DevObj.writeValue("gl_status", (uint32_t)Status.mStatus);  
   
      //DevObj.publishValue("alert", bmsData.); 
      //DevObj.publishValue("Water_Temp", testVal); 

      DevObj.startPublishing();
  }    
}


bool MQTT_Check(){
  if(!Params.isValid())
    return true;
  unsigned long lastConTime=DevObj.connected();
  long deltaTime= millis() - lastConTime;
  //DebugCntr=(int)deltaTime;
  if((millis() < lastConTime) || (deltaTime < 10000l) || (lastConTime==0))  // return OK after start, if timeout is in limit or, 
                                                                            // connection never happened - situation, when params are not valid for broker
    return true;
  else
    return false;

}

 //publishConfig(DEVICE_INDEX_NAME,"Req Power",nullptr,PUB_CONFTOPIC2,"W",nullptr,"{{value_json.reqPower}}",PUB_TOPIC2, SUB_TOPIC2);
