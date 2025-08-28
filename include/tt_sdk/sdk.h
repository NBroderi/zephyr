/**
 * @file
 *
 * Provides the SDK API used by the application.
 */

#ifndef TT_SDK_TT_SDK_H
#define TT_SDK_TT_SDK_H

// provide common definitions and types
#include "core/common.h"
#include "core/compiler.h"
#include "core/error.h"
#include "version.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Defines the configuration used to initialize the SDK.
 */
typedef struct {
    /**
     * @brief The enrollment data to use.
     */
    const TtEnrollmentData* enrollment;

    /**
     * @brief An optional callback for SDK heartbeats.
     *
     * This function is called on a regular basis by the SDK.
     * It can be used by the application to check if the SDK is still working as expected.
     * If the application is not interested in this information it can set this to @c NULL .
     */
    void (*heartbeatCallback)();

    /**
     * @brief An optional callback for SDK status changes.
     *
     * This function is called by the SDK every time the internal state changes.
     * If the application is not interested in this information it can set this to @c NULL .
     */
    void (*stateCallback)(TtState oldState, TtState newState);

    /**
     * @brief The name of the MQTT broker.
     *
     * The name must NOT contain a protocol specifier or a path component!
     * Valid examples are:
     *   - <tt>"example.com"</tt>
     *   - <tt>"demeter-mqtt.dev-y.tributech-node.com"</tt>
     *
     * See @ref TT_MAX_SERVER_NAME_LEN for the maximum allowed length.
     */
    const char* mqttBroker;

    /**
     * @brief An optional JSON string containing a public/private RSA key pair (can be @c NULL if not needed).
     *
     * The JSON must contain the following two fields:
     *   - "public" (the public RSA key as PEM string)
     *   - "private" (the private RSA key as PEM string)
     */
    const char* preLoadedRsaKey;
} TtConfig;

/**
 * @brief Initializes the SDK.
 *
 * This function must be called before any other function of the API!
 * It initializes the whole SDK.
 *
 * @param [in] config The SDK configuration to use.
 * @return @ref TT_E_OK on success, otherwise an error code.
 */
TT_PUBLIC TT_NO_DISCARD TtError ttInit(const TtConfig* config);

/**
 * @brief Returns the current state of the SDK.
 *
 * @param [out] state Set to the current SDK state.
 * @return @ref TT_E_OK on success, otherwise an error code.
 */
TT_PUBLIC TT_NO_DISCARD TtError ttGetState(TtState* state);

/**
 * @brief Converts a SDK state enumeration value into a readable message.
 *
 * This function always returns a valid string and can therefore be used without additional checks.
 * A special string is returned for unknown states.
 *
 * @param [in] state The state to convert.
 * @return The readable message.
 */
TT_PUBLIC TT_NO_DISCARD const char* ttStateToString(TtState state);

/**
 * @brief Returns the time since the last SDK heartbeat.
 *
 * An example usage could look something like this:
 * ```{.c}
 * size_t lastHeartbeat;
 * if (TT_IS_OK(ttGetLastHeartbeat(&lastHeartbeat))) {
 *     // check if the SDK is still alive
 *     if (lastHeartbeat > 5000) {
 *         // no SDK heartbeat in the last 5 seconds
 *         LOG_ERR("The Tributech SDK seems to be stuck");
 *         reset();
 *     }
 * }
 * ```
 *
 * @param [out] lastHeartbeat Set to the milliseconds since the last SDK heartbeat.
 * @return @ref TT_E_OK on success, otherwise an error code.
 */
TT_PUBLIC TT_NO_DISCARD TtError ttGetLastHeartbeat(size_t* lastHeartbeat);

/**
 * @brief Provides the device ID.
 *
 * An example usage could look something like this:
 * ```{.c}
 * char deviceId[TT_DEVICE_ID_LEN + 1];
 * if (TT_IS_OK(ttGetDeviceId(deviceId, sizeof(deviceId)))) {
 *     // you can use the device ID, e.g. for logging
 *     printf("Device ID: %s", deviceId);
 * }
 * ```
 *
 * @param [out] buf Filled with the device ID.
 * @param [in] size The size of the @ref buf parameter in bytes.
 * @return @ref TT_E_OK on success, otherwise an error code.
 */
TT_PUBLIC TT_NO_DISCARD TtError ttGetDeviceId(char* buf, size_t size);

/**
 * @brief Provides the current twin configuration.
 *
 * An example usage could look something like this:
 * ```{.c}
 * size_t bufSize = 0;
 * if (TT_IS_OK(ttGetTwinConfig(NULL, &bufSize))) {
 *     // we now know how much memory needs to be allocated
 *     char* buf = malloc(bufSize);
 *     if (TT_IS_OK(ttGetTwinConfig(buf, &bufSize))) {
 *         // you can use the twin configuration, e.g. for logging/debugging
 *         printf("Twin config: %s", buf);
 *     }
 *     free(buf);
 * }
 * ```
 *
 * @param [out] buf The buffer to fill with the twin config
 *                  (can be @c NULL to query the needed size, see the example).
 * @param [in,out] size The buffer size (input), updated with the actual needed buffer size (output).
 * @return @ref TT_E_OK on success, otherwise an error code.
 */
TT_PUBLIC TT_NO_DISCARD TtError ttGetTwinConfig(char* buf, size_t* size);

