/* linux */
#define SYSCONFDIR     "/etc/dhcpcd"
#define SBINDIR        "/etc/dhcpcd"
#define LIBEXECDIR     "/etc/dhcpcd"
#define DBDIR  "/tmp/dhcp"
#define RUNDIR "/tmp/dhcp"
#define USE_LOGFILE 0
#define HAVE_EPOLL
#ifndef NBBY
#define NBBY  8
#endif
#include "compat/closefrom.h"
#include "compat/endian.h"
#include "compat/posix_spawn.h"
#include "compat/queue.h"
#include "compat/strtoi.h"

#include <signal.h>
#include <linux/rtnetlink.h>
