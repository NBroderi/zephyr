#include <zephyr/kernel.h>

#include <tt_sdk/plugins/heap.h>

static TtError zephyrAlloc(void*, size_t size, void** addr);
static TtError zephyrFree(void*, const void* addr);

TtHeap ttPluginHeap = {
    .context = NULL,
    .name = "zephyr",
    .init = NULL,
    .alloc = &zephyrAlloc,
    .free = &zephyrFree,
};

static TtError zephyrAlloc(void*, size_t size, void** addr)
{
    // allocations of size 0 are not specified fully, so we allocate a single byte instead
    if (size == 0) {
        size = 1;
    }

    // try to allocate the memory
    void* tmp = k_malloc(size);
    if (tmp == NULL) {
        return TT_E_NO_MEMORY;
    }
    *addr = tmp;

    return TT_E_OK;
}

static TtError zephyrFree(void*, const void* addr)
{
    k_free((void*)addr);
    return TT_E_OK;
}
