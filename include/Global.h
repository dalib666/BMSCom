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




//#define BUILTIN_LED_PIN     2   // internal LED
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



#endif //  GLOBAL_H
