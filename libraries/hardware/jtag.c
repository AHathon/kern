#include "libraries/hardware/jtag.h"

void JTAG_Init() 
{
    GPIO_SetAlt(22, FUNC_A4);  //TRST
    GPIO_SetAlt(23, FUNC_A4);  //RTCK
    GPIO_SetAlt(24, FUNC_A4);  //TDO
    GPIO_SetAlt(25, FUNC_A4);  //TCK
    GPIO_SetAlt(26, FUNC_A4);  //TDI
    GPIO_SetAlt(27, FUNC_A4);  //TMS
}