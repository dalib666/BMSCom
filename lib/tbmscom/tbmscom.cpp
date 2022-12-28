/* ========================================================================== */
/*                                                                            */
/*   tbmscom.cpp                                                              */                                                                     
/*                                                                            */
/*   Description:                                                             */
/*               Communication protocol with China TOPBMS                     */
/* ========================================================================== */

#include "tbmscom.hpp"
#include "assert.h"


#define b_to_w_be(buf,ind_) (buf[ind_]<<8 | buf[ind_+1])


extern int DebugCntr;


bool  TBMSCom::b_to_w_be_check(int framenumber,float & dataItem,uint8_t *buf,int ind,float scale,float rangeL,float rangeH){
    bool status = false;
    float value= scale * (float) (buf[ind]<<8 | buf[ind+1]);
    if((value >= rangeL)&&(value <= rangeH)){
        status=true;
        dataItem=value;
    }
    else
      m_data.frameErCntr[framenumber]++;
    return status;
}
bool  TBMSCom::b_to_w_be_check(int framenumber,uint16_t & dataItem,uint8_t *buf,int ind,uint16_t rangeL,uint16_t rangeH){
    bool status = false;
    uint16_t value= (buf[ind]<<8 | buf[ind+1]);
    if((value >= rangeL)&&(value <= rangeH)){
        status=true;
        dataItem=value;
    }
    else
      m_data.frameErCntr[framenumber]++;
    return status;
}

void  TBMSCom::init(HardwareSerial * uart, int baudRate){
    assert(uart !=nullptr);
    m_serialPtr=uart;
    m_baudRate=baudRate;
    m_seekFrameSpace=true;
    m_startFrame=false;
    m_startTime=0;
    m_frameTimoute=2 + ((uint32_t) Data::FRAMEDATALEN * m_baudRate)/10000;
    m_serialPtr->setTimeout(m_frameTimoute); 
    initAllData();

}


void TBMSCom::getData(Data * ptr){
    *ptr=m_data;
}


