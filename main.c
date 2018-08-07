
#include "board.h"
#include "pin_mux.h"
#include "fsl_debug_console.h"

void main()
{
	BOARD_BootClockHSRUN();
	BOARD_InitUARTs();
	BOARD_InitDebugConsole();
	PRINTF("***********************************************\r\n");
	PRINTF("********** MK66FN2M0xxx18 BOOTLOADER **********\r\n");
	PRINTF("***********************************************\r\n");
	while(1)
	{
		
	}
}
