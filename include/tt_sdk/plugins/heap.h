/**
 * @file
 *
 * This file provides the interface and data structures for the heap plugin type.
 * The heap plugin provides basic functionality to allocate and free memory at runtime.
 */

#ifndef TT_SDK_PLUGINS_HEAP_H
#define TT_SDK_PLUGINS_HEAP_H

#include "../core/compiler.h"
#include "../core/error.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Represents a heap plugin.
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
     * @brief Initializes the heap plugin.
     *
     * This can be @c NULL if no initialization of the plugin is needed.
     *
     * @param [in] ctx The context of the plugin.
     * @return An error code.
     */
    TtError (*init)(void* ctx);

    /**
     * @brief The function to allocate new memory.
     *
     * If no memory could be allocated the function must return @ref TT_E_NO_MEMORY .
     * The address of the allocated memory (if an allocation was possible) is returned in the @ref addr parameter.
     * All memory allocated must be naturally aligned.
     *
     * @param [in] ctx The context of the plugin.
     * @param [in] size The allocation size in bytes.
     * @param [out] addr Set to the allocated address on success (unchanged in case of an error).
     * @return An error code.
     */
    TtError (*alloc)(void* ctx, size_t size, void** addr);

    /**
     * @brief The function to free memory allocated by @ref alloc .
     *
     * @param [in] ctx The context of the plugin.
     * @param [in] addr The memory to free.
     * @return An error code.
     */
    TtError (*free)(void* ctx, const void* addr);
} TtHeap;

/**
 * @brief Tries to allocate memory.
 *
 * The allocated memory may contain undefined data (old data).
 * See @ref ttHeapAllocZero() for a function that allocates memory that is filled with zeros.
 * This function simply returns @c NULL if allocating the requested memory was not possible.
 * It is the caller's responsibility to check the returned address before using it!
 *
 * @param bytes The number of bytes to allocate.
 * @return The allocated memory or @c NULL in case of an error.
 */
TT_NO_DISCARD void* ttHeapAlloc(size_t bytes);

/**
 * @brief Tries to allocate memory and fills it with zeros.
 *
 * This function simply returns @c NULL if allocating the requested memory was not possible.
 * It is the caller's responsibility to check the returned address before using it!
 * If the allocation was successful the allocated memory will be initialized with zeros.
 *
 * @param bytes The number of bytes to allocate.
 * @return The allocated memory or @c NULL in case of an error.
 */
TT_NO_DISCARD void* ttHeapAllocZero(size_t bytes);

/**
 * @brief Frees memory allocated with @ref ttHeapAlloc() or @ref ttHeapAllocZero() .
 *
 * @param addr The memory to free.
 */
void ttHeapFree(const void* addr);

#ifdef __cplusplus
}
#endif

#endif
