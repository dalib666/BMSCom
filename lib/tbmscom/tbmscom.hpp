/* ========================================================================== */
/*                                                                            */
/*   tbmscom.hpp                                                              */
/*   V 1.0.0                                                                  */
/*   Description:                                                             */
/*               Communication protocol with China TOPBMS modul               */
/* ========================================================================== */

#ifndef   tbmscom_hpp    
#define   tbmscom_hpp

#include <Arduino.h>
#include <dtypes.h>

class TBMSCom{
    static const   int MIN_FRAME_SPACE=8u;
    static const int CHECK_RX_TIMEOUT_PERIOD=1000u;    //[ms]  period of checking RX timeout
    static const int RX_TIMEOUT = 5000;                //[ms]  RX timeout
    //byte m_rxBufSH[RXBUFER_LEN];
    HardwareSerial * m_serialPtr;
    int m_newRxDatalen;
    int m_rxIndex;
    int m_baudRate;
    unsigned long m_time1;
    int m_prev_avail;
    bool m_startFrame;
    bool m_seekFrameSpace;
    int m_frameTimoute;
    unsigned long m_startTime;
    unsigned long m_lastCheckRunTime;
    /**
     Convert from 2 bytes to float, check against the ranges,rescale by scale coef.
    @return status - true/false - refreshed by new valid value/erro value, no refresh was done
    */
    bool  b_to_w_be_check(int framenumber,float & dataItem,uint8_t *buf,int ind,float scale,float rangeL,float rangeH);
    bool  b_to_w_be_check(int framenumber,uint16_t & dataItem,uint8_t *buf,int ind,uint16_t rangeL,uint16_t rangeH);
    /**
     * init frame data
     * @frameNr - frame number
    */
    void initData(int frameNumber);

    /**
     * init all data  */
    void initAllData();

    public:
    static const   int RXBUFER_LEN=40;
      uint8_t rxBuff[RXBUFER_LEN];
    //TBMSCom();
    /**
    * Must be called from main
    */ 
    struct Data{
        typedef struct StatesBit{
            uint16_t MOS_tm:1;      // MOS temp. is under zero
            uint16_t sw_state:1;    // switche state
            uint16_t sign:1;        // sign of ??
            uint16_t discharged:1;  // indication of discharged battery
            uint16_t overdischarged:1;  // indication of very discharged battery 
            uint16_t overcurrent:1;  // indication of overcurrent
            uint16_t unknown:1;     // unknown var
            uint16_t overloaded:1;  // indication of overloading
            uint16_t unknown1:1;    // unknown var
        }StatesBit;
        typedef union StateDef{
            StatesBit bit;
            uint16_t all;
        }StateDef;
        static const int U_CELL_NR=93; // number of cells
        static const int MODUL_NR=8; // max number of battery moduls
        static const int FRAMES_NR=18;
        static const int FRAMES_MAXNR=FRAMES_NR-1;
        int frameErCntr[FRAMES_NR];    // er. counters of Rx frames
        int frameRxCntr[FRAMES_NR];    // counters of valid  Rx frames
        unsigned long frameRxTime[FRAMES_NR]; // time of last valid Rx frame
        // frame 1
        static const int FRAMEDATALEN=18;
        float u_min;   // [V] min voltage per cell (0.01) 
        float i_max;   // [A] max charge/discharge current (0.1)
        uint16_t ah;   // AH number ??? 
        uint16_t cell_nr; // number of cells
        float u_max;   // [V] max charge voltage per cell (0.01)
        float t_max;   // [°C] max temperature of battery (0.1)
        float u_bat;   // [V] actual voltage of the battery(0.01)
        float i_bat;   // [A] actual current of the battery 
        float unknown; // ??

        // frame 2,10    
        float t[MODUL_NR];      // [°C] temperature of module 1(0.1)
      
        // frame  3...8, 11, 12,13, 14, 15, 16-part
        float u_cell[U_CELL_NR]; //[V] cells voltage (0.001) 
        // frame 8end
        uint16_t soc;    //[%] State of the charge (1)
        StateDef state; // warning, state ind.
        // frame 9
        //float ur1;   //[V] recovery voltage ??? hostitelské úložiště obnovovací napětí nabíjení
        //float ur2;  //[V] ??? Napětí obnovy vybití dvoubajtového hostitelského úložiště

 
        // frame 9
        // frame 10
        // frame 17     
        // frame 16
        uint16_t    ch_dsch_state;      

    }m_data;
    
    void main();    
    void getData(Data * ptr);
    void init(HardwareSerial * uart, int baudRate);
    /* Must be called in period max 2 ms */
    void period();
};

#endif //tbmscom_hpp