/**************************************************************************************************************************************************
*       Main.c              Set's up the MSP430 uC Peripherals.  All calls are initiated from this routine.   While Loop cycles 65,535 times      *
*                           before putting the MCU in Sleep mode.  ISR must pull Sleep mode out of the Status Register to restart Idle_count      *
*                                                                                                                                                 *
*       Author:             Mike Claassen                                                                                                         *
*                                                                                                                                                 *
*       Revision Date:      August 2009                                                                                                           *
*                                                                                                                                                 *
*       Revision Level:     1st pass                                                                                                              *
*                                                                                                                                                 *
*       For Support:        https://e2e.ti.com/support/development_tools/mavrk/default.aspx                                                       *
*                                                                                                                                                 *
***************************************************************************************************************************************************
*       Copyright © 2009-2010 Texas Instruments Incorporated - http://www.ti.com/                                                                 *
***************************************************************************************************************************************************
*  Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met: *
*                                                                                                                                                 *
*    Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.                 *
*                                                                                                                                                 *
*    Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the        *
*    documentation and/or other materials provided with the distribution.                                                                         *
*                                                                                                                                                 *
*    Neither the name of Texas Instruments Incorporated nor the names of its contributors may be used to endorse or promote products derived      *
*    from this software without specific prior written permission.                                                                                *
*                                                                                                                                                 *
*  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT          *
*  LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT     *
*  OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT         *
*  LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY    *
*  THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE      *
*  OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.                                                                           *
***************************************************************************************************************************************************
*                                 MODULE CHANGE LOG                                                                                               *
*                                                                                                                                                 *
*       Date Changed:             {date of change}                Developer:       {developer name}                                               *
*       Change Description:       {describe change}                                                                                               *
*                                                                                                                                                 *
**************************************************************************************************************************************************/
/**************************************************************************************************************************************************
*                                 Included Headers                                                                                                *
**************************************************************************************************************************************************/
#include "stdef.h"                                // Common Expression Definitions
#include "ADS1x9x.h"                              // ADS1298 device defintions


/**************************************************************************************************************************************************
*                                 Definitions                                                                                                     *
**************************************************************************************************************************************************/
#define ADS1298_DATA_LENGTH 27

/**************************************************************************************************************************************************
*                                 Prototypes                                                                                                      *
**************************************************************************************************************************************************/
/**************************************************************************************************************************************************
*                                 Global Variables                                                                                              *
**************************************************************************************************************************************************/
//unsigned char ADS1x9x_Data [ADS1298_DATA_LENGTH];

/**************************************************************************************************************************************************
*                                 External Variables                                                                                              *
**************************************************************************************************************************************************/
extern unsigned char ADS1x9x_SPI_Settings [6];


/**************************************************************************************************************************************************
*                                 main.c   -   Initialization                                                                                     *
**************************************************************************************************************************************************/


    //enable_ADS1x9x_Conversion ();

/**************************************************************************************************************************************************
*    ISR Called Subroutines                                                                                                                       *
**************************************************************************************************************************************************/
/*
#include "stdef.h"                                // Common Expression Definitions
#include "ADS1x9x.h"                              // ADS1298 device defintions
extern volatile ADS1x9x_Status_Flags_type ADS1x9x_Status_Flags;

#ifdef __TI_COMPILER_VERSION__
#pragma CODE_SECTION(DRDY_PIN_VECTOR,".text:_isr");
#endif /* __TI_COMPILER_VERSION__ */

/*#pragma vector=DRDY_PIN_VECTOR
__interrupt void DRDY_PIN_VECTOR(void)
{
 
    if (DRDY_IV == DRDY_IFG)
    {
        DRDY_IV &= ~DRDY_IFG;                          // CLEAR ISR Flag
        ADS1x9x_Status_Flags.ADC_Data_Ready = SET;
    }
}

*/