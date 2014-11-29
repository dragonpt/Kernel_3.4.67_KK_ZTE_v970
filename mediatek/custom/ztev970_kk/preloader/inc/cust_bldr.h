#ifndef CUST_BLDR_H
#define CUST_BLDR_H

#include "cust_part.h"
#include "boot_device.h"
#include "part.h"
#include "uart.h"

/*=======================================================================*/
/* Pre-Loader Features                                                   */
/*=======================================================================*/
#ifdef MTK_EMMC_SUPPORT
#define CFG_PMT_SUPPORT         (1)//(0)
#define CFG_BOOT_DEV            (BOOTDEV_SDMMC)
#else
#define CFG_PMT_SUPPORT         (1)
#define CFG_BOOT_DEV            (BOOTDEV_NAND)
#endif
#define CFG_UART_TOOL_HANDSHAKE (1)
#define CFG_USB_TOOL_HANDSHAKE  (1)
#define CFG_USB_DOWNLOAD        (1)
#define FEATURE_DOWNLOAD_SCREEN     (1)
//#define FEATURE_DOWNLOAD_INFO   (1)

#define CFG_LOG_BAUDRATE        (921600)
#define CFG_META_BAUDRATE       (115200)
#define CFG_UART_LOG            (UART1) //(UART4)
#define CFG_UART_META           (UART1)

#define CFG_EMERGENCY_DL_SUPPORT    (1)
#define CFG_EMERGENCY_DL_TIMEOUT_MS (1000 * 60 * 5) /* 5 mins */

#define CFG_WIFI_CLOCK_SET (1)
/*=======================================================================*/
/* Misc Options                                                          */	
/*=======================================================================*/
#define FEATURE_MMC_ADDR_TRANS
/*                                                                                            */
#define CFG_LGE_RAM_DUMP_ON_PANIC       (1) 
/*                                                 */

#endif /* CUST_BLDR_H */
