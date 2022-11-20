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

#define DEBUG_MODE

#define SW_VERSION  "V1.0.0"
#define COMP_DATE __DATE__;
#define COMP_TIME __TIME__;
#define LOWLOOP_TIME  1000           //[ms]
#define HIGHLOOP_TIME 200           //[ms]
#define EXLOWLOOP_TIME  60000ul  //60000ul     //[ms]
#ifdef DEBUG_MODE
  #define DEBUG_PART(cs) cs
#else
  #define DEBUG_PART(cs)  (void)0
#endif
#define CPUINTERFACE_SPEED 9600
#define HWTIMER_PERIOD 2  //[ms] period of HW timer ISR


#define BUILTIN_LED_PIN     2   // internal LED, RED led
#define GREEN_LED_PIN     2   // internal LED, GREEN led
#define RELE_HEATING_PIN      14   
#define RELE_VENTILATION_PIN  12
#define RELE_MAIN_PIN         13  // main switcher - serial contact
#define RELE_RESERVE_PIN      10  // reserve switcher - serial contact

#define LED_ACTIVELEV false
#define RELE_ACTIVELEV false

/*#define STATUS_LED_PIN      12    // operational status LED
#define WIFISTATUS_LED_PIN  16    // wifi status LED
#define WIFICONF_BUT_PIN  4       // button for WIFI conf.
#define LED_ACTIVELEV false
*/

extern int DebugCntr;
extern long LoopCntr;
extern long LowLoopCntr;
extern long HighLoopCntr;
extern long ExLowLoopCntr;
extern int DebugCntr;
extern int PingErrCntr;

#define BUFFER_LEN 64
extern int RX_buffer_IND;
extern uint8_t RX_buffer[];
extern uint32_t RX_time[];
extern bool startMes;

extern TBMSCom TBMSComobj; 
extern ESP8266Timer ITimer;

#endif //  GLOBAL_H
