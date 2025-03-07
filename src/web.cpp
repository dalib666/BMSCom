/* ================================================================================ */
/*                                                                                  */
/*   web.cpp                                                                        */
/*                                                                                  */
/*                                                                                  */
/*   Description:                                                                   */
/*               Parmaetrization for actual deployment  handling of Html requests   */
/* ================================================================================ */

#include "web.h"
#include "Global.h"
#include "tbmscom.hpp"
#include "GlStatus.h" 
#include "Params.h" 
#include <ESP8266HTTPUpdateServer.h>
#include <LittleFS.h>
#include "Global.h"
//#include <FS.h>

const char Update_page[] PROGMEM = R"=====(
  <form method="post" enctype="multipart/form-data">
      <input type="file" name="name">
      <input class="button" type="submit" value="Upload">
  </form>
)=====";

ESP8266WebServer WebServer(80);
ESP8266HTTPUpdateServer HttpUpdater;

void handleNotFound();
void handleRoot();
void handle_preupdate();
void handle_log();
void handleDiagData();
void handle_reset();
void handle_test_temp_control();
void handle_test_vent_control();
void handle_showP();
void handleFileUpload();                // upload a new file to the SPIFFS
/*
void handleCmd();
void handle_ext_info();
void handle_SaveP();
void handle_setP();
void handle_setDefP();


void handle_help();
void sendFilePage(String pagefileName);
*/
bool handleFileRead(String path);

void web_init() {

  
    WebServer.on("/", handleRoot);
    WebServer.onNotFound(handleNotFound);
    WebServer.on("/preupdate",  handle_preupdate);  
    WebServer.on("/log",  handle_log);
    WebServer.on("/diag",  handleDiagData);  
    WebServer.on("/reset",  handle_reset);
    WebServer.on("/tTCtrl", handle_test_temp_control);
    WebServer.on("/tVCtrl", handle_test_vent_control);
    WebServer.on("/showP",  handle_showP); 
  
    WebServer.on("/upload", HTTP_GET, []() {                  // if the client requests the upload page
      String s = Update_page;
      WebServer.send(200, "text/html", s);
    });

    WebServer.on("/upload", HTTP_POST,                        // if the client posts to the upload page
      [](){ WebServer.send(200); },                           // Send status 200 (OK) to tell the client we are ready to receive
      handleFileUpload                                       // Receive and save the file
    );

    /*
    WebServer.on("/help", handle_help);
    WebServer.on("/cmd", handleCmd);

    WebServer.on("/extinfo", handle_ext_info);

    WebServer.on("/saveP",  handle_SaveP);
    WebServer.on("/setP",  handle_setP);   

    WebServer.on("/setDefP",  handle_setDefP); 
  
    */

    HttpUpdater.setup(&WebServer);  // use "/update" dir for update oper
    
    WebServer.begin();
}



void handleNotFound(){
  String uri = WebServer.uri();
  if (LittleFS.exists(uri)){
    handleFileRead(uri);    // try if uri does not mean direct file
    return;
  }

  String message = "File Not Found\n\n";
  message += "URI: ";
  message += WebServer.uri();
  message += "\nMethod: ";
  message += (WebServer.method() == HTTP_GET)?"GET":"POST";
  message += "\nArguments: ";
  message += WebServer.args();
  message += "\n";
  for (uint8_t i=0; i<WebServer.args(); i++){
    message += " " + WebServer.argName(i) + ": " + WebServer.arg(i) + "\n";
  }
  WebServer.send(404, "text/plain", message);
}

void addOneLine(String  &mes,const char * name,int nameInd,float value, const char * unit=nullptr){
  if(unit==nullptr)
    unit="-";  
  mes+=name + (String)nameInd + (String)" = "+ value + (String)" " + (String)unit + (String)"\n";
}
template<class T> void addOneLine(String  &mes,const char * name,T value, const char * unit=nullptr){
  if(unit==nullptr)
    unit="-";  
  mes+=name + (String)" = "+ value + (String)" " + (String)unit + (String)"\n";
}
template void addOneLine(String  &mes,const char * name,float value, const char * unit=nullptr);
template void addOneLine(String  &mes,const char * name,uint16_t value, const char * unit=nullptr);

