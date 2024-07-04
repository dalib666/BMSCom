/* ========================================================================== */
/*                                                                            */
/*   GlStatus.h                                                                 */
/*   (c) 2019                                                                 */
/*                                                                            */
/*   Description                                                              */
/*                                                                            */
/* ========================================================================== */

#ifndef GlStatus_H
#define GlStatus_H
#include "Arduino.h"
#include "Global.h"



#include "DebugFnc.h"   // must be after definition of DEBUG_MODE macro 

#pragma once

class GLStatusDef{
  public:
    static const int TEXTLEN=100;
    static const int MAX_ER_TEX=1; //TODO only now because of low memory
    /**
     *  @param refPer [ms]refresh func. calling period 
     */
    GLStatusDef(int refPer){
      m_led_period=LED_PERIOD/refPer;
      m_redCounter=m_led_period;
      m_greenCounter=m_led_period;
      m_redLed=false;
      m_greenLed=true;
      mStatus = STATUS_INIT;
      gerror.all=0;
      gwarning.all=0;
      m_lastETextRead=0;
    }
 
    enum  StatusLev{
      STATUS_INIT,
      STATUS_OK,        // all is OK
      STATUS_WARNING,   // some short-time error happened 
      STATUS_ERROR,     // some active error limits some function 
    };

    struct ErBits{
      uint8_t wifi_com:1;      // not communication with ESP CPU
      uint8_t hRegDeact:1;  // not valid Real time
      uint8_t defPar:1;      // parameters are in default state
    };

    union StatusEr{
      ErBits bits;
      uint8_t all;
    };

    struct WBits{
      uint8_t vRegDeact:1;   // not connected into WIFI
    };

    union StatusW{
      WBits bits;
      uint8_t all;
    };

    StatusW   gwarning; //warnings evulated by global modul
    StatusEr  gerror;   // erros evulated by global modul;
    StatusLev mStatus;  
  



  bool oper(){return((mStatus==STATUS_OK)||(mStatus==STATUS_WARNING));} //Info about operational state
  const char * getMStatusStr(){
    static const char * str;
    switch(mStatus){
      case   STATUS_INIT: str="Init"; break;
      case   STATUS_OK: str="OK"; break;
      case   STATUS_WARNING: str="Warning"; break;//Short-time error happened from last reset or other not serious error is active
      case   STATUS_ERROR:  str="Error";    break;//Permanent error with impact to some function is active

    }
    return str;
  }
  const char * getErrorStr(){
    if(gerror.bits.hRegDeact)
      return "Heating regulation is deactivated - heating is switched-off !";
    if(gerror.bits.wifi_com)
      return "Not connected into WIFI";
    return "";
  }

  const char * getWarningStr(){
    if(gwarning.bits.vRegDeact)
      return "Ventilation regulation deactivated - ventilation is switched-on permanently !";  
    return "";  
  }
  /*
  void setSafe(){mStatus=STATUS_SAFE_STATE;}
  void setTest(bool val){
    if(val)
      mStatus=STATUS_TEST;
    else
      mStatus=STATUS_OPER; 
  }
  */
  

  void refresh();

  private:
    void procesData();
    //void proceOtherDev();
    //void addErrText(bool cond,const char * text,...);  
    //void addWarText(bool cond,const char * text,...);  
    static const int LED_PERIOD=1000;   //[ms]
    int m_led_period;
    int m_redCounter;
    int m_greenCounter;
    bool m_redLed;
    bool m_greenLed;
    //char  m_ErStr[MAX_ER_TEX][TEXTLEN];
    //char  m_WarStr[TEXTLEN];  
    int m_lastETextRead;
    int m_lastETextwrite;

    //bool m_builtInLed;   
};

extern GLStatusDef Status;



#endif //  GLStatus_H
