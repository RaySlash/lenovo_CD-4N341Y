/* true.c - Return zero.
 *
 * Copyright 2007 Rob Landley <rob@landley.net>
 *
 * See http://opengroup.org/onlinepubs/9699919799/utilities/true.html

USE_TRUE(NEWTOY(true, NULL, TOYFLAG_BIN))
USE_TRUE(OLDTOY(:, true, TOYFLAG_NOFORK))

config TRUE
  bool "true"
  default y
  help
    Return zero.
*/

#include "toys.h"

void true_main(void)
{
  return;
}