#pragma once
#ifndef __ASSEMBLER__

#include <stl/slinkedlist.h>
#include"Type.h"

/**  
 * @defgroup libexec libexec
 * @{  
 */

/**
 * Abstracts a memory region read from a format.
 */
typedef struct MemoryRegion
{
    /**
     * Constructor.
     */
    MemoryRegion() : virtualAddress(0), size(0), data(0)
    {
    }
    
    /**
     * Destructor.
     */
    ~MemoryRegion()
    {
	if (data)
	    delete data;
    }

    /** Beginning of the region. */
    addr_t virtualAddress;
    
    /** Size of the memory region. */
    size_t size;

    /** Page protection flags. */
    uint16_t flags;
    
    /** Memory contents. */
    uint8_t *data;
}
MemoryRegion;

/** Entry point of a program. */
typedef addr_t EntryPoint;

/** Forward declaration. */
class ExecutableFormat;

/**
 * Confirms if we understand the given format.
 * @return true on success and false on failure.
 */
typedef ExecutableFormat* (*FormatDetectorForPath)(const char *path);
typedef ExecutableFormat* (*FormatDetectorForMem)(void* mem);

/**
 * Abstraction class of various executable formats.
 */
class ExecutableFormat
{
    public:

	/**
	 * Class constructor.
	 * @param path Filesystem path to the executable.
	 */
	ExecutableFormat(const char *path);
	
	/**
	 * Class constructor.
	 * @param mem mem address to the executable.
	 */
	ExecutableFormat(void* mem);

	/**
	 * Class destructor.
	 */
	virtual ~ExecutableFormat();

	/**
	 * Retrieve path to the executable.
	 * @return Path on filesystem to the executable.
	 */
	const char * getPath()
	{
	    return path;
	}
    
	/**
	 * Memory regions a program needs at runtime.
	 * @param regions Memory regions to fill.
	 * @param max Maximum number of memory regions.
	 * @return Number of memory regions or an error code on error.
	 */
	virtual int regions(MemoryRegion *regions, size_t max) = 0;

	/**
	 * Lookup the program entry point.
	 * @return Program entry point.
	 */
	virtual addr_t entry() = 0;

	/**
	 * Find a ExecFormat which can handle the given format.
	 * @param path Path to the file to read.
	 * @return A pointer to an ExecutableFormat on success
	 *         and NULL if not found.
	 */
	static ExecutableFormat * findForPath(const char *path);
	
	static ExecutableFormat * findForMem(void* mem);

    private:
    
	/** Path to the executable. */
	const char *path = nullptr;
	void* mem = nullptr;
};

/**
 * @}
 */

#endif /* __ASSEMBLER__ */
