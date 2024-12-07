/* ========================================================================== */
/*                                                                            */
/*   Global.h                                                              */
/*   (c) 2012 Author                                                          */
/*                                                                            */
/*   Description                                                              */
/*                                                                            */
/* ========================================================================== */

#ifndef GLOBAL_H
#define GLOBAL_H
#include <Arduino.h>
#include <ESP8266TimerInterrupt.h>
#include "tbmscom.hpp"


#define SW_VERSION  "V1.6.2"
#define COMP_DATE __DATE__;
#define COMP_TIME __TIME__;
#define MIDLELOOP_TIME 1000
#define LOWLOOP_TIME  20000           //[ms]
#define HIGHLOOP_TIME 100           //[ms]
//#define EXLOWLOOP_TIME  60000ul  //60000ul     //[ms]
#define STATUSLOOP_TIME HIGHLOOP_TIME       //[ms]
#ifdef DEBUG_MODE
  #define DEBUG_PART(cs) cs
#else
  #define DEBUG_PART(cs)  (void)0
#endif
#define CPUINTERFACE_SPEED 9600
#define HWTIMER_PERIOD 2  //[ms] period of HW timer ISR
#define START_TIME_OF_MQTT_MONITORING 60000ul //[ms]

#define RED_LED_PIN           2     // internal LED, RED led
#define GREEN_LED_PIN         16    // internal LED, GREEN led
#define RELE_HEATING_PIN      13   
#define RELE_VENTILATION_PIN  12
#define RELE_MAIN_PIN         14    // main switcher - serial contact
//#define RELE_RESERVE_PIN      9    // reserve switcher - do not use conflict with FLASH

#define LED_ACTIVELEV false
#define RELE_ACTIVELEV false

#define WIFICONF_BUT_PIN      0     // button for WIFI conf.
#define WIFICONF_BUT_ACTLEV   false
/*#define STATUS_LED_PIN      12    // operational status LED
#define WIFISTATUS_LED_PIN  16    // wifi status LED

#define LED_ACTIVELEV false
*/
#define TEMP_REG_HTEMP  13.0f // C 
#define TEMP_REG_HYST   2.0f // C
//#define VTEMP_REG_HTEMP  18.0f // C - requested heating temperature
//#define VTEMP_REG_HYST   1.0f // C  - hysterezy 
#define VTEMP_REG_CTEMP  26.0f // C - requested cooling temperature
#define VTEMP_REG_CHYST   1.0f // C - hysterezy

#define CELL_CRIT_VALUE 3.33f  //[V] below value is heating off



extern int DebugCntr;
extern int DebugCntr1;
extern long LoopCntr;
extern long LowLoopCntr;
extern long HighLoopCntr;
extern long ExLowLoopCntr;
extern int DebugCntr;
extern int PingErrCntr;
extern int Loop_runs_perSec;
extern bool LifeLed;
extern bool CritErrorLed;

extern bool Rele_heating;
extern bool Rele_ventilating;
extern int FT_TempControl;   
extern int FT_VentControl;  

#define BUFFER_LEN 64
extern int RX_buffer_IND;
extern uint8_t RX_buffer[];
extern uint32_t RX_time[];
extern bool startMes;

extern TBMSCom TBMSComobj; 
extern ESP8266Timer ITimer;


#define PIN_BUTTON 0
#define PIN_BUTTON_ACTLEV false


#endif //  GLOBAL_H
