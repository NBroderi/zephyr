/**
 * @file
 *
 * This file provides the interface and data structures for the storage plugin type.
 * The storage API is an @b optional API and should @b NOT be used directly!
 *
 * Use the macro @c CONFIG_TT_STORAGE to check if the storage API is available:
 * @code
 * #ifdef CONFIG_TT_STORAGE
 *     <storage API is available>
 * #else
 *     <storage API is not available>
 * #endif
 * @endcode
 */

#ifndef TT_SDK_PLUGINS_STORAGE_H
#define TT_SDK_PLUGINS_STORAGE_H

#include "../core/compiler.h"
#include "../core/error.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Contains information about the storage layout.
 */
typedef struct
{
    /**
     * @brief The number of blocks.
     *
     * A block is the smallest erase size.
     * A block can only be written once and must then be erased before it can be written again.
     */
    size_t blockCount;

    /**
     * @brief The size of each block in bytes.
     *
     * To get the total storage size the @ref blockCount must be multiplied with the block size.
     */
    size_t blockSize;
} TtStorageInfo;

/**
 * @brief Represents an storage plugin.
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
     * @brief Initializes the storage plugin.
     *
     * This can be @c NULL if no initialization of the plugin is needed.
     *
     * @param [in] ctx The context of the plugin.
     * @return An error code.
     */
    TtError (*init)(void* ctx);

    /**
     * @brief Returns information about the storage.
     *
     * @param [in] ctx The context of the plugin.
     * @param [out] info Filled with the storage information.
     * @return An error code.
     */
    TtError (*getInfo)(void* ctx, TtStorageInfo* info);

    /**
     * @brief Reads data from memory.
     *
     * @param [in] ctx The context of the plugin.
     * @param [in] offset The start offset in bytes.
     * @param [out] buf The buffer to read the data into.
     * @param [in] len The number of bytes to read (must @b not be bigger than the sizeof of @ref buf ).
     * @return An error code.
     */
    TtError (*read)(void* ctx, size_t offset, void* buf, size_t len);

    /**
     * @brief Writes data into memory.
     *
     * If the memory was not erased before, the content after the write will be undefined!
     *
     * @param [in] ctx The context of the plugin.
     * @param [in] offset The start offset in bytes.
     * @param [in] buf The data to write.
     * @param [in] len The number of bytes to write.
     * @return An error code.
     */
    TtError (*write)(void* ctx, size_t offset, const void* buf, size_t len);

    /**
     * @brief Erases the given memory range.
     *
     * The erase offset and length must both align to the erase block size!
     *
     * @param [in] ctx The context of the plugin.
     * @param [in] offset The start offset in bytes.
     * @param [in] len The number of bytes to erase.
     * @return An error code.
     */
    TtError (*erase)(void* ctx, size_t offset, size_t len);
} TtStorage;

/**
 * @brief Returns information about the storage.
 *
 * @param info Filled with the storage information.
 * @return An error code.
 */
TT_NO_DISCARD TtError ttStorageGetInfo(TtStorageInfo* info);

/**
 * @brief Reads data from memory.
 *
 * @param offset The start offset in bytes.
 * @param buf The buffer to read the data into.
 * @param len The number of bytes to read (must @b not be bigger than the sizeof of @ref buf ).
 * @return An error code.
 */
TT_NO_DISCARD TtError ttStorageRead(size_t offset, void* buf, size_t len);

/**
 * @brief Writes data into memory.
 *
 * If the memory was not erased before, the content after the write will be undefined!
 *
 * @param offset The start offset in bytes.
 * @param buf The data to write.
 * @param len The number of bytes to write.
 * @return An error code.
 */
TT_NO_DISCARD TtError ttStorageWrite(size_t offset, const void* buf, size_t len);

/**
 * @brief Erases the given memory range.
 *
 * The erase offset and length must both align to the erase block size!
 *
 * @param offset The start offset in bytes.
 * @param len The number of bytes to erase.
 * @return An error code.
 */
TT_NO_DISCARD TtError ttStorageErase(size_t offset, size_t len);

#ifdef __cplusplus
}
#endif

#endif
