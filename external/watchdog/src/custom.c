#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <errno.h>
#include <fcntl.h>
#include <string.h>
#if USE_SYSLOG
#include <syslog.h>
#endif
#include <sys/ioctl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

#include "extern.h"
#include "watch_err.h"

int custom_check(void){

  copy_properties_to_fts();

  return (ENOERR);
}
