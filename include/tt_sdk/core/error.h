/**
 * @file
 *
 * This file defines the SDK error codes and different helper macros and functions.
 * The prefix @c TT_E_ is used for all SDK error codes.
 */

#ifndef TT_SDK_CORE_ERROR_H
#define TT_SDK_CORE_ERROR_H

#include "compiler.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Defines the different error codes used within the SDK.
 *
 * Note that some error codes (in some situations) do not necessarily indicate an error.
 * Some examples:
 *   - @ref TT_E_NOT_FOUND may simply indicate that the requested object was never created.
 *   - @ref TT_E_NOT_SUPPORT may mean that the requested functionality is not support, but alternatives may be available.
 */
typedef enum {
    /**
     * @brief The operation succeeded (success, no error).
     */
    TT_E_OK = 0,

    /**
     * @brief An internal fault that cannot be mapped to any other error code.
     *
     * This error code should be used carefully since it does not provide any details about the cause!
     */
    TT_E_FAULT,

    /**
     * @brief An invalid argument was passed to a function.
     *
     * This may apply to @c NULL pointers, out-of-range arguments, and other cases.
     */
    TT_E_INVALID_ARG,

    /**
     * @brief The requested functionality is not supported.
     */
    TT_E_NOT_SUPPORTED,

    /**
     * @brief The requested memory could not be allocated.
     */
    TT_E_NO_MEMORY,

    /**
     * @brief A timeout happened.
     */
    TT_E_TIMEOUT,

    /**
     * @brief The object is in an invalid state.
     */
    TT_E_INVALID_STATE,

    /**
     * @brief The object does not exist.
     */
    TT_E_NOT_FOUND,

    /**
     * @brief The object was not initialized.
     */
    TT_E_NOT_INITIALIZED,

    /**
     * @brief A protocol error was detected.
     */
    TT_E_PROTOCOL,

    /**
     * @brief The last object was reached.
     */
    TT_E_NO_MORE,

    /**
     * @brief The value has not the right type.
     */
    TT_E_INVALID_TYPE,
} TtError;

/**
 * @brief Helper macro to check if an error code means success.
 *
 * This is the same as checking for equality to @ref TT_E_OK (which is the only value that always indicate success).
 */
#define TT_IS_OK(E) ((E) == TT_E_OK)

/**
 * @brief Helper macro to check if an error code means failure.
 *
 * This is the same as checking for inequality to @ref TT_E_OK since all other values generally indicate a failure.
 * Note that some error codes in some cases do not actually indicate an error.
 */
#define TT_IS_ERR(E) ((E) != TT_E_OK)

/**
 * @brief Helper function to convert an error code into a readable message.
 *
 * This function always returns a valid string and can therefore be used without additional checks.
 * A special string is returned for unknown error numbers.
 *
 * @param err The error code to convert.
 * @return The readable message.
 */
TT_PUBLIC TT_NO_DISCARD const char* ttErrorToString(TtError err);

#ifdef __cplusplus
}
#endif

#endif
