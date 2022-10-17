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
//#include "dtypes.h"

class TBMSCom{
    public:
    TBMSCom();
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
        static const int U_CELL_NR=48; // number of cells
        // frame 1
        float u_min;   // [V] min voltage per cell (0.01) 
        float i_max;   // [A] max charge/discharge current (0.1)
        uint16_t ah;   // AH number ??? 
        uint16_t cell_nr; // number of cells
        float u_max;   // [V] max charge voltage per cell (0.01)
        float t_max;   // [°C] max temperature of battery (0.1)
        float u_bat;   // [V] actual voltage of the battery(0.01)
        float i_bat;   // [A] actual current of the battery 
        float unknown; // ??

        // frame 2    
        float t1;      // [°C] temperature of module 1(0.1)
        float t2;      // [°C] temperature of module 2(0.1)
        float t3;      // [°C] temperature of module 3(0.1)
        float t4;      // [°C] temperature of module 4(0.1)               

        // frame  3...8, 11, 12,13, 14, 15, 16-part
        float u_cell[U_CELL_NR]; //[V] cells voltage (0.001) 
        // frame 8end
        uint8_t soc;    //[%] State of the charge (1)
        StateDef state; // warning, state ind.
 
        // frame 9
        // frame 10
        // frame 17       

    };
    int init(HardwareSerial * uart);
    void main();    
    void getData(Data * ptr);
};

#endif //tbmscom_hpp