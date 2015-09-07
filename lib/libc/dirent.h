//
// Created by ssj on 15-8-24.
//
#pragma once
#include "Type.h"

static const unsigned char DT_CHR = 0x1;
static const unsigned char DT_DIR = 0x2;
static const unsigned char DT_FIFO = 0x4;
static const unsigned char DT_LNK = 0x8;
static const unsigned char DT_REG = 0x10;
static const unsigned char DT_SOCK = 0x20;
static const unsigned char DT_UNKNOWN = 0x40;
static const unsigned char DT_BLK = 0x80;
struct dirent {
    ino_t          d_ino;       /* inode number */
    off_t          d_off;       /* not an offset; see NOTES */
    uint32_t d_reclen;    /* length of this record */
    unsigned char  d_type;      /* type of file; not supported
                                              by all filesystem types */
    char           d_name[256]; /* filename */
};