/**
 * @brief Returns the first or next configured stream.
 *
 * This function basically returns the next stream after the one passed as argument.
 * The parameter @ref cur must be @c NULL to get the first stream.
 *
 * @param [in] cur The current stream ID or @c NULL to get the first stream.
 * @param [out] next Set to the next stream ID on success.
 * @return @ref TT_E_OK on success,
 *         @ref TT_E_NO_MORE if there is no next stream,
 *         @ref TT_E_NOT_FOUND if @ref cur is an unknown stream ID,
 *         or another error code.
 */
TT_PUBLIC TT_NO_DISCARD TtError ttGetNextStream(const TtStreamId* cur, TtStreamId* next);

/**
 * @brief Finds a stream by name and provides it ID.
 *
 * @param [in] name The name to search for.
 * @param [out] id Set to the stream ID on success.
 * @return @ref TT_E_OK on success,
 *         @ref TT_E_NOT_FOUND if @ref name is unknown,
 *         or another error code.
 */
TT_PUBLIC TT_NO_DISCARD TtError ttGetStreamByName(const char* name, TtStreamId* id);

/**
 * @brief Returns details about a stream.
 *
 * @param [in] id The stream ID.
 * @param [out] type Set to the stream's data type (can be @c NULL if not needed).
 * @param [out] merkleDepth The merkle tree depth (can be @c NULL if not needed).
 * @param [out] nameLen Set to the stream's name length without the terminating null
 *                (can be @c NULL if not needed).
 * @return @ref TT_E_OK on success,
 *         @ref TT_E_NOT_FOUND if @ref id is unknown,
 *         or another error code.
 */
TT_PUBLIC TT_NO_DISCARD TtError ttGetStreamDetails(const TtStreamId* id, TtDataType* type, size_t* merkleDepth, size_t* nameLen);

/**
 * @brief Provides the name of a stream.
 *
 * The caller is responsible to pass a big enough buffer.
 * The required size can be queried using @ref ttGetStreamDetails() using the @c nameLen parameter.
 * Don't forget to add at least one more character to @c nameLen for the terminating null!
 *
 * @param [in] id The stream ID.
 * @param [out] buf The buffer filled with the name on success.
 * @param [in] bufLen The size of @ref buf in characters.
 * @return @ref TT_E_OK on success,
 *         @ref TT_E_NOT_FOUND if @ref id is unknown,
 *         @ref TT_E_NO_MEMORY if @ref buf is too small,
 *         or another error code.
 */
TT_PUBLIC TT_NO_DISCARD TtError ttGetStreamName(const TtStreamId* id, char* buf, size_t bufLen);

/**
 * @brief Adds a signed 32 bit integer to a stream.
 *
 * @param [in] streamId The ID of the stream to add the value to.
 * @param [in] value The value to add.
 * @return @ref TT_E_OK on success,
 *         @ref TT_E_NOT_FOUND if @ref streamId is unknown,
 *         @ref TT_E_INVALID_TYPE if the stream has a different data type,
 *         or another error code.
 */
TT_PUBLIC TT_NO_DISCARD TtError ttAddInt32(const TtStreamId* streamId, int32_t value);

/**
 * @brief Adds a signed 64 bit integer to a stream.
 *
 * @param [in] streamId The ID of the stream to add the value to.
 * @param [in] value The value to add.
 * @return @ref TT_E_OK on success,
 *         @ref TT_E_NOT_FOUND if @ref streamId is unknown,
 *         @ref TT_E_INVALID_TYPE if the stream has a different data type,
 *         or another error code.
 */
TT_PUBLIC TT_NO_DISCARD TtError ttAddInt64(const TtStreamId* streamId, int64_t value);

/**
 * @brief Adds a single precision flotaing point to a stream.
 *
 * @param [in] streamId The ID of the stream to add the value to.
 * @param [in] value The value to add.
 * @return @ref TT_E_OK on success,
 *         @ref TT_E_NOT_FOUND if @ref streamId is unknown,
 *         @ref TT_E_INVALID_TYPE if the stream has a different data type,
 *         or another error code.
 */
TT_PUBLIC TT_NO_DISCARD TtError ttAddFloat(const TtStreamId* streamId, float value);

/**
 * @brief Adds a double precision flotaing point to a stream.
 *
 * @param [in] streamId The ID of the stream to add the value to.
 * @param [in] value The value to add.
 * @return @ref TT_E_OK on success,
 *         @ref TT_E_NOT_FOUND if @ref streamId is unknown,
 *         @ref TT_E_INVALID_TYPE if the stream has a different data type,
 *         or another error code.
 */
TT_PUBLIC TT_NO_DISCARD TtError ttAddDouble(const TtStreamId* streamId, double value);

/**
 * @brief Adds a UTF8 null-terminated string to a stream.
 *
 * @param [in] streamId The ID of the stream to add the value to.
 * @param [in] value The value to add.
 * @return @ref TT_E_OK on success,
 *         @ref TT_E_NOT_FOUND if @ref streamId is unknown,
 *         @ref TT_E_INVALID_TYPE if the stream has a different data type,
 *         or another error code.
 */
TT_PUBLIC TT_NO_DISCARD TtError ttAddUtf8(const TtStreamId* streamId, const char* value);

#ifdef __cplusplus
}
#endif

#endif
