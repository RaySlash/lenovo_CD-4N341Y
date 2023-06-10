#include <syscall.h>
#include <unistd.h>

/* Wrappers for ioprio syscall */

int ioprio_set(int which, int who, int ioprio) {
    return syscall(__NR_ioprio_set, which, who, ioprio);
}

int ioprio_get(int which, int who) {
    return syscall(__NR_ioprio_get, which, who);
}
