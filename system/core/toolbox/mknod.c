/*
 * Copyright (c) 20012, The Android Open Source Project
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *  * Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 *  * Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in
 *    the documentation and/or other materials provided with the
 *    distribution.
 *  * Neither the name of Google, Inc. nor the names of its contributors
 *    may be used to endorse or promote products derived from this
 *    software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
 * FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE
 * COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
 * BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS
 * OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED
 * AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
 * OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT
 * OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/stat.h>

// if glibc version >= 2.25
#include <sys/sysmacros.h>

static void usage(char *cmd);

const struct {
    char name;
    mode_t mode;
} modes_table [] = {
    {'b', S_IFBLK},
    {'c', S_IFCHR},
    {'u', S_IFCHR},
    {'p', S_IFIFO}
};

int mknod_main(int argc, char *argv[]) {
    mode_t mode = 0666;
    char *name;
    dev_t dev = 0;
    int i, total;

    if ((argc != 5) && (argc != 3)) {
        usage(argv[0]);
        return -1;
    }

    name  = argv[1];
    total = sizeof(modes_table) / sizeof(modes_table[0]);
    for (i = 0; i < total; i ++) {
        if (*argv[2] == modes_table[i].name) {
            mode |= modes_table[i].mode;
            break;
        }
    }

    if (i == total) {
        usage(argv[0]);
        return -1;
    }
    if (argc == 5) {
        dev = makedev(atoi(argv[3]), atoi(argv[4]));
    }
    if (mknod(name, mode, dev)) {
        fprintf(stderr, "mknod error: %s\n", strerror(errno));
    }
    return 0;
}

static void usage(char *cmd) {
    fprintf(stderr, "Usage: %s NAME TYPE [MAJOR MINOR]\n"
        "b      create a block (buffered) special file\n"
        "c, u   create a character (unbuffered) special file"
        "p      create a FIFO"
        , cmd);
}

#ifdef HOST
int main(int argc, char *argv[])
{
    return mknod_main(argc, argv);
}
#endif
