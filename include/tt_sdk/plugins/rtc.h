/**
 * @file
 *
 * This file provides the interface and data structures for the RTC plugin type.
 * The RTC plugin provides the current date and time to validate certificates.
 * The date and time must @b always be in UTC, @b not local time!
 */

#ifndef TT_SDK_PLUGINS_RTC_H
#define TT_SDK_PLUGINS_RTC_H

#include "../core/compiler.h"
#include "../core/error.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Contains the current date and time as returned by an RTC.
 *
 * This structure is similar to <tt>struct tm</tt> from the C standard library, but also has some differences:
 *   - The field @c tm_wday is missing.
 *   - The field @c tm_yday is missing.
 *   - The field @c tm_isdst is missing.
 *   - The year is absolute (not relative to 1900 as with @c tm_year ).
 */
typedef struct
{
    /**
     * @brief The second (0 to 59).
     */
    uint8_t second;

    /**
     * @brief The minute (0 to 59).
     */
    uint8_t minute;

    /**
     * @brief The hour (0 to 23).
     */
    uint8_t hour;

    /**
     * @brief The day (1 to 31).
     */
    uint8_t day;

    /**
     * @brief The month (0 to 11).
     */
    uint8_t month;

    /**
     * @brief The year.
     */
    uint16_t year;
} TtRtcValue;

/**
 * @brief Represents an RTC plugin.
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
     * @brief Initializes the RTC plugin.
     *
     * This can be @c NULL if no initialization of the plugin is needed.
     *
     * @param [in] ctx The context of the plugin.
     * @return An error code.
     */
    TtError (*init)(void* ctx);

    /**
     * @brief Returns the current date and time from the RTC.
     *
     * @param [in] ctx The context of the plugin.
     * @param [out] value Set to the current date and time.
     * @return An error code.
     */
    TtError (*get)(void* ctx, TtRtcValue* value);
} TtRtc;

/**
 * @brief Retrieves the current date and time from the RTC.
 *
 * @param value Set to the current date and time on success.
 * @return An error code.
 */
TT_NO_DISCARD TtError ttRtcGet(TtRtcValue* value);

#ifdef __cplusplus
}
#endif

#endif