void TBMSCom::main(){   
    int indBuf=0; 
    int maxInd=0;
    if(m_startFrame && m_serialPtr->available()){
        m_startFrame = false;
        DebugCntr=2;
        int frameNumber=m_serialPtr->read();
        if(frameNumber >0 && frameNumber <= Data::FRAMES_MAXNR){           //> 0
            int readout = m_serialPtr->readBytes(rxBuff,Data::FRAMEDATALEN);
            m_seekFrameSpace=true;
            DebugCntr=3;
            if(readout == Data::FRAMEDATALEN && (millis() - m_startTime) < 20){    
                switch(frameNumber){

                    case 16:
                        m_data.ch_dsch_state=b_to_w_be(rxBuff,14);
                        m_data.lcd_state=b_to_w_be(rxBuff,16);
                        m_data.frameRxCntr[frameNumber]++;
                        m_data.frameRxTime[frameNumber]=millis();
                    break;    

                    case 11:
                    case 12:
                    case 13:
                    case 14:
                    case 15:
                        maxInd=(frameNumber-11)*9 + 57;
                        indBuf=0;
                        for(int ind= maxInd - 9; ind < maxInd; ind++){
                            if(m_data.cell_nr <= ind)
                                break;  // no more valid cells
                            if(!b_to_w_be_check(frameNumber,m_data.u_cell[ind],rxBuff,indBuf,0.001f,3.0f,4.3f)){
                                break;
                            }
                            indBuf+=2;
                        }  
                        m_data.frameRxCntr[frameNumber]++;    
                        m_data.frameRxTime[frameNumber]=millis(); 
                     break;

                    case 10:
                        indBuf=2;
                        for(int ind=4;ind<8;ind++){
                            //m_data.t[ind]=b_to_w_be(rxBuff,indBuf) * 0.1;
                            if(!b_to_w_be_check(frameNumber,m_data.t[ind],rxBuff,indBuf,0.1f,0.0f,40.0f))
                                break;
                            indBuf+=4;
                        }    
                        m_data.frameRxCntr[frameNumber]++;
                        m_data.frameRxTime[frameNumber]=millis();
                     break;

                    case 8:
                        if(!b_to_w_be_check(frameNumber,m_data.u_cell[45],rxBuff,0,0.001f,3.0f,4.3f))
                            break;
                        if(!b_to_w_be_check(frameNumber,m_data.u_cell[46],rxBuff,2,0.001f,3.0f,4.3f))
                            break;    
                        if(!b_to_w_be_check(frameNumber,m_data.u_cell[47],rxBuff,4,0.001f,3.0f,4.3f))
                            break;    
                        //m_data.soc=b_to_w_be(rxBuff,8);
                        if(!b_to_w_be_check(frameNumber,m_data.soc,rxBuff,8,0,100))
                            break;
                        //m_data.state.all=b_to_w_be(rxBuff,14);
                        if(!b_to_w_be_check(frameNumber,m_data.state.all,rxBuff,16,0,512))
                            break;

                        if(m_data.state.bit.overdischarged)
                            m_data.warning="Batery is very discharged";
                        else
                        if(m_data.state.bit.discharged)
                            m_data.warning="Batery is discharged";
                        else    
                        if(!m_data.state.bit.sw_state)
                            m_data.warning="Batery is disconnected from FV inverter";
                        else
                            m_data.warning="no warning";
                        /*    
                        uint16_t overloaded:1;  // indication of overloading
            uint16_t unknown:1;     // unknown var
            uint16_t overcurrent:1;  // indication of overcurrent      
            uint16_t MOS_tm:1;      // MOS temp. is under zero
            uint16_t res:7;*/
                        m_data.frameRxCntr[frameNumber]++;
                        m_data.frameRxTime[frameNumber]=millis();  
                    break;

                    case 1:
                        //m_data.u_min=b_to_w_be(rxBuff,0) * 0.01;   // [V] min voltage per cell (0.01) 
                        if(!b_to_w_be_check(frameNumber,m_data.u_min,rxBuff,0,0.01f,3.0f,4.2f))
                            break;
                        //m_data.i_max=b_to_w_be(rxBuff,2) * 0.1;   // [A] max charge/discharge current (0.1)
                        if(!b_to_w_be_check(frameNumber,m_data.i_max,rxBuff,2,0.1f,10,30))
                            break;
                        m_data.ah=b_to_w_be(rxBuff,4);   // AH number ??? 
                                        
                        //m_data.cell_nr=b_to_w_be(rxBuff,6); // number of cells
                        if(!b_to_w_be_check(frameNumber,m_data.cell_nr,rxBuff,6,12,96))
                            break;                    
                        
                        //m_data.u_max=b_to_w_be(rxBuff,8) * 0.01;   // [V] max charge voltage per cell (0.01)
                        if(!b_to_w_be_check(frameNumber,m_data.u_max,rxBuff,8,0.01f,4.00f,4.15f))
                            break;   
                        //m_data.t_max=b_to_w_be(rxBuff,10) * 0.1;   // [°C] max temperature of battery (0.1)
                        if(!b_to_w_be_check(frameNumber,m_data.t_max,rxBuff,10,0.1f,30,50))
                            break; 
                        //m_data.u_bat=b_to_w_be(rxBuff,12) * 0.01;   // [V] actual voltage of the battery(0.01)
                        if(!b_to_w_be_check(frameNumber,m_data.u_bat,rxBuff,12,0.01f,36,403))
                            break; 
                       // m_data.i_bat=b_to_w_be(rxBuff,14) * 0.01;   // [A] actual current of the battery 
                        if(!b_to_w_be_check(frameNumber,m_data.i_bat,rxBuff,14,0.01f,0,30))
                            break;                         

                        if(m_data.state.bit.sign)
                            m_data.i_bat = -m_data.i_bat;
                        m_data.unknown=b_to_w_be(rxBuff,16); // ??
                        m_data.frameRxCntr[frameNumber]++;
                        m_data.frameRxTime[frameNumber]=millis();
                    break;

                    case 2:
                        indBuf=2;
                        for(int ind=0;ind<4;ind++){
                            //m_data.t[ind]=b_to_w_be(rxBuff,indBuf) * 0.1;    
                            if(!b_to_w_be_check(frameNumber,m_data.t[ind],rxBuff,indBuf,0.1f,1.0f,40.0f))
                                break;                               
                            indBuf+=4;
                        }    
                        m_data.frameRxCntr[frameNumber]++;
                        m_data.frameRxTime[frameNumber]=millis();
                    break;

                    case 3:
                    case 4:
                    case 5:
                    case 6:
                    case 7:
                        maxInd=(frameNumber-2)*9;
                        indBuf=0;
                        if(m_data.cell_nr > 0){
                            for(int ind= maxInd - 9; ind < maxInd; ind++){
                            // m_data.u_cell[ind]=b_to_w_be(rxBuff,indBuf) * 0.001f;
                                if(m_data.cell_nr <= ind)
                                    break;  // no more valid cells
                                if(!b_to_w_be_check(frameNumber,m_data.u_cell[ind],rxBuff,indBuf,0.001f,3.0f,4.3f))
                                    break;                            
                                indBuf+=2;
                            }      
                        }    
                        m_data.frameRxCntr[frameNumber]++;
                        m_data.frameRxTime[frameNumber]=millis();
                     break;

                     
                }
            }    
        }
    }
    else{
        if((millis() - m_lastCheckRunTime) > CHECK_RX_TIMEOUT_PERIOD && millis() > RX_TIMEOUT){
            m_lastCheckRunTime=millis();
            unsigned long limitTime=millis() - RX_TIMEOUT;
            for(int frameInd=1;frameInd< Data::FRAMES_NR;frameInd++){
                if(millis() > m_data.frameRxTime[frameInd] && m_data.frameRxTime[frameInd] < limitTime){
                   initData(frameInd);
                }
            }
        }
    }
    m_seekFrameSpace=true;
}

