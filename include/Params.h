/* ========================================================================== */
/*                                                                            */
/*   Params.h                                                                 */
/*                                                                            */
/*                                                                            */
/*   Description:                                                             */
/*               Parmaetrization for actual deployment                        */
/* ========================================================================== */

#ifndef Params_h 
#define Params_h

#include "ParamBase.hpp"



class ParamsDef: public ParamBase{

  public:
    ParamsDef();
    //Non-volatile paremeters
    //*****************************************
    static float Heat_ReqTemp;
    static float Cool_ReqTemp;

    //MQTT interface
    static IPAddress MqttBrokerIP;   
    static String MqttPass;
    static String MqttUserName;
    static String DevName;           // identification of device in system 
    static String Identifier;        // unique identification of device
    static String Model;             // model type identification

    virtual void initToDef();
  private:
    //static const ParamRecord ParDB[];

};
extern ParamsDef Params;
#endif