/**
 * @file
 *
 * This file provides the interface and data structures for the FlashFS plugin type.
 * The FlashFS plugin provides a simple filesystem-like way to store data persistently.
 * Instead of file names an enumeration is used to identify each file.
 * This simplifies the implementation and overhead.
 */

#ifndef TT_SDK_PLUGINS_FLASHFS_H
#define TT_SDK_PLUGINS_FLASHFS_H

#include "../core/compiler.h"
#include "../core/error.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Defines the different files.
 *
 * The API ensures that each enumeration value will have the same value.
 * This allows an implementation to use the integer value
 * (instead of a big @c switch statement to handle all possible values).
 */
typedef enum {
    /**
     * @brief Stores the agent ID.
     */
    TT_FLASHFS_FILE_AGENT_ID = 0,

    /**
     * @brief Stores the device configuration.
     */
    TT_FLASHFS_FILE_CONFIG = 1,

    /**
     * @brief Stores the current communication certificate information.
     */
    TT_FLASHFS_FILE_CUR_COMM_CERT = 2,

    /**
     * @brief Stores the new communication certificate information (not yet used).
     */
    TT_FLASHFS_FILE_NEW_COMM_CERT = 3,

    /**
     * @brief Stores a pre-loaded RSA key.
     */
    TT_FLASHFS_FILE_PRE_LOADED_RSA_KEY = 6,

    /**
     * @brief Defines the number of files that need to be supported.
     *
     * This is not an actual file!
     */
    TT_FLASHFS_FILE__COUNT = 7,
} TtFlashfsFile;

/**
 * @brief Represents an FlashFS plugin.
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
     * @brief Initializes the FlashFS plugin.
     *
     * This can be @c NULL if no initialization of the plugin is needed.
     *
     * @param [in] ctx The context of the plugin.
     * @return An error code.
     */
    TtError (*init)(void* ctx);

    /**
     * @brief Returns the size of a file.
     *
     * @param [in] ctx The context of the plugin.
     * @param [in] type The file type to return the size for.
     * @param [out] bytes Set to the size in bytes (optional, can be @c NULL if not needed).
     * @return @ref TT_E_OK on success,
     *         @ref TT_E_NOT_FOUND if the file does not exist,
     *         or an error code.
     */
    TtError (*getSize)(void* ctx, TtFlashfsFile type, size_t* bytes);

    /**
     * @brief Reads data from a file.
     *
     * The file must exist and the offset must be inside the file.
     *
     * @param [in] ctx The context of the plugin.
     * @param [in] type The file type to read from.
     * @param [in] offset The offset to read from in bytes.
     * @param [out] buf The buffer to fill with the read data.
     * @param [in] len Must be set to the @ref buf length in bytes,
     *            will be updated with the actual number of bytes read.
     * @return An error code.
     */
    TtError (*fileRead)(void* ctx, TtFlashfsFile type, size_t offset, void* buf, size_t* len);

    /**
     * @brief Writes data to a file.
     *
     * If the file exist it is overwritten with the new content.
     * A partial write is not supported.
     *
     * @param [in] ctx The context of the plugin.
     * @param [in] type The file type to read from.
     * @param [in] buf The buffer containing the data to write.
     * @param [in] len The number of bytes to write.
     * @return An error code.
     */
    TtError (*fileWrite)(void* ctx, TtFlashfsFile type, const void* buf, size_t len);

    /**
     * @brief Deletes a file.
     *
     * @param [in] ctx The context of the plugin.
     * @param [in] type The file type to delete.
     * @return An error code.
     */
    TtError (*fileDelete)(void* ctx, TtFlashfsFile type);
} TtFlashfs;

/**
 * @brief Returns the size of a file.
 *
 * @param type The file type to return the size for.
 * @param bytes Set to the size in bytes (optional, can be @c NULL if not needed).
 * @return @ref TT_E_OK on success,
 *         @ref TT_E_NOT_FOUND if the file does not exist,
 *         or an error code.
 */
TT_NO_DISCARD TtError ttFlashfsGetSize(TtFlashfsFile type, size_t* bytes);

/**
 * @brief Reads data from a file.
 *
 * The file must exist and the offset must be inside the file.
 *
 * @param type The file type to read from.
 * @param offset The offset to read from in bytes.
 * @param buf The buffer to fill with the read data.
 * @param len Must be set to the @ref buf length in bytes,
 *            will be updated with the actual number of bytes read.
 * @return An error code.
 */
TT_NO_DISCARD TtError ttFlashfsRead(TtFlashfsFile type, size_t offset, void* buf, size_t* len);

/**
 * @brief Writes data to a file.
 *
 * If the file exist it is overwritten with the new content.
 * A partial write is not supported.
 *
 * @param type The file type to read from.
 * @param buf The buffer containing the data to write.
 * @param len The number of bytes to write.
 * @return An error code.
 */
TT_NO_DISCARD TtError ttFlashfsWrite(TtFlashfsFile type, const void* buf, size_t len);

/**
 * @brief Deletes a file.
 *
 * @param type The file type to delete.
 * @return An error code.
 */
TT_NO_DISCARD TtError ttFlashfsDelete(TtFlashfsFile type);

#ifdef __cplusplus
}
#endif

#endif
