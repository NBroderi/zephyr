#ifndef TT_FREERTOS_FLASHFS_H
#define TT_FREERTOS_FLASHFS_H

#include "../../../include/tt_sdk/core/compiler.h"
#include "../../../include/tt_sdk/core/error.h"
#include <stdio.h>


#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Defines the maximum length of the mount point.
 *
 * This length includes the terminating zero.
 */
#define TT_ZEPHYR_FLASHFS_MOUNT_POINT_MAX_LEN 20

/**
 * @brief Initializes the FlashFS plugin for FreeRTOS.
 *
 * @param mountPoint The mount point of the underlying file system.
 * @return An error code.
 */
TT_NO_DISCARD TtError ttFreeRTOSFlashfsInit(void *ctx);


#ifdef __cplusplus
}
#endif

#endif
