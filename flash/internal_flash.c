#include "internal_flash.h"

/*! @brief Flash driver Structure */
flash_config_t s_flashDriver;
flash_security_state_t securityStatus = kFLASH_SecurityStateNotSecure; /* Return protection status */
uint32_t pflashBlockBase = 0;
uint32_t pflashTotalSize = 0;
uint32_t pflashSectorSize = 0;
uint32_t pflashBlockSize = 0;

static void error_trap(void)
{
    PRINTF("\r\n\r\n\r\n\t---- HALTED DUE TO FLASH ERROR! ----\r\n\r\n\r\n");
    while (1)
    {
    }
}

void IFLASH_Init()
{
    status_t result;
    /* Clean up Flash driver Structure*/
    memset(&s_flashDriver, 0, sizeof(flash_config_t));

    /* Setup flash driver structure for device and initialize variables. */
    result = FLASH_Init(&s_flashDriver);
    if (kStatus_FLASH_Success != result)
    {
        error_trap();
    }
    /* Get flash properties*/
    FLASH_GetProperty(&s_flashDriver, kFLASH_PropertyPflashBlockBaseAddr, &pflashBlockBase);
    FLASH_GetProperty(&s_flashDriver, kFLASH_PropertyPflashTotalSize, &pflashTotalSize);
    FLASH_GetProperty(&s_flashDriver, kFLASH_PropertyPflashSectorSize, &pflashSectorSize);
    FLASH_GetProperty(&s_flashDriver, kFLASH_PropertyPflashBlockSize, &pflashBlockSize);
    /* Print flash information - PFlash. */
    PRINTF("\r\n Internal flash Information: ");
    PRINTF("\r\n Total Program Flash Size:\t%d KB, Hex: (0x%x)", (pflashTotalSize / 1024), pflashTotalSize);
    PRINTF("\r\n Program Flash Sector Size:\t%d KB, Hex: (0x%x) ", (pflashSectorSize / 1024), pflashSectorSize);
    PRINTF("\r\n Program Flash Block Size:\t%d B, Hex: (0x%x) ", (pflashBlockBase), pflashBlockBase);
    /* Check security status. */
    result = FLASH_GetSecurityState(&s_flashDriver, &securityStatus);
    if (kStatus_FLASH_Success != result)
    {
        error_trap();
    }
    /* Print security status. */
    switch (securityStatus)
    {
        case kFLASH_SecurityStateNotSecure:
            PRINTF("\r\n Flash is UNSECURE!");
            break;
        case kFLASH_SecurityStateBackdoorEnabled:
            PRINTF("\r\n Flash is SECURE, BACKDOOR is ENABLED!");
            break;
        case kFLASH_SecurityStateBackdoorDisabled:
            PRINTF("\r\n Flash is SECURE, BACKDOOR is DISABLED!");
            break;
        default:
            break;
    }
    PRINTF("\r\n");
}

status_t IFLASH_EraseSector(uint32_t startSector, uint32_t endSector)
{
    status_t result;
    uint32_t destAdrss, eraseSize;
     /* Check security status. */
    result = FLASH_GetSecurityState(&s_flashDriver, &securityStatus);
    if (kStatus_FLASH_Success != result)
    {
        error_trap();
    }
    if (securityStatus != kFLASH_SecurityStateNotSecure)        
    {
        PRINTF("Flash is secured, can't not excute delete\r\n");
        return kStatus_FLASH_ProtectionViolation;
    }
    /* Check input parameter */
    if (endSector < startSector)
    {
        PRINTF("Input argument invalid\r\n");
        return kStatus_FLASH_InvalidArgument;
    }
    eraseSize = (endSector - startSector + 1) * pflashSectorSize;
    /* Erase a number of sector*/
    while (startSector < endSector)
    {
        destAdrss = pflashBlockBase + (startSector * pflashSectorSize);
        result = FLASH_Erase(&s_flashDriver, destAdrss, pflashSectorSize, kFLASH_ApiEraseKey);
        if (kStatus_FLASH_Success != result)
        {
            error_trap();
        }
        /* Verify sector if it's been erased. */
        result = FLASH_VerifyErase(&s_flashDriver, destAdrss, pflashSectorSize, kFLASH_MarginValueUser);
        if (kStatus_FLASH_Success != result)
        {
            error_trap();
        }
        startSector++;
    }
    
    /* Print message for user. */
    PRINTF("\r\n Successfully Erased Sector 0x%x -> 0x%x\r\n", destAdrss, destAdrss + eraseSize);
    return result;
}

status_t IFLASH_Erase(uint32_t startAddr, uint32_t length)
{
    status_t result;
    result = FLASH_Erase(&s_flashDriver, startAddr, length, kFLASH_ApiEraseKey);
    return result;
}

status_t IFLASH_CheckBlank(uint32_t startAddr, uint32_t length)
{
    status_t result;
    result = FLASH_VerifyErase(&s_flashDriver, startAddr, length, kFLASH_MarginValueUser);
    PRINTF("Verify blank memory address 0x%x, size 0x%x, result: %d\r\n", startAddr, length, result);
    return result;
}

status_t IFLASH_CopyRamToFlash(uint32_t dstAddr, uint32_t *srcAddr, uint32_t numOfBytes)
{
    status_t result;
    result = FLASH_Program(&s_flashDriver, dstAddr, srcAddr, numOfBytes);
    PRINTF("Write to flash memory address 0x%x, size 0x%x, result: %d\r\n", dstAddr, numOfBytes, result);
    return result;
}

void IFLASH_CopyFlashToRam(uint32_t srcAddr, uint8_t* dstAddr, uint32_t numOfBytes)
{
    while (numOfBytes > 0)
    {
        *dstAddr = *((volatile uint8_t*)srcAddr);
        dstAddr++;
        srcAddr++;
        numOfBytes--;
    }
}

bool IFLASH_IsUnsecure()
{
    status_t result;
    result = FLASH_GetSecurityState(&s_flashDriver, &securityStatus);
    if (kStatus_FLASH_Success != result)
    {
        error_trap();
    }
    /* Print security status. */
    if (securityStatus != kFLASH_SecurityStateNotSecure)
        return false;
    else
        return true;
}
