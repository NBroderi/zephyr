/**
 * @file
 *
 * This file provides common settings and definitions that are needed by more than one module.
 */

#ifndef TT_SDK_CORE_COMMON_H
#define TT_SDK_CORE_COMMON_H

#include "compiler.h"

/**
 * @brief The number of bytes needed to store a hash.
 *
 * This value must be used whenever a buffer for a hash is allocated.
 */
#define TT_HASH_BYTES 32

/**
 * @brief The number of bytes needed to store a signature.
 *
 * This value must be used whenever a buffer for a signature is allocated.
 */
#define TT_SIGNATURE_BYTES 256

/**
 * @brief The length of a device ID in characters without the terminating null.
 */
#define TT_DEVICE_ID_LEN 36

/**
 * @brief The length of a stream ID in characters without the terminating null.
 */
#define TT_STREAM_ID_LEN 36

/**
 * @brief Converts milliseconds to microseconds.
 *
 * @param MS The milliseconds to convert.
 */
#define TT_MS_TO_US(MS) ((size_t)1000 * (MS))

/**
 * @brief Converts seconds to microseconds.
 *
 * @param S The seconds to convert.
 */
#define TT_SEC_TO_US(S) ((size_t)1000 * 1000 * (S))

/**
 * @brief Converts minutes to microseconds.
 *
 * @param MIN The minutes to convert.
 */
#define TT_MIN_TO_US(MIN) ((size_t)60 * 1000 * 1000 * (MIN))

/**
 * @brief Defines the maximum length of a server name, including the terminating null.
 */
#define TT_MAX_SERVER_NAME_LEN 128

/**
 * @brief Defines the MQTT broker server port.
 */
#define TT_MQTT_BROKER_PORT 1883

/**
 * @brief Represents the address of a socket.
 */
typedef struct
{
    /**
     * @brief The IPv4 address.
     *
     * The IP address is always in network byte order.
     * This means that the IP address "127.0.0.1" looks like this:
     *   - <tt>ip[0] == 127</tt>
     *   - <tt>ip[1] == 0</tt>
     *   - <tt>ip[2] == 0</tt>
     *   - <tt>ip[3] == 1</tt>
     *
     * Consequently, this means that the IP address can be casted to a 32 bit value in network byte order:
     * <tt>const uint32_t ipNetwork = *((const uint32_t*)ip);</tt>
     *
     * To convert the IP address to a 32 bit value in host byte order a function call is needed:
     * <tt>const uint32_t ipHost = ntohl(*((const uint32_t*)ip));</tt>
     */
    uint8_t ip[4];

    /**
     * @brief The port number.
     */
    uint16_t port;
} TtSocketAddr;

/**
 * @brief Defines the data needed for enrollment.
 */
typedef struct {
    /**
     * @brief The name of the enrollment server.
     *
     * The name must NOT contain a protocol specifier or a path component!
     * Valid examples are:
     *   - <tt>"example.com"</tt>
     *   - <tt>"dev-y.tributech-node.com"</tt>
     *
     * See @ref TT_MAX_SERVER_NAME_LEN for the maximum allowed length.
     */
    const char* server;

    /**
     * @brief The CA certificate of the server.
     */
    const char* caCert;

    /**
     * @brief The client certificate.
     */
    const char* clientCert;

    /**
     * @brief The private key.
     */
    const char* privKey;
} TtEnrollmentData;

/**
 * @brief Defines the possible internal states of the SDK.
 */
typedef enum {
    /**
     * @brief The initial state, not initialized.
     */
    TT_SDK_STATE_NOT_INITIALIZED,

    /**
     * @brief The SDK is in an error state.
     */
    TT_SDK_STATE_ERROR,

    /**
     * @brief The SDK is starting and initializing.
     */
    TT_SDK_STATE_STARTING,

    /**
     * @brief The SDK is requesting the first certificate.
     */
    TT_SDK_STATE_FIRST_CERT,

    /**
     * @brief The first certificate was requested, but not yet activated.
     */
    TT_SDK_STATE_CERT_NOT_ACTIVATED,

    /**
     * @brief The SDK is running normally.
     */
    TT_SDK_STATE_RUNNING,

    /**
     * @brief The SDK is requesting the next certificate.
     *
     * This is basically equal to @ref TT_SDK_STATE_RUNNING
     * but the SDK is requesting a new certificate in the background.
     */
    TT_SDK_STATE_NEXT_CERT,
} TtState;

/**
 * @brief Defines the data types the SDK can handle.
 */
typedef enum {
    /**
     * @brief A signed 32 bit integer equal to the @c int32_t type.
     */
    TT_DATA_TYPE_INT32,

    /**
     * @brief A signed 64 bit integer equal to the @c int64_t type.
     */
    TT_DATA_TYPE_INT64,

    /**
     * @brief A 32 bit floating point equal to the @c float type.
     */
    TT_DATA_TYPE_FLOAT,

    /**
     * @brief A 64 bit floating point equal to the @c double type.
     */
    TT_DATA_TYPE_DOUBLE,

    /**
     * @brief A UTF8 string.
     */
    TT_DATA_TYPE_UTF8,
} TtDataType;

/**
 * @brief Represents a stream ID.
 */
typedef struct {
    /**
     * @brief The stream ID.
     */
    char id[TT_STREAM_ID_LEN + 1];
} TtStreamId;

#endif
