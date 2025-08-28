#include <stdint.h>

#include <tt_sdk/core/tasks.h>
#include <tt_sdk/plugins/rtos.h>
#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>

// enable logging for this source file
LOG_MODULE_REGISTER(zephyrRtosPlugin);

K_THREAD_STACK_DEFINE(stackWatchdog, TT_TASK_WATCHDOG_STACK + K_THREAD_STACK_RESERVED);

static TtError zephyrTaskCreate(void*, void (*func)(void*), void* data, size_t stack, TtTaskPriority priority, const char* name, uintptr_t* handle, uintptr_t* taskCtx);
static void zephyrSleepUsec(void*, size_t usec);
static size_t zephyrGetTicks(void*);
static TtError zephyrMutexCreate(void*, uintptr_t* mutex);
static TtError zephyrMutexLock(void*, uintptr_t mutex, size_t timeput);
static TtError zephyrMutexUnlock(void*, uintptr_t mutex);
static TtError zephyrMutexDestroy(void* ctx, uintptr_t mutex);

TtRtos ttPluginRtos = {
    .context = NULL,
    .name = "zephyr",
    .init = NULL,
    .taskCreate = &zephyrTaskCreate,
    .sleepUsec = &zephyrSleepUsec,
    .getTicks = &zephyrGetTicks,
    .mutexCreate = &zephyrMutexCreate,
    .mutexLock = &zephyrMutexLock,
    .mutexUnlock = &zephyrMutexUnlock,
    .mutexDestroy = &zephyrMutexDestroy,
};

// get the task stack from the task name
// zephyr does not support dynamic stack allocation
// thus stacks must be defined at compile time
// sets pTaskStack to the task stack or NULL if no stack was found for the given name
// returns the size of the stack or 0 if no stack was found for the given name
static size_t getTaskStack(const char* taskName, k_thread_stack_t** pTaskStack)
{
    if (pTaskStack == NULL) {
        return 0;
    }

    if (strcmp(taskName, TT_TASK_WATCHDOG_NAME) == 0) {
        *pTaskStack = stackWatchdog;
        return K_THREAD_STACK_SIZEOF(stackWatchdog);
    } else {
        *pTaskStack = NULL;
        return 0;
    }
}

static int getTaskPriority(TtTaskPriority priority)
{
    // for more details see https://docs.zephyrproject.org/latest/kernel/services/threads/index.html#thread-priorities
    switch (priority) {
    case TT_TASK_PRIO_NORMAL:
        return 3;
    default:
        return 7; // lowest priority
    }
}

static size_t zephyrGetTicks(void*)
{
    // get the raw tick counter
    const int64_t milliseconds = k_uptime_get();

    // convert to a sane number
    return (milliseconds < 0 ? 0 : (size_t)milliseconds);
}

static void taskEntry(void* rawFunc, void* data, void* taskName)
{
    // cast the function pointer
    void (*func)(void*) = rawFunc;

    // call the function and pass the data
    func(data);

    // tasks should never finish/terminate
    LOG_ERR("Tributech SDK task '%s' terminated", (const char*)taskName);
}

static TtError zephyrTaskCreate(void*, void (*func)(void*), void* data, size_t stack, TtTaskPriority priority, const char* name, uintptr_t* handle, uintptr_t* taskCtx)
{
    // stacks are defined at compile time
    TT_UNUSED(stack);

    // get the predefined task stack
    k_thread_stack_t* taskStack;
    size_t stackSize = getTaskStack(name, &taskStack);
    if (taskStack == NULL) {
        LOG_ERR("No task stack registered for task %s", name);
        return TT_E_NO_MEMORY;
    }

    // allocate the thread control block
    struct k_thread* ptrThread = (struct k_thread*)k_malloc(sizeof(struct k_thread));
    if (ptrThread == NULL) {
        return TT_E_NO_MEMORY;
    }

    // initialize and start the thread
    k_thread_create(ptrThread,
        taskStack,
        stackSize,
        &taskEntry,
        func,
        data,
        (void*)name,
        getTaskPriority(priority),
        0, // options, for details see https://docs.zephyrproject.org/latest/kernel/services/threads/index.html#thread-options
        K_NO_WAIT);

    // return the pointer to the created thread
    *handle = (uintptr_t)ptrThread;
    *taskCtx = 0;

    return TT_E_OK;
}

static void zephyrSleepUsec(void*, size_t usec)
{
    k_sleep(K_USEC(usec));
}

static TtError zephyrMutexCreate(void*, uintptr_t* mutex)
{
    // allocate the mutex
    struct k_mutex* ptrMutex = k_malloc(sizeof(struct k_mutex));
    if (ptrMutex == NULL) {
        return TT_E_NO_MEMORY;
    }

    // initialize the mutex
    k_mutex_init(ptrMutex);

    // return the mutex pointer as handle
    *mutex = (uintptr_t)ptrMutex;

    return TT_E_OK;
}

static TtError zephyrMutexLock(void*, uintptr_t mutex, size_t timeout)
{
    struct k_mutex* ptrMutex = (struct k_mutex*)mutex;

    // try to lock the mutex
    int ret = 0;
    if (timeout == SIZE_MAX) {
        ret = k_mutex_lock(ptrMutex, K_FOREVER);
    } else {
        ret = k_mutex_lock(ptrMutex, K_MSEC(timeout));
    }

    // check for timeouts
    if (ret != 0) {
        return TT_E_TIMEOUT;
    }

    // the mutex was locked
    return TT_E_OK;
}

static TtError zephyrMutexUnlock(void*, uintptr_t mutex)
{
    struct k_mutex* ptrMutex = (struct k_mutex*)mutex;

    // unlock the mutex
    k_mutex_unlock(ptrMutex);

    return TT_E_OK;
}

static TtError zephyrMutexDestroy(void* ctx, uintptr_t mutex)
{
    struct k_mutex* ptrMutex = (struct k_mutex*)mutex;

    // delete the mutex
    k_free(ptrMutex);

    return TT_E_OK;
}

__attribute__((weak)) void z_impl_sys_rand_get(void* dst, size_t outlen)
{
    if (dst == NULL) {
        return;
    }
    for (size_t i = 0; i < outlen; i++) {
        ((uint8_t*)dst)[i] = (uint8_t)i;
    }
}
