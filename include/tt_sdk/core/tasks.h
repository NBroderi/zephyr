/**
 * @file
 *
 * This file provides the settings for all tasks started by the SDK.
 */

#ifndef TT_SDK_CORE_TASKS_H
#define TT_SDK_CORE_TASKS_H

/**
 * @brief The watchdog task priority.
 */
#define TT_TASK_WATCHDOG_PRIO TT_TASK_PRIO_NORMAL

/**
 * @brief The watchdog task stack size in bytes.
 */
#define TT_TASK_WATCHDOG_STACK 1024

/**
 * @brief The watchdog task name.
 */
#define TT_TASK_WATCHDOG_NAME "watchdog"

/**
 * @brief The certificate manager task priority.
 */
#define TT_TASK_CERTMANAGER_PRIO TT_TASK_PRIO_NORMAL

/**
 * @brief The certificate manager task stack size in bytes.
 */
#define TT_TASK_CERTMANAGER_STACK 1024

/**
 * @brief The certificate manager task name.
 */
#define TT_TASK_CERTMANAGER_NAME "certmanager"

/**
 * @brief The data manager task priority.
 */
#define TT_TASK_DATAMANAGER_PRIO TT_TASK_PRIO_NORMAL

/**
 * @brief The data manager task stack size in bytes.
 */
#define TT_TASK_DATAMANAGER_STACK 1024

/**
 * @brief The data manager task name.
 */
#define TT_TASK_DATAMANAGER_NAME "datamanager"

#endif
