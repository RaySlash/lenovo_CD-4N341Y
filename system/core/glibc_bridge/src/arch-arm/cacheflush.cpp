#include <unistd.h>
#include <sys/syscall.h>

extern "C" int cacheflush(long start, long end, long flags) {
    return syscall(__ARM_NR_cacheflush, start, end, flags);
}
