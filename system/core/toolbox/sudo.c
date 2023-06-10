/*
 * Copyright (C) 2012 The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <pwd.h>
#include <grp.h>

#include <private/android_filesystem_config.h>

/* we support up to 32 supplementary group ID */
#define NR_SUPP_GIDS 32

static void usage(int argc, char *argv[])
{
    fprintf(stdout,
        "Usage: sudo -u UID [-g GID[:GID]...] FILE [ARG] ...\n");
}

int sudo_main(int argc, char *argv[])
{
    uid_t uid = -1, myuid;
    gid_t gid = -1, supp_gids[NR_SUPP_GIDS];
    int nr_supp_gids = 0, i, index;

    /* only root can sudo */
    myuid = getuid();
    if (myuid != 0) {
        fprintf(stderr, "sudo: only root can sudo, %d is not allowed", myuid);
        exit(-1);
    }

    if(argc < 4) {
        usage(argc, argv);
        exit(-1);
    }

    index = 1;
    while (index < argc - 1) {
        if (!strcmp("-u", argv[index])) {
            /* parse uid */
            char *user = argv[++index];
            struct passwd *pw;

            pw = getpwnam(user);
            if (pw == 0) {
                fprintf(stderr, "%s not a valid user\n", user);
                exit(-1);
            } else {
                uid = pw->pw_uid;
                gid = pw->pw_gid;
                setenv("HOME", pw->pw_dir, 1);
            }
        } else if (!strcmp("-g", argv[index])) {
            /* parse (supplementary) gids */
            char * current = argv[++index], *next;
            struct group *grp;

            for (i = 0; current && (i < NR_SUPP_GIDS); ++i) {
                next = strchr(current, ':');
                if (next) {
                    *next++ = '\0';
                }

                grp = getgrnam(current);
                if (grp == NULL) {
                    fprintf(stderr, "%s is not a valid group\n", current);
                    exit(-1);
                } else {
                    supp_gids[i] = grp->gr_gid;
                }
                current = next;
            }
            nr_supp_gids = i;
        } else {
            break;
        }
        ++index;
    }

    if (uid == (uid_t)-1 || uid == 0) {
        usage (argc, argv);
        exit(-1);
    }

    /* set supplemtary groups */
    if (nr_supp_gids && setgroups(nr_supp_gids, supp_gids)) {
        fprintf(stderr, "sudo: setgroups error %s\n", strerror(errno));
        exit(-1);
    }

    /* set uid and gid */
    if (setgid(gid) || setuid(uid)) {
        fprintf(stderr, "sudo: %s\n", strerror(errno));
        exit(-1);
    }

    if (index == argc) {
        usage(argc, argv);
        exit(-1);
    } else {
        /* Copy the rest of the args from main. */
        int nr_args = argc - index;
        char **exec_args = (char **)malloc(sizeof(char*)
            * (nr_args + 1));
        if (!exec_args) {
            fprintf(stderr, "memory alloc error %s\n", strerror(errno));
            exit(-1);
        }
        memcpy(exec_args, &argv[index], sizeof(*exec_args) * nr_args);
        exec_args[nr_args] = NULL;

        if (execvp(argv[index], exec_args)) {
            fprintf(stderr, "%s: %s exec failed\n", argv[0], argv[index]);
        }
    }

    return -1;
}
