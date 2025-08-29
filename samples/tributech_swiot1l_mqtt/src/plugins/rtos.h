/**
 * @file
 *
 * This file provides the interface and data structures for the RTOS plugin type.
 * The RTOS plugin provides basic RTOS functionalities such as task support and mutexes.
 */

#ifndef TT_SDK_PLUGINS_RTOS_H
#define TT_SDK_PLUGINS_RTOS_H

#include "../core/compiler.h"
#include "../core/error.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Defines a task that is in an invalid state.
 *
 * @param N The name of the @ref TtTask instance.
 */
#define TT_TASK_DEFINE(N) TtTask N = { .handle = 0, .taskContext = 0 }

/**
 * @brief Defines a mutex that is in an invalid state.
 *
 * @param N The name of the @ref TtMutex instance.
 */
#define TT_MUTEX_DEFINE(N) TtMutex N = { .handle = 0 }

/**
 * @brief Defines the different task priorities.
 */
typedef enum {
    /**
     * @brief The default task priority.
     */
    TT_TASK_PRIO_NORMAL,
} TtTaskPriority;

/**
 * @brief Represents a task.
 */
typedef struct {
    /**
     * @brief The RTOS task handle.
     */
    uintptr_t handle;

    /**
     * @brief The RTOS-specific task context.
     */
    uintptr_t taskContext;
} TtTask;

/**
 * @brief Represents a mutex.
 */
typedef struct {
    /**
     * @brief The RTOS mutex handle.
     */
    uintptr_t handle;
} TtMutex;

/**
 * @brief Represents a RTOS plugin.
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
     * @brief Initializes the RTOS plugin.
     *
     * This can be @c NULL if no initialization of the plugin is needed.
     *
     * @param [in] ctx The context of the plugin.
     * @return An error code.
     */
    TtError (*init)(void* ctx);

    /**
     * @brief Creates a new task.
     *
     * Each task has two assigned values:
     *   - handle: A unique handle assigned by the RTOS, used to identify the tasks
     *   - taskCtx: A task context for the task (optional, only used by the RTOS plugin)
     *
     * @param [in] ctx The context of the plugin.
     * @param [in] func The main function of the task.
     * @param [in] data The data passed to the task function.
     * @param [in] stack The stack size in bytes (may be increased by the plugin).
     * @param [in] priority The priority of the new task.
     * @param [in] name An optional name for the task for debugging (may be ignored by the plugin).
     * @param [out] handle Set to the opaque task handle on success.
     * @param [out] taskCtx Set to the opaque task context on success.
     * @return An error code.
     */
    TtError (*taskCreate)(void* ctx, void (*func)(void*), void* data, size_t stack, TtTaskPriority priority, const char* name, uintptr_t* handle, uintptr_t* taskCtx);

    /**
     * @brief Delays the current task for at least the given amount of microseconds.
     *
     * The function may sleep longer than the given time, but never shorter.
     *
     * @param [in] ctx The context of the plugin.
     * @param [in] usec The number of microseconds to sleep.
     */
    void (*sleepUsec)(void* ctx, size_t usec);

    /**
     * @brief Returns the current tick counter.
     *
     * The tick counter must be in milliseconds.
     * If the RTOS has a lower resolution it must convert the internal tick counter into milliseconds.
     *
     * @param [in] ctx The context of the plugin.
     * @return The number of ticks/milliseconds since startup.
     */
    size_t (*getTicks)(void* ctx);

    /**
     * @brief Creates a new mutex.
     *
     * @param [in] ctx The context of the plugin.
     * @param [out] mutex Set to the opaque mutex handle on success.
     * @return An error code.
     */
    TtError (*mutexCreate)(void* ctx, uintptr_t* mutex);

    /**
     * @brief Locks a mutex.
     *
     * @param [in] ctx The context of the plugin.
     * @param [in] mutex The mutex to lock.
     * @param [in] timeout The wait timeout in milliseconds or @c SIZE_MAX to wait forever.
     * @return @ref TT_E_OK when locked,
     *         @ref TT_E_TIMEOUT when a timeout happened,
     *         or an error code.
     */
    TtError (*mutexLock)(void* ctx, uintptr_t mutex, size_t timeout);

    /**
     * @brief Unlocks a previously locked mutex.
     *
     * @param [in] ctx The context of the plugin.
     * @param [in] mutex The locked mutex to unlock.
     * @return An error code.
     */
    TtError (*mutexUnlock)(void* ctx, uintptr_t mutex);

    /**
     * @brief Destroys a mutex.
     *
     * This function frees all resources allocated by a call to @ref mutexCreate .
     *
     * @param [in] ctx The context of the plugin.
     * @param [in] mutex The mutex to destroy.
     * @return An error code.
     */
    TtError (*mutexDestroy)(void* ctx, uintptr_t mutex);
} TtRtos;

/**
 * @brief Creates a new task.
 *
 * @param func The main function of the task.
 * @param data The data passed to the task function.
 * @param stack The stack size in bytes (may be increased by the plugin).
 * @param priority The priority of the new task.
 * @param name An optional name for the task for debugging (may be ignored by the plugin).
 * @param task Filled with the task information on success.
 * @return An error code.
 */
TT_NO_DISCARD TtError ttRtosTaskCreate(void (*func)(void*), void* data, size_t stack, TtTaskPriority priority, const char* name, TtTask* task);

/**
 * @brief Delays the current task for at least the given amount of microseconds.
 *
 * The function may sleep longer than the given time, but never shorter.
 *
 * @param usec The number of microseconds to sleep.
 */
void ttRtosSleepUsec(size_t usec);

/**
 * @brief Returns the current tick counter in milliseconds.
 *
 * @param ticks Set to the number of ticks/milliseconds since startup.
 * @return An error code.
 */
TT_NO_DISCARD TtError ttRtosGetTicks(size_t* ticks);

/**
 * @brief Creates a new mutex.
 *
 * The mutex is initially in an unlocked state.
 *
 * @param mutex Initialized with the newly created mutex on success.
 * @return An error code.
 */
TT_NO_DISCARD TtError ttRtosMutexCreate(TtMutex* mutex);

/**
 * @brief Locks a mutex.
 *
 * @param mutex The mutex to lock.
 * @param timeout The timeout in milliseconds or @c SIZE_MAX for no timeout.
 * @return An error code.
 */
TT_NO_DISCARD TtError ttRtosMutexLock(TtMutex* mutex, size_t timeout);

/**
 * @brief Unlocks a mutex.
 *
 * The mutex must be previously successfully locked by the same task using @ref ttRtosMutexLock() .
 *
 * @param mutex The mutex to unlock.
 * @return An error code.
 */
TT_NO_DISCARD TtError ttRtosMutexUnlock(TtMutex* mutex);

/**
 * @brief Destroys a mutex.
 *
 * @param mutex The mutex to destroy.
 * @return An error code.
 */
TT_NO_DISCARD TtError ttRtosMutexDestroy(TtMutex* mutex);

/**
 * @brief Checks if a mutex handle is valid.
 *
 * @param mutex The mutex to check.
 * @return @c true if the mutex handle is valid.
 */
TT_NO_DISCARD static inline bool ttRtosMutexValid(TtMutex* mutex)
{
    return mutex != NULL && mutex->handle != 0;
}

#ifdef __cplusplus
}
#endif

#endif