void handleRoot() {
  String message = "BMSCom diag status:\n";
  message +=SW_VERSION; message +="\n";
  message +="Comp - date/time= "; message +=COMP_DATE; message +="/"; message +=COMP_TIME; message +="\n";

  

  message +="\n";
  addOneLine(message, "GlStatus",Status.mStatus, "");
  addOneLine(message, "GlError",Status.getErrorStr(), "");
  addOneLine(message, "GlWarning",Status.getWarningStr(), "");
  addOneLine(message, "BMS-Warning",TBMSComobj.m_data.warning, "");
  addOneLine(message, "Rele_heating",Rele_heating, "");
  addOneLine(message, "Rele_ventilating",Rele_ventilating, "");
  addOneLine(message, "u_min",TBMSComobj.m_data.u_min, "V");
  addOneLine(message, "u_max", TBMSComobj.m_data.u_max, "V");
  addOneLine(message, "i_max",TBMSComobj.m_data.i_max, "A");
  addOneLine(message, "i_bat",TBMSComobj.m_data.i_bat, "A");
  addOneLine(message, "ch_dsch_state",TBMSComobj.m_data.ch_dsch_state);
  addOneLine(message, "lcd_state",TBMSComobj.m_data.lcd_state);
  addOneLine(message, "u_bat",TBMSComobj.m_data.u_bat, "V");
  addOneLine(message, "t_max",TBMSComobj.m_data.t_max, "C");
  addOneLine(message, "cell_nr",TBMSComobj.m_data.cell_nr);
  addOneLine(message, "u_cellMax",TBMSComobj.m_data.u_cellMax);
  addOneLine(message, "u_cellMin",TBMSComobj.m_data.u_cellMin); 
  addOneLine(message, "t_cellMin",TBMSComobj.m_data.t_cellMin);  
  addOneLine(message, "ah",TBMSComobj.m_data.ah);

  for(int ind=0; ind < TBMSCom::Data::MODUL_NR; ind++ ){
    addOneLine(message, "t",ind+1,TBMSComobj.m_data.t[ind],"C");
  }

  addOneLine(message, "soc=",TBMSComobj.m_data.soc,"%");
  
  //TBMSComobj.m_data.state.bit.unknown1=1;
  addOneLine(message, "state=",TBMSComobj.m_data.state);
  addOneLine(message, "discharged=",TBMSComobj.m_data.discharged);


  for(int ind=0; ind < TBMSCom::Data::U_CELL_NR; ind++ ){

    addOneLine(message, "u_cell",ind+1,TBMSComobj.m_data.u_cell[ind] * 1000,"mV");
  }
  message +="\n";
  
 // message +="PingErrCntr = "; message +=PingErrCntr; message +="\n"; 

 
  WebServer.send(200, "text/plain", message);
}


void handle_log(){
  String message = "Debug Log:\n";
  for(int index=0; index < RX_buffer_IND;index++){
    message+=(String)RX_time[index] + (String)" | " + String(RX_buffer[index],HEX) + (String) "\n";
  }
  message+= (String)"RX_buffer_IND = " + RX_buffer_IND+ (String) "\n";
  WebServer.send(200, "text/plain", message);    
  while(Serial.available())
    Serial.read();
  RX_buffer_IND=0;
  startMes = false;
  
}


void handle_preupdate(){
  
  ITimer.disableTimer();
  String message = "Ready for update \n"; 
  WebServer.send(200, "text/plain", message);    
  
}



void handleDiagData() {
 
  String message = "Debug Info: \n";
  message +="========================================================\n";  
  message +="Wifi - RSSI: "; message += WiFi.RSSI(); message +="dBm \n";
  message+="Memory info:\n";
  message+="Free heap memory:"; message+=ESP.getFreeHeap(); message +="\n";
  message+="Heap fragmentation:"; message+=ESP.getHeapFragmentation(); message +="\n\n"; //needs higher Core version

  message+="Reset reason:"; message+=ESP.getResetReason(); message +="\n\n";


  message+= "frameErCntr = ";
  for(int i=0;i<TBMSCom::Data::FRAMES_NR;i++){
    message+= (String)TBMSComobj.m_data.frameErCntr[i] + (String)" | ";
  }
  message +="\n";
  message +="========================================================\n";
  message+= "frameRxCntr = ";
  for(int i=0;i<TBMSCom::Data::FRAMES_NR;i++){
    message+= (String)TBMSComobj.m_data.frameRxCntr[i] + (String)" | ";
  }
  message +="\n";
  message +="========================================================\n";
 message +="RxBuffer = "; 
  int framLen=TBMSCom::RXBUFER_LEN;
  byte * bufPtr=TBMSComobj.rxBuff;
  for(int i=0;i< framLen;i++){
    message +=  String(bufPtr[i],HEX);
    message +=" , ";
  }
  message +="\n";  
  message +="Loop_runs_perSec = "; message +=Loop_runs_perSec; message +="\n";
  message +="DebugCntr = "; message +=DebugCntr; message +="\n";
  message +="DebugCntr1 = "; message +=DebugCntr1; message +="\n";
  message +="LoopCntr = "; message +=LoopCntr; message +="\n";   
  message +="RunTime = "; message +=(float)millis()/1000.0f; message +="\n";     
  message +="LowLoopCntr = "; message +=LowLoopCntr; message +="\n"; 
  message +="HighLoopCntr = "; message +=HighLoopCntr; message +="\n";

  
  WebServer.send(200, "text/plain", message);
}


