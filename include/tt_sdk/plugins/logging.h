/**
 * @file
 *
 * This file provides the interface and data structures for the logging plugin type.
 */

#ifndef TT_SDK_PLUGINS_LOGGING_H
#define TT_SDK_PLUGINS_LOGGING_H

#include "../core/compiler.h"
#include "../core/error.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Defines the different log levels.
 */
typedef enum {
    /**
     * @brief A situation that will affect the functionality of the SDK.
     */
    TT_LL_ERROR,

    /**
     * @brief A situation that might affect the functionality, but could be recovered from.
     */
    TT_LL_WARN,

    /**
     * @brief A pure informational message that does not indicate any problems.
     */
    TT_LL_INFO,

    /**
     * @brief A debugging message.
     */
    TT_LL_DEBUG,

    /**
     * @brief The number of defined log levels.
     */
    TT_LL__COUNT,
} TtLogLevel;

/**
 * @brief Represents a logging plugin.
 */
typedef struct
{
    /**
     * @brief A context pointer passed to all functions in the structure.
     *
     * The content of the pointer is never read nor modified by the SDK itself.
     * It is for private use by the plugin itself.
     */
    void* context;

    /**
     * @brief The name of the plugin.
     */
    const char* name;

    /**
     * @brief Initializes the logging plugin.
     *
     * This can be @c NULL if no initialization of the plugin is needed.
     *
     * @param [in] ctx The context of the plugin.
     * @return An error code.
     */
    TtError (*init)(void* ctx);

    /**
     * @brief The log function called for each generated log message.
     *
     * @param [in] ctx The context of the plugin.
     * @param [in] level The log message level.
     * @param [in] msg The log message.
     */
    void (*log)(void* ctx, TtLogLevel level, const char* msg);
} TtLogging;

/**
 * @brief Logs a log message with the given log level.
 *
 * @param level The log message level.
 * @param  msg The log message.
 */
void ttLoggingLog(TtLogLevel level, const char* msg);

#ifdef __cplusplus
}
#endif

#endif
