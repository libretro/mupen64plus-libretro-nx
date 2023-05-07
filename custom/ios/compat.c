#include <libkern/OSCacheControl.h>

void __clear_cache(void *start, void *end)
{
    size_t len = (char*)end - (char*)start;
    sys_dcache_flush(start, len);
    sys_icache_invalidate(start, len);
}