void handle_reset(){
  String message = "BMSCom is reseted \n"; 
  WebServer.send(200, "text/plain", message);  
  delay(1000); // to give time to send responce
  ESP.restart();
}

void handle_test_temp_control(){
 
  String argStr=WebServer.arg("on");
  String message = "Temp reg. test is:"; 

  if(argStr.length()>0){
    int state_i = argStr.toInt();
    FT_TempControl=state_i;
    message+= (state_i)? "on":"off";
  }
  else
    message+= " error in input";


  WebServer.send(200, "text/plain", message);
}

void handle_test_vent_control(){
 
  String argStr=WebServer.arg("on");
  String message = "Ventilation reg. test is:"; 

  if(argStr.length()>0){
    int state_i = argStr.toInt();
    FT_VentControl=state_i;
    message+= (state_i)? "on":"off";
  }
  else
    message+= " error in input";


  WebServer.send(200, "text/plain", message);
}

void handle_showP(){
  String message = "List of params \n"; 
  for(int i=0; i< Params.getDBSize(); i++){
    message+=Params.getNameByInd(i); message += " = "; message +=Params.readParam(i); message +="\n";    
  }
  WebServer.send(200, "text/plain", message);   
}

void handleFileUpload(){ // upload a new file to the SPIFFS
 static File fsUploadFile; 
  HTTPUpload& upload = WebServer.upload();
  if(upload.status == UPLOAD_FILE_START){
    String filename = upload.filename;
    if(!filename.startsWith("/")) filename = "/"+filename;
    Serial.print("handleFileUpload Name: "); Serial.println(filename);
    delay(100);  // to be able send info before reset if will happen
    fsUploadFile = LittleFS.open(filename, "w");            // Open the file for writing in LittleFS (create if it doesn't exist)
    filename = String();
  } else if(upload.status == UPLOAD_FILE_WRITE){
    if(fsUploadFile)
      fsUploadFile.write(upload.buf, upload.currentSize); // Write the received bytes to the file
  } else if(upload.status == UPLOAD_FILE_END){
    if(fsUploadFile) {                                    // If the file was successfully created
      fsUploadFile.close();                               // Close the file again
      Serial.print("handleFileUpload Size: "); Serial.println(upload.totalSize);
      WebServer.send(200, "text/plain", "File is uploaded.");
      //WebServer.sendHeader("Location","/success.html");      // Redirect the client to the success page
      //WebServer.send(303);
    } else {
      WebServer.send(500, "text/plain", "500: couldn't create file");
    }
  }
}


String getContentType(String filename) { // convert the file extension to the MIME type
  if (filename.endsWith(".html")) return "text/html";
  else if (filename.endsWith(".css")) return "text/css";
  else if (filename.endsWith(".js")) return "application/javascript";
  else if (filename.endsWith(".ico")) return "image/x-icon";
  else if (filename.endsWith(".gz")) return "application/x-gzip";
  return "text/plain";
}

bool handleFileRead(String path) { // send the right file to the client (if it exists)
  Serial.println("handleFileRead: " + path);
  if (path.endsWith("/")) path += "index.html";          // If a folder is requested, send the index file
  String contentType = getContentType(path);             // Get the MIME type
  String pathWithGz = path + ".gz";
  if (LittleFS.exists(pathWithGz) || LittleFS.exists(path)) { // If the file exists, either as a compressed archive, or normal
    if (LittleFS.exists(pathWithGz))                         // If there's a compressed version available
      path += ".gz";                                         // Use the compressed verion
    File file = LittleFS.open(path, "r");                    // Open the file
    size_t sent = WebServer.streamFile(file, contentType);    // Send it to the client
    file.close();                                          // Close the file again
    Serial.println(String("\tSent file: ") + path);
    return true;
  }
  Serial.println(String("\tFile Not Found: ") + path);   // If the file doesn't exist, return false
  return false;
}


