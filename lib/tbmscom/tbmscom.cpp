/* ========================================================================== */
/*                                                                            */
/*   tbmscom.cpp                                                              */                                                                     
/*                                                                            */
/*   Description:                                                             */
/*               Communication protocol with China TOPBMS                     */
/* ========================================================================== */

#include "tbmscom.hpp"
#include "assert.h"


#define b_to_w_be(buf,ind) (buf[ind]<<8 | buf[ind+1])

extern int DebugCntr;



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
    if(m_startFrame && m_serialPtr->available()){
        m_startFrame = false;
        DebugCntr=2;
        int frameNumber=m_serialPtr->read();
        if(frameNumber >0){           
            int readout = m_serialPtr->readBytes(rxBuff,Data::FRAMEDATALEN);
            if(readout == Data::FRAMEDATALEN){    
                switch(frameNumber){
                    case 1:
                        m_data.u_min=b_to_w_be(rxBuff,0) * 0.01;   // [V] min voltage per cell (0.01) 
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
                        m_data.t1=b_to_w_be(rxBuff,2) * 0.1;
                        m_data.t2=b_to_w_be(rxBuff,6) * 0.1;
                        m_data.t3=b_to_w_be(rxBuff,10) * 0.1;
                        m_data.t4=b_to_w_be(rxBuff,14) * 0.1;                                        
                    
                    break;

                    case 3:
                    case 4:
                    case 5:
                    case 6:
                    case 7:
                        int maxInd=(frameNumber-2)*8;
                        int indBuf=0;
                        for(int ind= maxInd - 8; ind < maxInd; ind++){
                            m_data.u_cell[ind]=b_to_w_be(rxBuff,indBuf) * 0.001f;
                            indBuf+=2;
                        }      
                     break;

                    /*case 8:
                                            
                    break;*/

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