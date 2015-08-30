//
// Created by ssj on 15-8-18.
//
#pragma once
#include "stdio.h"

#define FP_TABLE_SIZE 32

/**
 * @defgroup libc libc (ISO C99)
 * @{
 */


/**
 * @brief Seek operations
 * @{
 */

/** Seek relative to current position. */
#define SEEK_CUR	0

/** Seek relative to end-of-file. */
#define SEEK_END	1

/** Seek relative to start-of-file. */
#define SEEK_SET	2

/**
 * @}
 */

/**
 * @brief File streams
 * @{
 */

/**
 * A structure containing information about a file.
 */
struct FILE
{
    /** File descriptor. */
    int fd;
};
extern FILE fp_table[FP_TABLE_SIZE];

/**
 * @brief Open a stream.
 *
 * The fopen() function shall open the file whose pathname is the
 * string pointed to by filename, and associates a stream with it.
 * The mode argument points to a string. If the string is one of the
 * following, the file shall be opened in the indicated mode. Otherwise,
 * the behavior is undefined.
 *
 * r or rb           Open file for reading.
 * w or wb           Truncate to zero length or create file for writing.
 * a or ab           Append; open or create file for writing at end-of-file.
 * r+ or rb+ or r+b  Open file for update (reading and writing).
 * w+ or wb+ or w+b  Truncate to zero length or create file for update.
 * a+ or ab+ or a+b  Append; open or create file for update, writing at end-of-file.
 *
 * @param filename Path to the file to open.
 * @param mode Mode describes how to open the file.
 * @return Upon successful completion, fopen() shall return a pointer
 *         to the object controlling the stream. Otherwise, a null
 *         pointer shall be returned, and errno shall be set to indicate the error.
 */
extern "C" FILE * fopen(const char *filename,
                        const char *mode);

/**
 * @brief Binary input.
 *
 * The fread() function shall read into the array pointed to by ptr
 * up to nitems elements whose size is specified by size in bytes,
 * from the stream pointed to by stream. For each object, size calls
 * shall be made to the fgetc() function and the results stored, in
 * the order read, in an array of unsigned char exactly overlaying
 * the object. The file position indicator for the stream (if defined)
 * shall be advanced by the number of bytes successfully read. If an
 * error occurs, the resulting value of the file position indicator
 * for the stream is unspecified. If a partial element is read, its
 * value is unspecified.
 *
 * @param ptr Output buffer.
 * @param size Size of each item to read.
 * @param nitems Number of items to read.
 * @param stream FILE pointer to read from.
 * @return Upon successful completion, fread() shall return the number
 *         of elements successfully read which is less than nitems only
 *         if a read error or end-of-file is encountered. If size or
 *         nitems is 0, fread() shall return 0 and the contents of the
 *         array and the state of the stream remain unchanged. Otherwise,
 *         if a read error occurs, the error indicator for the stream shall
 *         be set, and errno shall be set to indicate
 *         the error.
 */
extern "C" size_t fread(void *ptr, size_t size,
                        size_t nitems, FILE *stream);
extern "C" size_t fwrite(const void *ptr, size_t size, size_t nmemb,FILE *stream);
/**
 * @brief Close a stream.
 *
 * The fclose() function shall cause the stream pointed to by stream
 * to be flushed and the associated file to be closed. Any unwritten
 * buffered data for the stream shall be written to the file; any unread
 * buffered data shall be discarded. Whether or not the call succeeds, the
 * stream shall be disassociated from the file and any buffer set by the setbuf()
 * or setvbuf() function shall be disassociated from the stream. If the associated
 * buffer was automatically allocated, it shall be deallocated.
 *
 * @param stream File stream to close.
 * @return Upon successful completion, fclose() shall return 0; otherwise,
 *         it shall return EOF and set errno to indicate the error.
 */
extern "C" int fclose(FILE *stream);


extern "C" int fseek(FILE *stream, long offset, int whence);
extern "C" long ftell(FILE* stream);
/**
 * @}
 */

extern FILE* stdin;
extern FILE* stderr;
extern FILE* stdout;
/**
 * @}
 */
