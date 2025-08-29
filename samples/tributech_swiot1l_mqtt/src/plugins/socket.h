/**
 * @file
 *
 * This file provides the interface and data structures for the socket plugin type.
 * The socket plugin provides access to the network layer (typically TCP/IP).
 */

#ifndef TT_SDK_PLUGINS_SOCKET_H
#define TT_SDK_PLUGINS_SOCKET_H

#include "../core/common.h"
#include "../core/compiler.h"
#include "../core/error.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Represents a socket plugin.
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
     * @brief Initializes the socket plugin.
     *
     * This can be @c NULL if no initialization of the plugin is needed.
     *
     * @param [in] ctx The context of the plugin.
     * @return An error code.
     */
    TtError (*init)(void* ctx);

    /**
     * @brief The function to resolve a DNS name.
     *
     * @param [in] ctx The context of the plugin.
     * @param [in] name The DNS name to resolve.
     * @param [out] addr Filled with the address on success.
     * @return An error code.
     */
    TtError (*resolve)(void* ctx, const char* name, TtSocketAddr* addr);

    /**
     * @brief The function to create a new TCP socket and connect it.
     *
     * @param [in] ctx The context of the plugin.
     * @param [in] addr The address to connect to.
     * @param [out] handle Set to the socket handle on success.
     * @return An error code.
     */
    TtError (*create)(void* ctx, const TtSocketAddr* addr, uintptr_t* handle);

    /**
     * @brief The function to close a socket.
     *
     * @param [in] ctx The context of the plugin.
     * @param [in] handle The socket handle to close.
     * @return An error code.
     */
    TtError (*close)(void* ctx, uintptr_t handle);

    /**
     * @brief The function to send data.
     *
     * The @ref len parameter is updated by the function with the actual number of bytes sent.
     *
     * @param [in] ctx The context of the plugin.
     * @param [in] handle The socket handle.
     * @param [in] data The data to send.
     * @param [in] len The data length (updated by the function).
     * @return An error code.
     */
    TtError (*send)(void* ctx, uintptr_t handle, const void* data, size_t* len);

    /**
     * @brief The function to receive data.
     *
     * The @ref len parameter is updated by the function with the actual number of bytes received.
     *
     * @param [in] ctx The context of the plugin.
     * @param [in] handle The socket handle.
     * @param [out] data The buffer to write the received data into.
     * @param [in] len The data length (updated by the function).
     * @return An error code.
     */
    TtError (*recv)(void* ctx, uintptr_t handle, void* data, size_t* len);

    /**
     * @brief Configures an optional socket timeout for a socket created by @ref ttSocketCreate().
     *
     * @param [in] ctx The context of the plugin.
     * @param [in] handle The socket handle.
     * @param [in] timeoutMs The timeout in ms.
     * @return An error code.
     */
    TtError (*setTimeout)(void* ctx, uintptr_t handle, uint16_t timeoutMs);
} TtSocket;

/**
 * @brief Resolves a DNS name.
 *
 * @param name The DNS name to resolve.
 * @param addr Filled with the address on success.
 * @return An error code.
 */
TT_NO_DISCARD TtError ttSocketResolve(const char* name, TtSocketAddr* addr);

/**
 * @brief Creates a new TCP socket and connects it to the given address.
 *
 * @param addr The address to connect to.
 * @param handle Set to the socket handle on success.
 * @return An error code.
 */
TT_NO_DISCARD TtError ttSocketCreate(const TtSocketAddr* addr, uintptr_t* handle);

/**
 * @brief Closes a socket previously created by @ref ttSocketCreate() .
 *
 * @param handle The socket handle to close.
 * @return An error code.
 */
TT_NO_DISCARD TtError ttSocketClose(uintptr_t handle);

/**
 * @brief Sends data over a socket created by @ref ttSocketCreate() .
 *
 * The @ref len parameter is updated by the function with the actual number of bytes sent.
 *
 * @param handle The socket handle.
 * @param data The data to send.
 * @param len The data length (updated by the function).
 * @return An error code.
 */
TT_NO_DISCARD TtError ttSocketSend(uintptr_t handle, const void* data, size_t* len);

/**
 * @brief Receives data from a socket created by @ref ttSocketCreate() .
 *
 * The @ref len parameter is updated by the function with the actual number of bytes received.
 *
 * @param handle The socket handle.
 * @param data The buffer to write the received data into.
 * @param len The data length (updated by the function).
 * @return An error code.
 */
TT_NO_DISCARD TtError ttSocketRecv(uintptr_t handle, void* data, size_t* len);

/**
 * @brief Configures an optional socket timeout for a socket created by @ref ttSocketCreate().
 *
 * @param handle The socket handle.
 * @param timeoutMs The timeout in ms.
 * @return An error code.
 */
TT_NO_DISCARD TtError ttSocketTimeout(uintptr_t handle, uint16_t timeoutMs);

#ifdef __cplusplus
}
#endif

#endif
