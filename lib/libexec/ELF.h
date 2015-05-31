#pragma once

#include"ExecutableFormat.h"
#include "ELFHeader.h"
#include"Type.h"

/**   
 * @defgroup libexec_elf libexec (ELF)
 * @{   
 */

/**
 * Executable and Linkable Format (ELF).
 */
class ELF : public ExecutableFormat
{
    public:

	/**
	 * Class constructor.
	 * @param path Path on filesystem to the ELF executable.
	 * @param fd File descriptor of the ELF executable.
	 * @param header ELF header read from the file.
	 */
	ELF(const char *path, int fd, ELFHeader *header);
	ELF(void* mem,  ELFHeader *header);

	/**
	 * Class destructor.
	 */
	~ELF();
    
	/**
	 * Reads out segments from the ELF program table.
	 * @param regions Memory regions to fill.
	 * @param max Maximum number of memory regions.
	 * @return Number of memory regions or an error code on error.
	 */
	virtual int regions(MemoryRegion *regions, size_t max);

	/**
	 * Lookup the program entry point.
	 * @return Program entry point.
	 */
	addr_t entry();

	/**
	 * Confirms if we understand the given format.
	 * @param path Path to the file to read.
	 * @return true on success and false on failure.
	 */
	static ExecutableFormat * detectMem(void *mem);
	static ExecutableFormat * detectPath(const char *path);

    private:

	/** File descriptor of the ELF executable. */
	int fd;
	void* memory;
	
	/** ELF header. */
	ELFHeader header;
};

/**
 * @}
 */