/*
void handleCmd(){
  bool  cmdSuccess=false;
  String argStr;
  String power=WebServer.arg("Pact");
  String message = "SPM-CU return status of command :"; 
  if(power.length()>0){
    int pAct = power.toInt();
    CpuIf.m_uno.Test_Pact=(pAct != 0)? true:false;
    CpuIf.m_uno.Pact_f=pAct;  
    cmdSuccess=true;
  }
  String wTempStr=WebServer.arg("WTemp");
  if(wTempStr.length()>0){
    int wtemp = wTempStr.toInt();
    //Boyler.setTemp(wtemp); TODO
    cmdSuccess=true;          
  }   
  String pwmStr=WebServer.arg("Pwm");
  if(pwmStr.length()>0){
    int pwm = pwmStr.toInt();
    //Boyler.setPwm(pwm); TODO
    cmdSuccess=true;          
  }
  argStr=WebServer.arg("UnoIDebug");
  if(argStr.length()>0){
    CpuIf.m_esp.intDebug = argStr.toInt();
    cmdSuccess=true;          
  }
  argStr=WebServer.arg("TestMode");
  if(argStr.length()>0){
    Status.setTest(argStr.toInt()>0);
    cmdSuccess=true;          
  }
     


  if(cmdSuccess)
    message += "successfull.";    
  else
    message += "unknown command.";

  WebServer.send(200, "text/plain", message);
}


void handle_ext_info(){

  FSInfo fs_info;
  SPIFFS.info(fs_info);
  
  String message = "File system info : \n"; 
  message+="totalBytes:";fs_info.totalBytes; message +="\n";
  message+="usedBytes:"; message+=fs_info.usedBytes; message +="\n";
  message+="blockSize:"; message+=fs_info.blockSize; message +="\n";
  message+="pageSize:"; message+=fs_info.pageSize; message +="\n";
  message+="maxOpenFiles:"; message+=fs_info.maxOpenFiles; message +="\n"; 
  message+="maxPathLength:"; message+=fs_info.maxPathLength; message +="\n";
  message += "\n"; 
  message += "Net info : \n"; 
  message += "IP = "; message += WiFi.localIP().toString();  message += "\n"; 
  unsigned char mac[6];
  WiFi.macAddress(mac);
  char macs[40];
  sprintf(macs, "%x:%x:%x:%x:%x:%x", mac[0],mac[1],mac[2],mac[3],mac[4],mac[5]);
  message += "MAC = "; message +=macs;  message += "\n"; 
  message += "RSSI:"; message +=WiFi.RSSI(); message += "\n"; 
  
  WebServer.send(200, "text/plain", message);
}


void handle_SaveP(){
  String message;
  if(Params_save()){
    message = "Parameters was saved. \n";    
  }
  else{
    message = "Parameters was not saved. Error occured. \n";      
  }
    
  WebServer.send(200, "text/plain", message);
}

void handle_setP(){

 String argStr;
 String message = "Uknown parametr ! \n"; 
 for(int i=0; i< NONVOL_PARAMS; i++){
   argStr=WebServer.arg(ParDB[i].name);
   if(argStr.length()>0){
      * ParDB[i].ptr = argStr.toFloat();
       message = "Parameters was set. \n";
      
      //to force change immediately in Uno
      CpuIf.m_esp.U_range_c=U_range_c;    
      CpuIf.m_esp.I_range_c=I_range_c;

      break;
   }
 }
  WebServer.send(200, "text/plain", message);
} 



void handle_setDefP(){
  Params_initToDef();
  handle_SaveP();
}

void handle_reset(){
  String message = "CU is reseted \n"; 
  WebServer.send(200, "text/plain", message);  
  delay(1000); // to give time to send responce
  ESP.restart();
}



void handle_help(){
//  sendFilePage("help.html");
  handleFileRead("/help.html");
}


void sendFilePage(String pagefileName){

  File pageF = SPIFFS.open(pagefileName, "r");
  if (!pageF) {
    String mess="Page ";
    mess+=pagefileName;
    mess+= " not found. \n";
    WebServer.send(200, "text/plain", mess);   
    return;
  }
  WebServer.streamFile(pageF, "text/html");
  pageF.close();  
}
 

String getContentType(String filename) { // convert the file extension to the MIME type
  if (filename.endsWith(".html")) return "text/html";
  else if (filename.endsWith(".css")) return "text/css";
  else if (filename.endsWith(".js")) return "application/javascript";
  else if (filename.endsWith(".ico")) return "image/x-icon";
  else if (filename.endsWith(".gz")) return "application/x-gzip";
  return "text/plain";
}


*/