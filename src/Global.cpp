/* ========================================================================== */
/*                                                                            */
/*   Global.c                                                           */
/*   (c) 2018 D.Kuchta                                                          */
/*                                                                            */
/*   Description                                                              */
/*                                                                            */
/* ========================================================================== */



#include "Global.h"

long LoopCntr=0;
long LowLoopCntr=0;
long HighLoopCntr=0;
long ExLowLoopCntr=0;
int DebugCntr=0;
int DebugCntr1=0;
bool LifeLed=false;
bool CritErrorLed=false;
bool Rele_heating=false;
bool Rele_ventilating=false;
int FT_TempControl=0;    // Function test of temp. regulation
int FT_VentControl=0;    // Function test fo ventilation
/*
void abort_(){ //char * errmsg){
  //Serial.println(errmsg);
  while(true);
}*/


