/* ========================================================================== */
/*                                                                            */
/*   tbmscom.cpp                                                              */                                                                     
/*                                                                            */
/*   Description:                                                             */
/*               Communication protocol with China TOPBMS                     */
/* ========================================================================== */

#include "tbmscom.hpp"
#include "assert.h"


#define b_to_w_be(buf,ind) (buf[ind]>>8 & buf[ind+1])





void  TBMSCom::init(HardwareSerial * uart){
    assert(uart !=nullptr);
    m_serialPtr=uart;
     
}

void TBMSCom::main(){

    if(m_newRxDatalen > 0){
        
        switch(m_rxBufSH[0]){
            case 1:
            m_data.u_min=b_to_w_be(m_rxBufSH,0) * 0.01;   // [V] min voltage per cell (0.01) 
            m_data.i_max=b_to_w_be(m_rxBufSH,0) * 0.01;;   // [A] max charge/discharge current (0.1)
            m_data.ah=b_to_w_be(m_rxBufSH,0);   // AH number ??? 
            m_data.cell_nr=b_to_w_be(m_rxBufSH,0); // number of cells
            m_data.u_max=b_to_w_be(m_rxBufSH,0) * 0.1;   // [V] max charge voltage per cell (0.01)
            m_data.t_max=b_to_w_be(m_rxBufSH,0) * 0.1;   // [°C] max temperature of battery (0.1)
            m_data.u_bat=b_to_w_be(m_rxBufSH,0) * 0.01;   // [V] actual voltage of the battery(0.01)
            m_data.i_bat=b_to_w_be(m_rxBufSH,0);   // [A] actual current of the battery 
            m_data.unknown=b_to_w_be(m_rxBufSH,0); // ??
            break;
        }

        m_newRxDatalen=0;
    }
}    

void TBMSCom::getData(Data * ptr){

}

void TBMSCom::serialEvent(){
    if(m_serialPtr->available()){
        unsigned long newTime = millis();
        if(m_previousTime > 0 && ((newTime - m_previousTime) > MIN_FRAME_SPACE)){
            if(m_rxIndex > 0 && m_newRxDatalen ==0){
                memcpy(m_rxBufSH,m_rxBuf ,sizeof(byte) * m_rxIndex);
                m_newRxDatalen=m_rxIndex;
            }
            m_rxIndex=0;
        }
        do {
            m_rxBuf[m_rxIndex]=m_serialPtr->read();
            m_rxIndex++;
        }while(m_serialPtr->available()); 
        
        m_previousTime=newTime;
    }
}