void TBMSCom::period(){

    if(m_seekFrameSpace){
        if(m_serialPtr->available() > 0){
            while(m_serialPtr->available())
                m_serialPtr->read();
            m_time1=0;
            return;
        }
        if(m_time1==0)
            m_time1=millis();
        uint32_t del_Time=millis() - m_time1;
        if(del_Time >= MIN_FRAME_SPACE){
            m_seekFrameSpace = false;
            DebugCntr=del_Time;
            m_startFrame=true;
            m_startTime=millis();
        }
    }
    else
        m_time1=0;
}

void TBMSCom::initAllData(){
    for(int ind=0;ind<Data::FRAMES_NR;ind++)
        initData(ind);
}

void TBMSCom::initData(int frameNumber){
    int maxInd;
    switch(frameNumber){
        case 1:
            m_data.u_min=NODATA_FL;
            m_data.i_max=NODATA_FL;
            m_data.ah=NODATA_UINT16;
            m_data.cell_nr=NODATA_UINT16;
            m_data.u_max=NODATA_FL;
            m_data.t_max=NODATA_FL;
            m_data.u_bat=NODATA_FL;
            m_data.i_bat=NODATA_FL;
        break;

        case 16:
            m_data.ch_dsch_state=NODATA_UINT16;
            m_data.lcd_state=NODATA_UINT16;
            break;

        case 11:
        case 12:
        case 13:
        case 14:
        case 15:
            maxInd=(frameNumber-11)*9 + 57;
            for(int ind= maxInd - 9; ind < maxInd; ind++){
               m_data.u_cell[ind]=  NODATA_FL; 
            }
            break;

        case 10:
            for(int ind=4;ind<8;ind++){
                m_data.t[ind]=NODATA_FL;
            }    
            break;   

        case 8:
            m_data.u_cell[45]=NODATA_FL;
            m_data.u_cell[46]=NODATA_FL;
            m_data.u_cell[47]=NODATA_FL;
            m_data.soc=NODATA_UINT16;
            m_data.state.all=NODATA_UINT16;
            break;    

        case 2:
            for(int ind=0;ind<4;ind++){
                m_data.t[ind]=NODATA_FL;
            }    
            break;

        case 3:
        case 4:
        case 5:
        case 6:
        case 7:
            maxInd=(frameNumber-2)*9;
            for(int ind= maxInd - 9; ind < maxInd; ind++){
                m_data.u_cell[ind]=NODATA_FL;
            }      
            break;

    }

}
 