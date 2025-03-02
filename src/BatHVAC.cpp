#include <Arduino.h>
#include "Global.h" 
#include "GlStatus.h" 
#include "BatHVAC.hpp"
#include "HFunc.hpp"  
#include "Params.h"
#include "DebugFnc.h"

namespace BatHVAC{

    void hystReg(bool heatMode,float in,float low_lev,float high_lev,int out_pin, bool & output){
        if(!output){
        if((heatMode && in < low_lev) || (!heatMode && in > high_lev)){
            digitalWrite(out_pin, RELE_ACTIVELEV);  
            output=true;
        }
        }
        else{
        if((heatMode && in > high_lev) || (!heatMode && in < low_lev)){
            digitalWrite(out_pin, !RELE_ACTIVELEV);  
            output=false;
        }
        }
    }

    void temp_control(){  


        float actTemp=(FT_TempControl!=0)? FT_TempControl:TBMSComobj.m_data.t_cellMin;

        if(isTempValid(actTemp) && (TBMSComobj.m_data.u_cellMin > CELL_CRIT_VALUE) && (TBMSComobj.m_data.u_cellMin < 5.0f)){
            //DEBUG_LOG(true,"actTemp=",actTemp);
            hystReg(true,actTemp,Params.Heat_ReqTemp-TEMP_REG_HYST/2.0f,Params.Heat_ReqTemp+TEMP_REG_HYST/2.0f,RELE_HEATING_PIN,Rele_heating);
            Status.gerror.bits.hRegDeact=false;
        }
        else{
            //DEBUG_PART(Serial.println("Regulation is off"));
            //invalid input data or other cond. not met
            Status.gerror.bits.hRegDeact=false;  
            Rele_heating=false;
            digitalWrite(RELE_HEATING_PIN, !RELE_ACTIVELEV);  
        }
    }

    void vent_control(){


        //float actMinTemp=(FT_VentControl!=0)? FT_VentControl:TBMSComobj.m_data.t_cellMin;   //for test purposes can be set test values
        float actMaxTemp=(FT_VentControl!=0)? FT_VentControl:TBMSComobj.m_data.t_cellMax;   

        if(Rele_heating){
            digitalWrite(RELE_VENTILATION_PIN, !RELE_ACTIVELEV);    
            Rele_ventilating=false;
            return;     // if heating is on - always disable venting
        }

        if( isTempValid(actMaxTemp)){

            //DEBUG_PART(Serial.println("Vent regulation is runing"));
            // ventilation is used only for cooling
            //hystReg(true,actMinTemp,VTEMP_REG_HTEMP-VTEMP_REG_HYST/2.0f,VTEMP_REG_HTEMP+VTEMP_REG_HYST/2.0f,RELE_VENTILATION_PIN,Rele_ventilating);
            hystReg(false,actMaxTemp,Params.Cool_ReqTemp-VTEMP_REG_CHYST/2.0f,Params.Cool_ReqTemp+VTEMP_REG_CHYST/2.0f,RELE_VENTILATION_PIN,Rele_ventilating);
            
            Status.gwarning.bits.vRegDeact=false;
        }
        else{
            //DEBUG_PART(Serial.println("Regulation is off - ventilation is on all the time."));
            //invalid input data or other cond. not met
            Status.gwarning.bits.vRegDeact=true;
            Rele_ventilating=true;
            digitalWrite(RELE_VENTILATION_PIN, RELE_ACTIVELEV);  
        }
    
    }

    
    void process(void *){
        temp_control();
        vent_control();
    }
}