#include "MK66F18.h"
#include "board.h"
#include "pin_mux.h"
#include "fsl_debug_console.h"
#include "flash/internal_flash.h"
#include "partition.h"
#define __VERSION               "1.0"
#define __AUTHOR                "ChauNM"
#define __APPLICATION_ENTRY     0x00010000
#define FIRMWARE_KEY_1			0x55AAAA55
#define FIRMWARE_KEY_2 			0x2112FEEF
/*!
 * @brief switch to application
*/
void SwitchApplication(uint32_t programAddr)
{
    // disable interrupt
    __disable_interrupt();
    //configure vector table
    SCB->VTOR = programAddr;
    //set stack pointer to application stack pointer
    __asm volatile("mov r0, %0\n\t" ::"r"(programAddr));
    __asm("ldr r1, [r0]");
    __asm("mov sp, r1");
    //get address of reset vector of application (@ program addr + 4)
    __asm("ldr r0, [r0, #4]");
    //jump to application
    __asm("bx r0");
}
/*
 * @brief main function 
*/
extern flash_config_t s_flashDriver;
uint8_t copyBuffer[COPY_BUFFER_SIZE];

void main()
{
    status_t result;
    uint32_t copySize = 0;
	uint32_t firmwareKey[2];
	BOARD_BootClockHSRUN();
	BOARD_InitUARTs();
	BOARD_InitDebugConsole();
    PRINTF("\r\n\r\n\r\n");
	PRINTF("***********************************************\r\n");
	PRINTF("********** MK66FN2M0xxx18 BOOTLOADER **********\r\n");
	PRINTF("***********************************************\r\n");
    PRINTF("Author: %s; ", __AUTHOR);
    PRINTF("Version %s; build %s %s\r\n", __VERSION, __DATE__, __TIME__);    
    /* Init internal flash */
    IFLASH_Init();
	// Read firmware key
	IFLASH_CopyFlashToRam(INFORMATION_START_ADDR, (uint8_t*)firmwareKey, sizeof(firmwareKey));
	// check if new image exists
	if ((firmwareKey[0] != FIRMWARE_KEY_1) || (firmwareKey[1] != FIRMWARE_KEY_2))
	{
		goto APPLICATION;
	}
    //check blank in image partition
    if (IFLASH_CheckBlank(IMAGE_START_ADDR, IMAGE_SIZE) != kStatus_FLASH_Success)
    {
        PRINTF("Found new image\r\n");
        // delete appliaction partition
        result = IFLASH_Erase(APPLICATION_START_ADDR, APPLICATION_SIZE);
		
        if (result != kStatus_FLASH_Success)
        {
            PRINTF("Erase application partition failed\r\n");
            while(1);
        }
        // copy from image partition to appliaction partition
        copySize = 0;
        while (copySize < APPLICATION_SIZE)
        {
            IFLASH_CopyFlashToRam(IMAGE_START_ADDR + copySize, copyBuffer, COPY_BUFFER_SIZE);
            result = IFLASH_CopyRamToFlash(APPLICATION_START_ADDR + copySize, (uint32_t*)copyBuffer, COPY_BUFFER_SIZE);
            copySize += COPY_BUFFER_SIZE;
            if (result != kStatus_FLASH_Success)
            {
                PRINTF("Update image failed\r\n");
                while(1);
            }
        }
        // erase image partition
        result = IFLASH_Erase(IMAGE_START_ADDR, IMAGE_SIZE);
    }
APPLICATION:
	if (IFLASH_CheckBlank(APPLICATION_START_ADDR, APPLICATION_SIZE) != kStatus_FLASH_Success)
	{
    	PRINTF("Run to appliaction @ 0x%x\r\n", APPLICATION_START_ADDR);
    	SwitchApplication(APPLICATION_START_ADDR);
	}
	else
		PRINTF("No appliaction firmware found\r\n"); 
	while(1)
	{
		//Application code for serial firmware update
	}
}
