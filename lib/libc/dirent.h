//
// Created by ssj on 15-8-24.
//
#pragma once
#include "Type.h"

struct dirent {
    ino_t          d_ino;       /* inode number */
    off_t          d_off;       /* not an offset; see NOTES */
    unsigned short d_reclen;    /* length of this record */
    unsigned char  d_type;      /* type of file; not supported
                                              by all filesystem types */
    char           d_name[256]; /* filename */
};
