/* ========================================================================== */
/*                                                                            */
/*   Params.cpp                                                              */
/*                                                                            */
/*                                                                            */
/*   Description:                                                             */
/*               Parmaetrization for actual deployment                        */
/* ========================================================================== */


#include "Params.h" 
#include <ArduinoJson.h>
#include <LittleFS.h>
#include "Global.h"

ParamsDef Params;


float ParamsDef::Heat_ReqTemp;      // Req temperature for heating battery
float ParamsDef::Cool_ReqTemp;      // Req temperature for cooling battery
//MQTT interface
IPAddress ParamsDef::MqttBrokerIP;   
String ParamsDef::MqttPass;
String ParamsDef::MqttUserName;
String ParamsDef::DevName;           // identification of device in system 
String ParamsDef::Identifier;        // unique identification of device
String ParamsDef::Model;     

const ParamsDef::ParamRecord ParDB[]={
  {"Heat_ReqTemp",&ParamsDef::Heat_ReqTemp,ParamsDef::DTYPE_FLOAT},
  {"Cool_ReqTemp",&ParamsDef::Cool_ReqTemp,ParamsDef::DTYPE_FLOAT},
  {"MqttPass",&ParamsDef::MqttPass,ParamsDef::DTYPE_STRING},
  {"MqttUserName",&ParamsDef::MqttUserName,ParamsDef::DTYPE_STRING},
  {"DevName",&ParamsDef::DevName,ParamsDef::DTYPE_STRING},
  {"Identifier",&ParamsDef::Identifier,ParamsDef::DTYPE_STRING},
  {"Model",&ParamsDef::Model,ParamsDef::DTYPE_STRING},
  {"MqttBrokerIP",&ParamsDef::MqttBrokerIP,ParamsDef::DTYPE_IP}
};

ParamsDef::ParamsDef():ParamBase("/Params.json",ParDB,sizeof(ParDB)/sizeof(ParamsDef::ParamRecord)){}

void ParamsDef::initToDef(){
  // control alg. parameters
    Heat_ReqTemp =TEMP_REG_HTEMP;
    Cool_ReqTemp =VTEMP_REG_CTEMP;
    //MQTT interface
    MqttBrokerIP.clear();
    MqttPass.clear();
    MqttUserName.clear();
    DevName.clear();
    Identifier.clear();
    Model.clear();
}

bool ParamsDef::isValid(){
  return true; //TBD
}