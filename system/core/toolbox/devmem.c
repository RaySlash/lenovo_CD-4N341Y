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

#include <errno.h>
#include <fcntl.h>
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

static int usage() {
    fprintf(stdout, "Usage: devmem ADDRESS [WIDTH [VALUE]]\n");
    fprintf(stdout, "       read from or write to device memory.\n");
    fprintf(stdout, "\n");
    fprintf(stdout, "              ADDRESS --   address of device memory\n");
    fprintf(stdout, "              WIDTH   --   8, 16, or 32(default). required for write\n");
    fprintf(stdout, "              VALUE   --   value to write. read if none is specified\n");
    return -1;
}

int devmem_main(int argc, char *argv[])
{
    if(argc < 2 || argc > 4) {
        return usage();
    }

    int is_write = (argc == 4);
    int addr = 0, width = 32, value = 0;
    addr = strtoul(argv[1], NULL, 0);
    if (argc > 2) {
        width = strtoul(argv[2], NULL, 0);
        if (argc > 3) {
            value = strtoul(argv[3], NULL, 0);
        }
    }

    if (width != 8 && width != 16 && width != 32) {
        fprintf(stderr, "width has to be 8, 16 or 32\n");
        return usage();
    }

    if (addr % (width >> 3)) {
        fprintf(stderr, "address 0x%x is not %dbit-aligned\n", addr, width);
        return usage();
    }

    int fd = open("/dev/mem", is_write ? (O_RDWR | O_SYNC) :
                  (O_RDONLY | O_SYNC));
    if (fd == -1) {
        fprintf(stderr, "cannot open /dev/mem, error %s\n", strerror(errno));
        return -1;
    }

    int page_size = getpagesize();
    void *base = mmap(NULL, page_size,
                      is_write ? (PROT_READ | PROT_WRITE) : PROT_READ,
                      MAP_SHARED, fd, addr & ~(page_size - 1));
    if (base == MAP_FAILED) {
        fprintf(stderr, "mmap failed, error %s\n", strerror(errno));
        close(fd);
        return -1;
    }

    void *virt = (char*)base + (addr & (page_size - 1));
    if (is_write) {
        switch (width) {
        case 8:
            *(unsigned char*)virt = (unsigned char)value;
            break;
        case 16:
            *(unsigned short*)virt = (unsigned short)value;
            break;
        case 32:
            *(unsigned int*)virt = (unsigned int)value;
            break;
        default:
            fprintf(stderr, "not reached\n");
            break;
        }
    } else {
        switch (width) {
        case 8:
            fprintf(stdout, "0x%02X\n", *(unsigned char*)virt);
            break;
        case 16:
            fprintf(stdout, "0x%04X\n", *(unsigned short*)virt);
            break;
        case 32:
            fprintf(stdout, "0x%08X\n", *(unsigned int*)virt);
            break;
        default:
            fprintf(stderr, "not reached\n");
            break;
        }
    }
    munmap(base, page_size);
    close(fd);
    return 0;
}
