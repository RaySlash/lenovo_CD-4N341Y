#include <unistd.h>
#include <sys/types.h>
#include <sys/syscall.h>

// gettid is not declared by glibc's headers but is used by Android.
// As a workaround, GoogleTV's AndroidConfig.h declares gettid,
// but it doesn't have access to pid_t and needs to return int instead.
// We need to ensure that pid_t and int are equivalent.

namespace android {
template<bool> struct CompileTimeAssert;
template<> struct CompileTimeAssert<true> {};
#define COMPILE_TIME_ASSERT(_exp) \
    template class CompileTimeAssert< (_exp) >;

COMPILE_TIME_ASSERT(sizeof(pid_t) == sizeof(int));
}

extern "C" int gettid(void) {
    return syscall(__NR_gettid);
}
