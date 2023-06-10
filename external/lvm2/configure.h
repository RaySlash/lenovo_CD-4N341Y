#ifndef CONFIGURE_H
#define CONFIGURE_H

#include <sys/ioctl.h>

#define DEFAULT_DM_ADD_NODE DM_ADD_NODE_ON_CREATE
#define DEFAULT_DM_NAME_MANGLING DM_STRING_MANGLING_NONE
#define DM_LIB_VERSION "1.02.131 (2016-07-15)"
#define DM_DEVICE_UID 0
#define DM_DEVICE_GID 0
#define DM_DEVICE_MODE 0600
#define DM_IOCTLS 1
#define HAVE_GETOPTLONG

#endif  // CONFIGURE_H
