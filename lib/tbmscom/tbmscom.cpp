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
    if((value > rangeL)&&(value < rangeH)){
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
    if((value > rangeL)&&(value < rangeH)){
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
    m_serialPtr->setTimeout(5+((uint32_t) Data::FRAMEDATALEN * m_baudRate)/10000); 
}


void TBMSCom::getData(Data * ptr){

}


void TBMSCom::main(){   
    int indBuf=0; 
    int maxInd=0;
    if(m_startFrame && m_serialPtr->available()){
        m_startFrame = false;
        DebugCntr=2;
        int frameNumber=m_serialPtr->read();
        if(frameNumber > 0){           
            int readout = m_serialPtr->readBytes(rxBuff,Data::FRAMEDATALEN);
            if(readout == Data::FRAMEDATALEN){    
                switch(frameNumber){

                    case 16:
                        m_data.ch_dsch_state=b_to_w_be(rxBuff,14);
                    break;    

                    case 11:
                    case 12:
                    case 13:
                    case 14:
                    case 15:
                        maxInd=(frameNumber-11)*9 + 49;
                        indBuf=0;
                        for(int ind= maxInd - 9; ind < maxInd+9; ind++){
                            if(!b_to_w_be_check(frameNumber,m_data.u_cell[ind],rxBuff,indBuf,0.001f,3.0f,4.3f)){
                                break;
                            }
                            indBuf+=2;
                        }      
                     break;

                    case 10:
                        indBuf=2;
                        for(int ind=4;ind<8;ind++){
                            //m_data.t[ind]=b_to_w_be(rxBuff,indBuf) * 0.1;
                            if(!b_to_w_be_check(frameNumber,m_data.t[ind],rxBuff,indBuf,0.1f,1.0f,40.0f))
                                break;
                            indBuf+=4;
                        }    
                     break;

                    case 8:
                        m_data.u_cell[45]=b_to_w_be(rxBuff,0) * 0.001f;      
                        m_data.u_cell[46]=b_to_w_be(rxBuff,2) * 0.001f;                      
                        m_data.u_cell[47]=b_to_w_be(rxBuff,4) * 0.001f;   
                        //m_data.soc=b_to_w_be(rxBuff,8);
                        if(!b_to_w_be_check(frameNumber,m_data.soc,rxBuff,8,0,100))
                            break;
                        //m_data.state.all=b_to_w_be(rxBuff,14);
                        if(!b_to_w_be_check(frameNumber,m_data.state.all,rxBuff,14,0,512))
                            break;
                          
                    break;

                    case 1:
                        //m_data.u_min=b_to_w_be(rxBuff,0) * 0.01;   // [V] min voltage per cell (0.01) 
                        if(!b_to_w_be_check(frameNumber,m_data.u_min,rxBuff,0,0.01f,3.0f,4.2f))
                            break;
                        m_data.i_max=b_to_w_be(rxBuff,2) * 0.1;   // [A] max charge/discharge current (0.1)
                        m_data.ah=b_to_w_be(rxBuff,4);   // AH number ??? 
                        m_data.cell_nr=b_to_w_be(rxBuff,6); // number of cells
                        m_data.u_max=b_to_w_be(rxBuff,8) * 0.01;   // [V] max charge voltage per cell (0.01)
                        m_data.t_max=b_to_w_be(rxBuff,10) * 0.1;   // [°C] max temperature of battery (0.1)
                        m_data.u_bat=b_to_w_be(rxBuff,12) * 0.01;   // [V] actual voltage of the battery(0.01)
                        m_data.i_bat=b_to_w_be(rxBuff,14) * 0.01;   // [A] actual current of the battery 
                        m_data.unknown=b_to_w_be(rxBuff,16); // ??
                        
                    break;

                    case 2:
                        indBuf=2;
                        for(int ind=0;ind<4;ind++){
                            m_data.t[ind]=b_to_w_be(rxBuff,indBuf) * 0.1;                                   
                            indBuf+=4;
                        }    
                    break;

                    case 3:
                    case 4:
                    case 5:
                    case 6:
                    case 7:
                        maxInd=(frameNumber-2)*9;
                        indBuf=0;
                        for(int ind= maxInd - 9; ind < maxInd; ind++){
                            m_data.u_cell[ind]=b_to_w_be(rxBuff,indBuf) * 0.001f;
                            indBuf+=2;
                        }      
                     break;

                     
                }
            }    
        }
    }
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
            DebugCntr=del_Time;
            m_startFrame=true;
        }
    }
    else
        m_time1=0;
}