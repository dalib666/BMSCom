/* ========================================================================== */
/*                                                                            */
/*   Global.c                                                           */
/*   (c) 2018 D.Kuchta                                                          */
/*                                                                            */
/*   Description                                                              */
/*                                                                            */
/* ========================================================================== */



#include "Global.h"
#include "GlStatus.h"
#include "assert.h"




GLStatusDef Status(STATUSLOOP_TIME);



void GLStatusDef::refresh(){
    procesData();
    //gener main status
      if(gerror.all)
        mStatus = STATUS_ERROR;    
      else 
        if(gwarning.all){
          mStatus = STATUS_WARNING;
        }
        else{            
          mStatus = STATUS_OK;
        } 
    //gener error RED LED          
    m_redCounter--;
    m_greenCounter--;
    if((m_redCounter ==0)||(m_greenCounter ==0)){   
      if(m_redCounter ==0)
        m_redLed=!m_redLed;
      if(m_greenCounter ==0){
        m_greenLed=!m_greenLed;
      }
      switch(mStatus){
        case STATUS_OK:
          m_redLed=!LED_ACTIVELEV;
          m_greenCounter=m_led_period/2;
          break;
          
        case STATUS_WARNING:
          if(m_redCounter ==0)
            m_redCounter  =m_led_period/4;
          if(m_greenCounter ==0)
            m_greenCounter=m_led_period/2;
          break;
    
        case STATUS_ERROR:
          if(m_redCounter ==0)
            m_redCounter  =m_led_period/2;
          if(m_greenCounter ==0)
            m_greenCounter=m_led_period/2;
          break;

        case STATUS_INIT:
        //TODO
        break;  

      }
      digitalWrite(RED_LED_PIN, m_redLed); 
      digitalWrite(GREEN_LED_PIN,m_greenLed);

      if((m_redCounter ==0) && (m_redLed != LED_ACTIVELEV)){
        m_redCounter  =m_led_period - m_redCounter  ; // set time of switche-off
      } 

      if((m_greenCounter ==0) && (m_greenLed != LED_ACTIVELEV)){
        m_greenCounter  =m_led_period - m_greenCounter  ; // set time of switche-off
      } 
    }

  }
  






void GLStatusDef::procesData(){




}








